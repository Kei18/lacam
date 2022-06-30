#include "../include/instance.hpp"

#include <fstream>
#include <iostream>
#include <regex>

const std::regex r_instance =
    std::regex(R"(\d+\t.+\.map\t\d+\t\d+\t(\d+)\t(\d+)\t(\d+)\t(\d+)\t.+)");

Instance::Instance(const std::string& scen_filename,
                   const std::string& map_filename, const int _N)
    : G(Graph()), starts(Config()), goals(Config()), N(_N)
{
  // load graph
  load_graph(G, map_filename);

  // load start-goal
  std::ifstream file(scen_filename);
  if (!file) {
    std::cout << "file " << scen_filename << " is not found." << std::endl;
    return;
  }
  std::string line;
  std::smatch results;

  while (getline(file, line)) {
    // for CRLF coding
    if (*(line.end() - 1) == 0x0d) line.pop_back();

    if (std::regex_match(line, results, r_instance)) {
      auto y_s = std::stoi(results[1].str());
      auto x_s = std::stoi(results[2].str());
      auto y_g = std::stoi(results[3].str());
      auto x_g = std::stoi(results[4].str());
      auto s = G.V[G.width * y_s + x_s];
      auto g = G.V[G.width * y_g + x_g];
      if (s == nullptr || g == nullptr) continue;
      starts.push_back(s);
      goals.push_back(g);
    }

    if (size(starts) == N) break;
  }
}

bool is_valid(const Instance& ins, const Solution& solution, const int verbose)
{
  if (solution.empty()) {
    if (verbose > 0) std::cout << "empty solution" << std::endl;
    return false;
  }

  // check start
  if (!is_same_config(solution.front(), ins.starts)) {
    if (verbose > 0) std::cout << "invalid starts" << std::endl;
    return false;
  }

  // check goal
  if (!is_same_config(solution.back(), ins.goals)) {
    if (verbose > 0) std::cout << "invalid goals" << std::endl;
    return false;
  }

  for (auto t = 1; t < size(solution); ++t) {
    for (auto i = 0; i < ins.N; ++i) {
      auto v_i_from = solution[t - 1][i];
      auto v_i_to = solution[t][i];
      // check connectivity
      if (v_i_from != v_i_to &&
          std::find(v_i_to->neighbor.begin(), v_i_to->neighbor.end(),
                    v_i_from) == v_i_to->neighbor.end()) {
        if (verbose > 0) std::cout << "invalid move" << std::endl;
        return false;
      }

      // check conflicts
      for (auto j = i + 1; j < ins.N; ++j) {
        auto v_j_from = solution[t - 1][j];
        auto v_j_to = solution[t][j];
        // vertex conflicts
        if (v_j_to == v_i_to) {
          if (verbose > 0) {
            std::cout << "vertex conflict at timestep-" << t
                      << ", vertex:" << v_i_to->id << ", agents: " << i
                      << " <-> " << j << std::endl;
          }
          return false;
        }
        // swap conflicts
        if (v_j_to == v_i_from && v_j_from == v_i_to) {
          if (verbose > 0) {
            std::cout << "edge conflict at timestep-" << t
                      << ", vertex:" << v_i_from->id << " <-> " << v_i_to->id
                      << ", agents: " << i << " <-> " << j << std::endl;
          }
          return false;
        }
      }
    }
  }

  return true;
}
