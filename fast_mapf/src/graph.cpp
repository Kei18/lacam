#include "../include/graph.hpp"

Vertex::Vertex(int _id) : id(_id), neighbor(Vertices()) {}

Graph::Graph() : V(Vertices()), width(0), height(0) {}
Graph::~Graph()
{
  for (auto v : V)
    if (v != nullptr) delete v;
}

const std::regex r_height = std::regex(R"(height\s(\d+))");
const std::regex r_width = std::regex(R"(width\s(\d+))");
const std::regex r_map = std::regex(R"(map)");

Graph::Graph(const std::string& filename) : V(Vertices()), width(0), height(0)
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

  V = Vertices(width * height, nullptr);

  // build vertices
  int y = 0;
  while (getline(file, line)) {
    // for CRLF coding
    if (*(line.end() - 1) == 0x0d) line.pop_back();
    for (int x = 0; x < width; ++x) {
      char s = line[x];
      if (s == 'T' or s == '@') continue;  // object
      int id = width * y + x;
      V[id] = new Vertex(id);
    }
    ++y;
  }
  file.close();

  // create edges
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      auto v = V[width * y + x];
      if (v == nullptr) continue;
      // left
      if (x > 0) {
        auto u = V[width * y + (x - 1)];
        if (u != nullptr) v->neighbor.push_back(u);
      }
      // right
      if (x < width - 1) {
        auto u = V[width * y + (x + 1)];
        if (u != nullptr) v->neighbor.push_back(u);
      }
      // up
      if (y < height - 1) {
        auto u = V[width * (y + 1) + x];
        if (u != nullptr) v->neighbor.push_back(u);
      }
      // down
      if (y > 0) {
        auto u = V[width * (y - 1) + x];
        if (u != nullptr) v->neighbor.push_back(u);
      }
    }
  }
}

int Graph::get_num_vertices() const
{
  int cnt = 0;
  for (auto v : V) {
    if (v != nullptr) ++cnt;
  }
  return cnt;
}

int Graph::size() const { return V.size(); }

bool is_same_config(const Config& C1, const Config& C2)
{
  const auto N = C1.size();
  for (int i = 0; i < N; ++i) {
    if (C1[i]->id != C2[i]->id) return false;
  }
  return true;
}
