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
  if (!is_reach_at_least_one(solution.back(), ins.goals)) {
    info(1, verbose, "invalid goals");
    return false;
  }

  for (size_t t = 1; t < solution.size(); ++t) {
    for (size_t i = 0; i < ins.nagents; ++i) {
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
      for (size_t j = i + 1; j < ins.nagents; ++j) {
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
  for (size_t i = 0; i < N; ++i) c += get_path_cost(solution, i);
  return c;
}

int get_sum_of_loss(const Solution& solution)
{
  if (solution.empty()) return 0;
  int c = 0;
  const auto N = solution.front().size();
  const auto T = solution.size();
  for (size_t i = 0; i < N; ++i) {
    auto g = solution.back()[i];
    for (size_t t = 1; t < T; ++t) {
      if (solution[t - 1][i] != g || solution[t][i] != g) ++c;
    }
  }
  return c;
}

int get_makespan_lower_bound(const Instance& ins, DistTable& dist_table)
{
  int c = 0;
  for (size_t i = 0; i < ins.nagents; ++i) {
    c = std::max(c, dist_table.get(i, ins.starts[i]));
  }
  return c;
}

int get_sum_of_costs_lower_bound(const Instance& ins, DistTable& dist_table)
{
  int c = 0;
  for (size_t i = 0; i < ins.nagents; ++i) {
    c += dist_table.get(i, ins.starts[i]);
  }
  return c;
}

void print_stats(const int verbose, const Instance& ins,
  const Solution& solution, const double comp_time_ms)
{
  auto ceil = [](float x) { return std::ceil(x * 100) / 100; };
  auto dist_table = DistTable(ins);
  const auto makespan = get_makespan(solution);
  const auto makespan_lb = get_makespan_lower_bound(ins, dist_table);
  const auto sum_of_costs = get_sum_of_costs(solution);
  const auto sum_of_costs_lb = get_sum_of_costs_lower_bound(ins, dist_table);
  const auto sum_of_loss = get_sum_of_loss(solution);
  info(1, verbose, "solved: ", comp_time_ms, "ms", "\tmakespan: ", makespan,
    " (lb=", makespan_lb, ", ub=", ceil((float)makespan / makespan_lb), ")",
    "\tsum_of_costs: ", sum_of_costs, " (lb=", sum_of_costs_lb,
    ", ub=", ceil((float)sum_of_costs / sum_of_costs_lb), ")",
    "\tsum_of_loss: ", sum_of_loss, " (lb=", sum_of_costs_lb,
    ", ub=", ceil((float)sum_of_loss / sum_of_costs_lb), ")");
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
  log << "agents=" << ins.nagents << "\n";
  log << "map_file=" << map_recorded_name << "\n";
  log << "solver=planner\n";
  log << "solved=" << !solution.empty() << "\n";
  log << "soc=" << get_sum_of_costs(solution) << "\n";
  log << "soc_lb=" << get_sum_of_costs_lower_bound(ins, dist_table) << "\n";
  log << "makespan=" << get_makespan(solution) << "\n";
  log << "makespan_lb=" << get_makespan_lower_bound(ins, dist_table) << "\n";
  log << "sum_of_loss=" << get_sum_of_loss(solution) << "\n";
  log << "sum_of_loss_lb=" << get_sum_of_costs_lower_bound(ins, dist_table)
    << "\n";
  log << "comp_time=" << comp_time_ms << "\n";
  log << "seed=" << seed << "\n";
  if (log_short) return;
  log << "starts=";
  for (size_t i = 0; i < ins.nagents; ++i) {
    auto k = ins.starts[i]->index;
    log << "(" << get_x(k) << "," << get_y(k) << "),";
  }
  log << "\ngoals=";
  for (size_t i = 0; i < ins.nagents; ++i) {
    auto k = ins.goals[i]->index;
    log << "(" << get_x(k) << "," << get_y(k) << "),";
  }
  log << "\nsolution=\n";
  std::vector<std::vector<int> > new_sol(ins.nagents, std::vector<int>(solution.size(), 0));
  for (size_t t = 0; t < solution.size(); ++t) {
    log << t << ":";
    auto C = solution[t];
    int idx = 0;
    for (auto v : C) {
      log << "(" << get_x(v->index) << "," << get_y(v->index) << "),";
      new_sol[idx][t] = v->index;
      idx++;
    }
    log << "\n";
  }
  log.close();


  std::ofstream out2("vis.yaml");
  out2 << "statistics:" << std::endl;
  out2 << "  makespan: " << get_makespan(solution) << std::endl;
  out2 << "  makespan_lb: " << get_makespan_lower_bound(ins, dist_table) << std::endl;
  out2 << "  seed: " << seed << "\n";
  out2 << "  solved: " << !solution.empty() << "\n";
  out2 << "  soc: " << get_sum_of_costs(solution) << "\n";
  out2 << "  soc_lb: " << get_sum_of_costs_lower_bound(ins, dist_table) << "\n";

  out2 << "schedule:" << std::endl;




  for (size_t a = 0; a < new_sol.size(); ++a) {
    out2 << "  agent" << a << ":" << std::endl;
    for (size_t t = 0; t < new_sol[a].size(); ++t) {
      out2 << "    - x: " << get_y(new_sol[a][t]) << std::endl
        << "      y: " << get_x(new_sol[a][t]) << std::endl
        << "      t: " << t << std::endl;
    }
  }

  out2.close();
}
