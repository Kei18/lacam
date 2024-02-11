#include "../include/graph.hpp"

Graph::~Graph()
{
  for (auto& v : V)
    if (v != nullptr) delete v;
  V.clear();
  delete cache;
}

// regular function to load graph
static const std::regex r_height = std::regex(R"(height\s(\d+))");
static const std::regex r_width = std::regex(R"(width\s(\d+))");
static const std::regex r_map = std::regex(R"(map)");

Graph::Graph(
  const std::string& filename,
  std::shared_ptr<spdlog::logger> _logger,
  uint goals_m,
  uint goals_k,
  uint ngoals,
  CacheType cache_type,
  std::mt19937* _randomSeed) :
  V(Vertices()),
  width(0),
  height(0),
  randomSeed(_randomSeed),
  logger(_logger)
{
  std::ifstream file(filename);
  if (!file) {
    logger->error("file {} is not found.", filename);
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

  if (is_cache(cache_type)) {
    // Generate cache
    cache = new Cache(_logger, cache_type, randomSeed);

    // create vertices
    int y = 0;
    while (getline(file, line)) {
      // for CRLF coding
      if (*(line.end() - 1) == 0x0d) line.pop_back();
      for (int x = 0; x < width; ++x) {
        char s = line[x];
        // record roads
        if (s == 'T' or s == '@') continue;
        auto index = width * y + x;
        auto v = new Vertex(V.size(), index, width);

        // record unloading ports
        if (s == 'U') {
          unloading_ports.push_back(v);
        }
        // record cache blocks
        else if (s == 'C') {
          v->cargo = true;
          // insert into cache
          cache->node_cargo.push_back(v);
          cache->node_id.push_back(v);
          cache->node_coming_cargo.push_back(v);
          cache->bit_cache_get_lock.push_back(0);
          cache->bit_cache_insert_lock.push_back(0);
          cache->is_empty.push_back(false);
          switch (cache_type) {
          case CacheType::LRU:
            cache->LRU.push_back(0);
            break;
          case CacheType::FIFO:
            cache->FIFO.push_back(0);
            break;
          case CacheType::RANDOM:
            break;
          default:
            logger->error("Unreachable cache type!");
            exit(1);
          }
        }
        // record cargo blocks
        else if (s == 'H') {
          v->cargo = true;
          cargo_vertices.push_back(v);
        }

        // record in whole map
        V.push_back(v);
        U[index] = v;
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
          if (v->cargo) {
            if (u != nullptr && !u->cargo) v->neighbor.push_back(u);
          }
          else {
            if (u != nullptr) v->neighbor.push_back(u);
          }
        }
        // right
        if (x < width - 1) {
          auto u = U[width * y + (x + 1)];
          if (v->cargo) {
            if (u != nullptr && !u->cargo) v->neighbor.push_back(u);
          }
          else {
            if (u != nullptr) v->neighbor.push_back(u);
          }
        }
        // up
        if (y < height - 1) {
          auto u = U[width * (y + 1) + x];
          if (v->cargo) {
            if (u != nullptr && !u->cargo) v->neighbor.push_back(u);
          }
          else {
            if (u != nullptr) v->neighbor.push_back(u);
          }
        }
        // down
        if (y > 0) {
          auto u = U[width * (y - 1) + x];
          if (v->cargo) {
            if (u != nullptr && !u->cargo) v->neighbor.push_back(u);
          }
          else {
            if (u != nullptr) v->neighbor.push_back(u);
          }
        }
      }
    }
    logger->info("Cache blocks:     {}", cache->node_id);
  }
  else {
    // create vertices
    int y = 0;
    while (getline(file, line)) {
      // for CRLF coding
      if (*(line.end() - 1) == 0x0d) line.pop_back();
      for (int x = 0; x < width; ++x) {
        char s = line[x];
        // record roads
        if (s == 'T' or s == '@') continue;
        auto index = width * y + x;
        auto v = new Vertex(V.size(), index, width);

        // record unloading ports
        if (s == 'U') {
          unloading_ports.push_back(v);
        }
        // record cargo blocks
        else if (s == 'H') {
          v->cargo = true;
          cargo_vertices.push_back(v);
        }
        // record in whole map
        V.push_back(v);
        U[index] = v;
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
  }

  logger->info("Unloading ports:  {}", unloading_ports);
  logger->info("Generating goals...");
  fill_goals_list(goals_m, goals_k, ngoals);
}

int Graph::size() const { return V.size(); }

Vertex* Graph::random_target_vertex() {
  if (cargo_vertices.empty()) {
    return nullptr;  // Return nullptr if the set is empty
  }

  int index = get_random_int(randomSeed, 0, cargo_vertices.size() - 1);
  auto it = cargo_vertices.begin();
  std::advance(it, index);

  return *it;
}

void Graph::fill_goals_list(uint goals_m, uint goals_k, uint ngoals) {
  std::deque<Vertex*> sliding_window;
  std::unordered_map<Vertex*, int> goal_count;
  std::unordered_set<Vertex*> diff_goals;
  uint goals_generated = 0;

  while (goals_list.size() < ngoals) {
    if (goals_generated % 1000 == 0) {
      logger->info("Generated {:5}/{:5} goals.", goals_generated, ngoals);
    }

    Vertex* selected_goal = random_target_vertex();
    if (!selected_goal) {
      // Stop if no more goals can be selected
      break;
    }

    if (sliding_window.size() == goals_m) {
      Vertex* removed_goal = sliding_window.front();
      sliding_window.pop_front();
      goal_count[removed_goal]--;
      if (goal_count[removed_goal] == 0) {
        goal_count.erase(removed_goal);
        diff_goals.erase(removed_goal);
      }
    }

    if (diff_goals.size() == goals_k) {
      int index = get_random_int(randomSeed, 0, goals_k - 1);
      auto it = diff_goals.begin();
      std::advance(it, index);
      selected_goal = *it;
    }

    // Update status
    sliding_window.push_back(selected_goal);
    goal_count[selected_goal]++;
    diff_goals.insert(selected_goal);
    goals_list.push_back(selected_goal);
    goals_generated++;
  }
}

Vertex* Graph::get_next_goal() {
  assert(!goals_list.empty());
  // We should let each reaching unploading-port agent to be disappeared when the total
  // number of goals has been assigned but yet finished. For now, we let it 
  // out to have new random target. Let us know if this will cause any problem
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
