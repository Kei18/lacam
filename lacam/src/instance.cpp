#include "../include/instance.hpp"

Instance::Instance(const std::string& map_filename, std::mt19937* MT, const int _nagents, const int _ngoals)
  : G(Graph(map_filename, MT)), starts(Config()), goals(Config()), nagents(_nagents), ngoals(_ngoals)
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

int Instance::update_on_reaching_goals(std::vector<Config>& vertex_list, int remain_goals) {
  std::cerr << "Remain goals: " << remain_goals << std::endl;
  int reached_count = 0;

  // Iterating through each time step
  for (size_t i = 0; i < vertex_list.size(); ++i) {
    bool any_vertex_reached = false;

    // Check each vertex at this time step
    for (size_t j = 0; j < vertex_list[i].size(); ++j) {
      // TODO: assign goals to closed free agents
      if ((*vertex_list[i][j] == *goals[j]) && (remain_goals > 0)) {
        remain_goals--;
        // Update goals and starts, mark that a vertex has reached
        if (goals[j] == G.unloading_ports[0]) {
          goals[j] = G.random_target_vertex();
          reached_count++;
        }
        else {
          goals[j] = G.unloading_ports[0];
        }
        any_vertex_reached = true;
      }
    }

    // If any vertex reached a goal at this time step, update starts and break
    if (any_vertex_reached) {
      starts = vertex_list[i];
      break;
    }
  }

  std::cerr << "Ends: " << starts << std::endl;
  return reached_count;
}
