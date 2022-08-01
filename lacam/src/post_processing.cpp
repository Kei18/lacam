#include "../include/post_processing.hpp"

#include "../include/dist_table.hpp"

bool is_feasible_solution(const Instance& ins, const Solution& solution,
                          const int verbose)
{
  if (solution.empty()) return true;

  // check start locations
  if (!is_same_config(solution.front(), ins.starts)) {
    info(1, verbose, "invalid starts");
    return false;
  }

  // check goal locations
  if (!is_same_config(solution.back(), ins.goals)) {
    info(1, verbose, "invalid goals");
    return false;
  }

  for (auto t = 1; t < solution.size(); ++t) {
    for (auto i = 0; i < ins.N; ++i) {
      auto v_i_from = solution[t - 1][i];
      auto v_i_to = solution[t][i];
      // check connectivity
      if (v_i_from != v_i_to &&
          std::find(v_i_to->neighbor.begin(), v_i_to->neighbor.end(),
                    v_i_from) == v_i_to->neighbor.end()) {
        info(1, verbose, "invalid move");
        return false;
      }

      // check conflicts
      for (auto j = i + 1; j < ins.N; ++j) {
        auto v_j_from = solution[t - 1][j];
        auto v_j_to = solution[t][j];
        // vertex conflicts
        if (v_j_to == v_i_to) {
          info(1, verbose, "vertex conflict");
          return false;
        }
        // swap conflicts
        if (v_j_to == v_i_from && v_j_from == v_i_to) {
          info(1, verbose, "edge conflict");
          return false;
        }
      }
    }
  }

  return true;
}

int get_makespan(const Solution& solution)
{
  if (solution.empty()) return 0;
  return solution.size() - 1;
}

int get_path_cost(const Solution& solution, int i)
{
  const auto makespan = solution.size();
  const auto g = solution.back()[i];
  auto c = makespan;
  while (c > 0 && solution[c - 1][i] == g) --c;
  return c;
}

int get_sum_of_costs(const Solution& solution)
{
  if (solution.empty()) return 0;
  int c = 0;
  const auto N = solution.front().size();
  for (auto i = 0; i < N; ++i) c += get_path_cost(solution, i);
  return c;
}

int get_makespan_lower_bound(const Instance& ins, DistTable& dist_table)
{
  int c = 0;
  for (auto i = 0; i < ins.N; ++i) {
    c = std::max(c, dist_table.get(i, ins.starts[i]));
  }
  return c;
}

int get_sum_of_costs_lower_bound(const Instance& ins, DistTable& dist_table)
{
  int c = 0;
  for (auto i = 0; i < ins.N; ++i) {
    c += dist_table.get(i, ins.starts[i]);
  }
  return c;
}

void print_stats(const int verbose, const Instance& ins,
                 const Solution& solution, const double comp_time_ms)
{
  auto dist_table = DistTable(ins);
  const auto makespan = get_makespan(solution);
  const auto makespan_lb = get_makespan_lower_bound(ins, dist_table);
  const auto sum_of_costs = get_sum_of_costs(solution);
  const auto sum_of_costs_lb = get_sum_of_costs_lower_bound(ins, dist_table);
  info(1, verbose, "solved: ", comp_time_ms, "ms", "\tmakespan: ", makespan,
       " (lb=", makespan_lb, ", sub-opt-ub=", (double)makespan / makespan_lb,
       ")", "\tsum_of_costs: ", sum_of_costs, " (lb=", sum_of_costs_lb,
       ", sub-opt-ub=", (double)sum_of_costs / sum_of_costs_lb, ")");
}

// for log of map_name
static const std::regex r_map_name = std::regex(R"(.+/(.+))");

void make_log(const Instance& ins, const Solution& solution,
              const std::string& output_name, const double comp_time_ms,
              const std::string& map_name, const int seed, const bool log_short)
{
  // map name
  std::smatch results;
  const auto map_recorded_name =
      (std::regex_match(map_name, results, r_map_name)) ? results[1].str()
                                                        : map_name;

  // for instance-specific values
  auto dist_table = DistTable(ins);

  // log for visualizer
  auto get_x = [&](int k) { return k % ins.G.width; };
  auto get_y = [&](int k) { return k / ins.G.width; };
  std::ofstream log;
  log.open(output_name, std::ios::out);
  log << "agents=" << ins.N << "\n";
  log << "map_file=" << map_recorded_name << "\n";
  log << "solver=planner\n";
  log << "solved=" << !solution.empty() << "\n";
  log << "soc=" << get_sum_of_costs(solution) << "\n";
  log << "soc_lb=" << get_sum_of_costs_lower_bound(ins, dist_table) << "\n";
  log << "makespan=" << get_makespan(solution) << "\n";
  log << "makespan_lb=" << get_makespan_lower_bound(ins, dist_table) << "\n";
  log << "comp_time=" << comp_time_ms << "\n";
  log << "seed=" << seed << "\n";
  if (log_short) return;
  log << "starts=";
  for (auto i = 0; i < ins.N; ++i) {
    auto k = ins.starts[i]->index;
    log << "(" << get_x(k) << "," << get_y(k) << "),";
  }
  log << "\ngoals=";
  for (auto i = 0; i < ins.N; ++i) {
    auto k = ins.goals[i]->index;
    log << "(" << get_x(k) << "," << get_y(k) << "),";
  }
  log << "\nsolution=\n";
  for (auto t = 0; t < solution.size(); ++t) {
    log << t << ":";
    auto C = solution[t];
    for (auto v : C) {
      log << "(" << get_x(v->index) << "," << get_y(v->index) << "),";
    }
    log << "\n";
  }
  log.close();
}
