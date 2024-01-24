#include <argparse/argparse.hpp>
#include <lacam.hpp>

int main(int argc, char* argv[])
{
  // arguments parser
  argparse::ArgumentParser program("lacam", "0.1.0");
  program.add_argument("-m", "--map").help("map file").required();                                              // map file
  program.add_argument("-ng", "--ngoals").help("number of goals").required();                                   // number of goals: agent first go to get goal, and then return to unloading port
  program.add_argument("-na", "--nagents").help("number of agents").required();                                 // number of agents
  program.add_argument("-s", "--seed").help("seed").default_value(std::string("0"));                            // random seed
  program.add_argument("-v", "--verbose").help("verbose").default_value(std::string("0"));                      // verbose
  program.add_argument("-t", "--time_limit_sec").help("time limit sec").default_value(std::string("10"));       // time limit (second)
  program.add_argument("-o", "--output").help("output file").default_value(std::string("./result/result.txt")); // output file
  program.add_argument("-l", "--log_short").default_value(false).implicit_value(true);

  try {
    program.parse_known_args(argc, argv);
  }
  catch (const std::runtime_error& err) {
    std::cerr << err.what() << std::endl;
    std::cerr << program;
    std::exit(1);
  }

  // setup instance
  const auto verbose = std::stoi(program.get<std::string>("verbose"));
  const auto time_limit_sec = std::stoi(program.get<std::string>("time_limit_sec"));
  const auto deadline = Deadline(time_limit_sec * 1000);
  const auto seed = std::stoi(program.get<std::string>("seed"));
  auto MT = std::mt19937(seed);
  const auto map_name = program.get<std::string>("map");
  const auto output_name = program.get<std::string>("output");
  const auto log_short = program.get<bool>("log_short");
  const auto ngoals = std::stoi(program.get<std::string>("ngoals"));
  const auto nagents = std::stoi(program.get<std::string>("nagents"));
  auto ins = Instance(map_name, &MT, nagents, ngoals);

  // check paras
  if (!ins.is_valid(1)) {
    std::cerr << "instance is invalid!" << std::endl;
    return 1;
  }
  if (nagents > ngoals) {
    std::cerr << "number of goals must larger or equal to number of agents" << std::endl;
    return 1;
  }

  // initliaze log system
  Log log;

  // solving
  int nagents_with_new_goals = 0;
  int step = 1;
  for (int i = 0; i < ngoals; i += nagents_with_new_goals) {
    // ternimal log
    std::cerr << "------------------------------------" << std::endl;
    std::cerr << "STEP " << step << std::endl;
    std::cerr << "Starts: " << ins.starts << std::endl;
    std::cerr << "Goals: " << ins.goals << std::endl;

    // statistics
    step++;

    auto solution = solve(ins, verbose - 1, &deadline, &MT);
    const auto comp_time_ms = deadline.elapsed_ms();

    // failure
    if (solution.empty()) {
      info(1, verbose, "failed to solve");
      return 1;
    }

    // update step solution
    if (!log.update_solution(solution)) {
      std::cerr << "Update step solution fails!" << std::endl;
      return 1;
    }

    // check feasibility
    if (!log.is_feasible_solution(ins, verbose)) {
      info(0, verbose, "invalid solution");
      return 1;
    }

    // post processing
    log.print_stats(verbose, ins, comp_time_ms);
    log.make_log(ins, output_name, comp_time_ms, map_name, seed, log_short);

    // assign new goals
    nagents_with_new_goals = ins.update_on_reaching_goals(solution, ngoals - i);
    std::cerr << "Reached Goals: " << nagents_with_new_goals << std::endl;
  }
  return 0;
}
