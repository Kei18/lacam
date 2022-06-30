#pragma once
#include "graph.hpp"
#include "utils.hpp"

struct Instance {
  Graph G;
  Config starts;
  Config goals;
  const int N;

  Instance(const std::string& scen_filename, const std::string& map_filename,
           const int _N = 1);
};

using Solution = std::vector<Config>;
bool is_valid(const Instance& ins, const Solution& solution,
              const int verbose = 0);
