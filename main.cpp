#include <argparse/argparse.hpp>
#include <iostream>
#include <planner.hpp>

int main(int argc, char* argv[])
{
  argparse::ArgumentParser program("fast_mapf", "0.1.0");
  program.add_argument("-m", "--map").help("map file").required();
  program.add_argument("-s", "--scen").help("scenario file").required();
  program.add_argument("-N", "--num").help("number of agents").required();

  try {
    program.parse_args(argc, argv);
  } catch (const std::runtime_error& err) {
    std::cerr << err.what() << std::endl;
    std::cerr << program;
    std::exit(1);
  }

  const auto scen_name = program.get<std::string>("scen");
  const auto map_name = program.get<std::string>("map");
  const auto N = std::stoi(program.get<std::string>("num"));

  const auto ins = Instance(scen_name, map_name, N);
  const auto solution = solve(ins);
  std::cout << is_valid(ins, solution) << std::endl;

  return 0;
}
