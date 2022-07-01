#include <AvailabilityInternal.h>

#include <argparse/argparse.hpp>
#include <fstream>
#include <iostream>
#include <planner.hpp>
#include <random>

int main(int argc, char* argv[])
{
  argparse::ArgumentParser program("fast_mapf", "0.1.0");
  program.add_argument("-m", "--map").help("map file").required();
  program.add_argument("-i", "--scen").help("scenario file").required();
  program.add_argument("-N", "--num").help("number of agents").required();
  program.add_argument("-s", "--seed")
      .help("seed")
      .default_value(std::string("0"));
  program.add_argument("-v", "--verbose")
      .help("verbose")
      .default_value(std::string("0"));
  program.add_argument("-t", "--time_limit_sec")
      .help("time limit sec")
      .default_value(std::string("10"));

  try {
    program.parse_args(argc, argv);
  } catch (const std::runtime_error& err) {
    std::cerr << err.what() << std::endl;
    std::cerr << program;
    std::exit(1);
  }

  // read instance
  const auto verbose = std::stoi(program.get<std::string>("verbose"));
  const auto time_limit_sec =
      std::stoi(program.get<std::string>("time_limit_sec"));
  const auto scen_name = program.get<std::string>("scen");
  const auto seed = std::stoi(program.get<std::string>("seed"));
  const auto map_name = program.get<std::string>("map");
  const auto N = std::stoi(program.get<std::string>("num"));
  const auto ins = Instance(scen_name, map_name, N);
  if (!is_valid_instance(ins, verbose)) return 1;

  // solve
  auto MT = std::mt19937(seed);
  const auto deadline = Deadline(time_limit_sec * 1000);
  const auto solution = solve(ins, verbose - 1, &deadline, &MT);
  const auto comp_time_ms = deadline.elapsed_ms();

  // failure
  if (solution.empty()) {
    info(1, verbose, "failed to solve");
    return 0;
  }
  // check feasibility
  if (!is_feasible_solution(ins, solution, verbose)) return 1;

  // compute metrics
  const auto dist_table = DistTable(ins);
  const auto makespan = get_makespan(solution);
  const auto makespan_lb = get_makespan_lower_bound(ins, dist_table);
  const auto sum_of_costs = get_sum_of_costs(solution);
  const auto sum_of_costs_lb = get_sum_of_costs_lower_bound(ins, dist_table);
  info(1, verbose, "solved: ", comp_time_ms, "ms", "\tmakespan: ", makespan,
       " (lb=", makespan_lb, ", sub-opt-ub=", (double)makespan / makespan_lb,
       ")", "\tsum_of_costs: ", sum_of_costs, " (lb=", sum_of_costs_lb,
       ", sub-opt-ub=", (double)sum_of_costs / sum_of_costs_lb, ")");

  // log
  auto get_x = [&](int k) { return k % ins.G.width; };
  auto get_y = [&](int k) { return k / ins.G.height; };
  const std::string log_file = "result.txt";
  std::ofstream log;
  log.open(log_file, std::ios::out);
  log << "agents=" << ins.N << "\n";
  log << "map_file=" << map_name.substr(11) << "\n";  // TODO
  log << "solver=planner\n";
  log << "solved=" << !solution.empty() << "\n";
  log << "soc=" << sum_of_costs << "\n";
  log << "makespan=" << makespan << "\n";
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

  return 0;
}
