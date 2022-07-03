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

  bool is_valid(const int verbose = 0) const;
};

using Solution = std::vector<Config>;
