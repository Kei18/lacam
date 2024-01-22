#include "../include/instance.hpp"

Instance::Instance(const std::string& map_filename, std::mt19937* MT, const int _nagents, const int _ngoals)
  : G(Graph(map_filename, MT)), starts(Config()), goals(Config()), unloading_port(Config()), nagents(_nagents), ngoals(_ngoals)
{
  const auto K = G.size();

  // set agents random start potition
  auto s_indexes = std::vector<int>(K);
  std::iota(s_indexes.begin(), s_indexes.end(), 0);
  std::shuffle(s_indexes.begin(), s_indexes.end(), *MT);
  int i = 0;
  while (true) {
    if (i >= K) return;
    starts.push_back(G.V[s_indexes[i]]);
    if (starts.size() == nagents) break;
    ++i;
  }

  // set goals
  int j = 0;
  while (true) {
    if (j >= K) return;
    goals.push_back(G.random_target_vertex());
    if (goals.size() == nagents) break;
    ++j;
  }
}

bool Instance::is_valid(const int verbose) const
{
  if (nagents != starts.size() || nagents != goals.size()) {
    info(1, verbose, "invalid N, check instance");
    return false;
  }
  return true;
}
