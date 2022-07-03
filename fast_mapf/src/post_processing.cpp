#include "../include/post_processing.hpp"

#include "../include/dist_table.hpp"

void print_stats(const int verbose, const Instance& ins,
                 const Solution& solution, const double comp_time_ms)
{
  const auto dist_table = DistTable(ins);
  const auto makespan = get_makespan(solution);
  const auto makespan_lb = get_makespan_lower_bound(ins, dist_table);
  const auto sum_of_costs = get_sum_of_costs(solution);
  const auto sum_of_costs_lb = get_sum_of_costs_lower_bound(ins, dist_table);
  info(1, verbose, "solved: ", comp_time_ms, "ms", "\tmakespan: ", makespan,
       " (lb=", makespan_lb, ", sub-opt-ub=", (double)makespan / makespan_lb,
       ")", "\tsum_of_costs: ", sum_of_costs, " (lb=", sum_of_costs_lb,
       ", sub-opt-ub=", (double)sum_of_costs / sum_of_costs_lb, ")");
}

static const std::regex r_map_name = std::regex(R"(.+/(.+))");

void make_log(const Instance& ins, const Solution& solution,
              const std::string& output_name, const double comp_time_ms,
              const std::string& map_name)
{
  // map name
  std::smatch results;
  const auto map_recorded_name =
      (std::regex_match(map_name, results, r_map_name)) ? results[1].str()
                                                        : map_name;

  // log for visualizer
  auto get_x = [&](int k) { return k % ins.G.width; };
  auto get_y = [&](int k) { return k / ins.G.height; };
  std::ofstream log;
  log.open(output_name, std::ios::out);
  log << "agents=" << ins.N << "\n";
  log << "map_file=" << map_recorded_name << "\n";
  log << "solver=planner\n";
  log << "solved=" << !solution.empty() << "\n";
  log << "soc=" << get_sum_of_costs(solution) << "\n";
  log << "makespan=" << get_makespan(solution) << "\n";
  log << "comp_time=" << comp_time_ms << "\n";
  log << "starts=";
  for (auto i = 0; i < ins.N; ++i) {
    auto k = ins.starts[i]->id;
    log << "(" << get_x(k) << "," << get_y(k) << "),";
  }
  log << "\ngoals=";
  for (auto i = 0; i < ins.N; ++i) {
    auto k = ins.goals[i]->id;
    log << "(" << get_x(k) << "," << get_y(k) << "),";
  }
  log << "\nsolution=\n";
  for (auto t = 0; t < solution.size(); ++t) {
    log << t << ":";
    auto C = solution[t];
    for (auto v : C) {
      log << "(" << get_x(v->id) << "," << get_y(v->id) << "),";
    }
    log << "\n";
  }
  log.close();
}
