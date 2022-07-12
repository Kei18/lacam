#pragma once
#include <random>

#include "graph.hpp"
#include "utils.hpp"

struct Instance {
  const Graph G;
  Config starts;
  Config goals;
  const uint N;

  Instance(const std::string& map_filename,
           const std::vector<int>& start_indexes,
           const std::vector<int>& goal_indexes);
  Instance(const std::string& scen_filename, const std::string& map_filename,
           const int _N = 1);
  Instance(const std::string& scen_filename, std::mt19937* MT,
           const int _N = 1);
  ~Instance() {}

  bool is_valid(const int verbose = 0) const;
};

using Solution = std::vector<Config>;
