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
