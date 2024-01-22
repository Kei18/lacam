/*
 * instance definition
 */
#pragma once
#include <random>

#include "graph.hpp"
#include "utils.hpp"

struct Instance {
  Graph G;            // graph
  Config starts;            // initial configuration
  Config goals;             // goal configuration
  Config unloading_port;    // delivery outlet configuration
  const uint nagents;       // number of agents
  const uint ngoals;        // number if goals

  Instance(const std::string& map_filename, std::mt19937* MT, const int _nagents = 1, const int _ngoals = 1);
  ~Instance() {}

  // simple feasibility check of instance
  bool is_valid(const int verbose = 0) const;
};

// solution: a sequence of configurations
using Solution = std::vector<Config>;
