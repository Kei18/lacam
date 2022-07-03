#include <argparse/argparse.hpp>
#include <fast_mapf.hpp>

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
  program.add_argument("-o", "--output")
      .help("output file")
      .default_value(std::string("./build/result.txt"));

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
  const auto output_name = program.get<std::string>("output");
  const auto N = std::stoi(program.get<std::string>("num"));
  const auto ins = Instance(scen_name, map_name, N);
  if (!ins.is_valid(verbose)) return 1;

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

  // post processing
  print_stats(verbose, ins, solution, comp_time_ms);
  make_log(ins, solution, output_name, comp_time_ms, map_name);
  return 0;
}
