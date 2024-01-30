/*
 * instance definition
 */
#pragma once
#include <random>

#include "graph.hpp"
#include "utils.hpp"

struct Instance {
  Graph graph;    // graph
  Config starts;  // initial configuration
  Config goals;   // goal configuration, can be in warehouse block/cache block
  Config cargo_goals;  // cargo goal configuration
  // Status control:
  // 0 -> cache miss, go for warehouse get cargo
  // 1 -> cache hit, go for cache get cargo
  // 2 -> warehouse get cargo, find empty block, go back to insert cache
  // 3 -> warehouse get cargo, cannot find empty block, directly back to
  // unloading port
  //   -> cache get cargo, go back to unloading port
  std::vector<uint> bit_status;
  const uint nagents;  // number of agents
  const uint ngoals;   // number if goals

  std::shared_ptr<spdlog::logger> logger;

  Instance(const std::string& map_filename, std::mt19937* MT,
           std::shared_ptr<spdlog::logger> _logger, const int _nagents = 1,
           const int _ngoals = 1);
  ~Instance() {}

  // simple feasibility check of instance
  bool is_valid(const int verbose = 0) const;

  // check first batch of agents reach goals
  uint update_on_reaching_goals(std::vector<Config>& vertex_list,
                                int remain_goals, uint& cache_access,
                                uint& cache_hit);
};

// solution: a sequence of configurations
using Solution = std::vector<Config>;
