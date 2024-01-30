#include "../include/graph.hpp"

Vertex::Vertex(int _id, int _index, int _width) : id(_id), index(_index), width(_width), neighbor(Vertices()) {}

CargoVertex::CargoVertex(int _index) : index(_index) {}

Graph::~Graph()
{
  for (auto& v : V)
    if (v != nullptr) delete v;
  V.clear();
}

// regular function to load graph
static const std::regex r_height = std::regex(R"(height\s(\d+))");
static const std::regex r_width = std::regex(R"(width\s(\d+))");
static const std::regex r_map = std::regex(R"(map)");

Graph::Graph(const std::string& filename, std::shared_ptr<spdlog::logger> _logger, uint goals_m, uint goals_k, uint ngoals, std::mt19937* _randomSeed) : V(Vertices()), cache(Cache(_logger)), width(0), height(0), randomSeed(_randomSeed), logger(std::move(_logger))
{
  std::ifstream file(filename);
  if (!file) {
    std::cout << "file " << filename << " is not found." << std::endl;
    return;
  }
  std::string line;
  std::smatch results;

  // read fundamental graph parameters
  while (getline(file, line)) {
    // for CRLF coding
    if (*(line.end() - 1) == 0x0d) line.pop_back();

    if (std::regex_match(line, results, r_height)) {
      height = std::stoi(results[1].str());
    }
    if (std::regex_match(line, results, r_width)) {
      width = std::stoi(results[1].str());
    }
    if (std::regex_match(line, results, r_map)) break;
  }

  U = Vertices(width * height, nullptr);

  // create vertices
  int y = 0;
  while (getline(file, line)) {
    // for CRLF coding
    if (*(line.end() - 1) == 0x0d) line.pop_back();
    for (int x = 0; x < width; ++x) {
      char s = line[x];
      if (s == 'H') {
        // record cargoes
        auto index = width * y + x;
        auto v = new CargoVertex(index);
        cargo_vertices.push_back(v);
      }
      else {
        // record roads
        if (s == 'T' or s == '@') continue;
        auto index = width * y + x;
        auto v = new Vertex(V.size(), index, width);
        V.push_back(v);
        U[index] = v;
        // record unloading ports
        if (s == 'U') {
          unloading_ports.push_back(v);
        }
        // record cache area
        else if (s == 'C') {
          cache.node_cargo.push_back(v);
          cache.node_id.push_back(v);
          cache.LRU.push_back(0);
          cache.bit_lock.push_back(0);
        }
      }
    }
    ++y;
  }
  file.close();

  // create edges
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      auto v = U[width * y + x];
      if (v == nullptr) continue;
      // left
      if (x > 0) {
        auto u = U[width * y + (x - 1)];
        if (u != nullptr) v->neighbor.push_back(u);
      }
      // right
      if (x < width - 1) {
        auto u = U[width * y + (x + 1)];
        if (u != nullptr) v->neighbor.push_back(u);
      }
      // up
      if (y < height - 1) {
        auto u = U[width * (y + 1) + x];
        if (u != nullptr) v->neighbor.push_back(u);
      }
      // down
      if (y > 0) {
        auto u = U[width * (y - 1) + x];
        if (u != nullptr) v->neighbor.push_back(u);
      }
    }
  }

  // Add goal candidate
  // TODO: This should be changed later when we apply
  // cargo point to be access but not pass by
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      auto c = cargo_vertices[width * y + x];
      if (c == nullptr) continue;
      // left
      if (x > 0) {
        auto u = U[width * y + (x - 1)];
        if (u != nullptr) goals_candidate.insert(u);
      }
      // right
      if (x < width - 1) {
        auto u = U[width * y + (x + 1)];
        if (u != nullptr) goals_candidate.insert(u);
      }
      // up
      if (y < height - 1) {
        auto u = U[width * (y + 1) + x];
        if (u != nullptr) goals_candidate.insert(u);
      }
      // down 
      if (y > 0) {
        auto u = U[width * (y - 1) + x];
        if (u != nullptr) goals_candidate.insert(u);
      }
    }
  }

  fill_goals_list(goals_m, goals_k, ngoals);
}

int Graph::size() const { return V.size(); }

Vertex* Graph::random_target_vertex() {
  if (goals_candidate.empty()) {
    return nullptr;  // Return nullptr if the set is empty
  }

  int index = get_random_int(randomSeed, 0, goals_candidate.size() - 1);
  auto it = goals_candidate.begin();
  std::advance(it, index);

  return *it;
}

void Graph::fill_goals_list(uint goals_m, uint goals_k, uint ngoals) {
  std::deque<Vertex*> sliding_window;
  std::unordered_map<Vertex*, int> goal_count;
  uint goals_generated = 0;

  while (goals_list.size() < ngoals) {
    Vertex* selected_goal = random_target_vertex();

    if (goals_generated % 100 == 0) {
      logger->debug("Generated {}/{} goals.", goals_generated, ngoals);
    }

    if (!selected_goal) {
      break; // Stop if no more goals can be selected
    }

    sliding_window.push_back(selected_goal);
    goal_count[selected_goal]++;

    if (sliding_window.size() > goals_m) {
      Vertex* removed_goal = sliding_window.front();
      sliding_window.pop_front();
      goal_count[removed_goal]--;
      if (goal_count[removed_goal] == 0) {
        goal_count.erase(removed_goal);
      }
    }

    if (goal_count.size() <= goals_k) {
      goals_list.push_back(selected_goal);
      goals_generated++;
    }
  }
}

Vertex* Graph::get_next_goal() {
  assert(!goals_list.empty());

  // We should let reach unploading port agents to be disappeared when the total
  // number of goals has been assigned but yet finished. For simplify, we let it 
  // out to have new random target
  if (goals_cnt >= goals_list.size()) {
    return random_target_vertex();
  }
  auto goal = goals_list[goals_cnt];
  goals_cnt++;
  return goal;
}

bool is_same_config(const Config& C1, const Config& C2)
{
  const auto N = C1.size();
  for (size_t i = 0; i < N; ++i) {
    if (C1[i]->id != C2[i]->id) return false;
  }
  return true;
}

bool is_reach_at_least_one(const Config& C1, const Config& C2)
{
  const auto N = C1.size();
  for (size_t i = 0; i < N; i++) {
    if (C1[i]->id == C2[i]->id) return true;
  }
  return false;
}

uint ConfigHasher::operator()(const Config& C) const
{
  uint hash = C.size();
  for (auto& v : C) {
    hash ^= v->id + 0x9e3779b9 + (hash << 6) + (hash >> 2);
  }
  return hash;
}
