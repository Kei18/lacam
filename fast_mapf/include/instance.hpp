#pragma once
#include "graph.hpp"

struct Instance {
  Graph G;
  Config starts;
  Config goals;
  const int N;

  Instance(const std::string& scen_filename, const std::string& map_filename,
           const int _N = 1);
};
