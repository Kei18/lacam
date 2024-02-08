/*
 * instance definition
 */
#pragma once
#include <random>

#include "graph.hpp"
#include "utils.hpp"

struct Instance {
  Graph graph;          // graph
  Config starts;        // initial configuration
  Config goals;         // goal configuration, can be in warehouse block/cache block
  Config cargo_goals;   // cargo goal configuration

  // Status control:
  // 0 -> cache miss, going for warehouse get cargo
  // 1 -> cache hit, going for cache get cargo
  // 2 -> warehouse get cargo, find empty block, going back to insert cache
  // 3 -> warehouse get cargo, cannot find empty block, going back to unloading port
  // 4 -> cache get cargo, going back to unloading port
  std::vector<uint> bit_status;

  const uint nagents;   // number of agents
  const uint ngoals;    // number of goals

  std::shared_ptr<spdlog::logger> logger;

  // Instructor
  Instance(
    const std::string& map_filename,
    std::mt19937* MT,
    std::shared_ptr<spdlog::logger> _logger,
    uint goals_m,
    uint goals_k,
    CacheType cache_type,
    const uint _nagents = 1,
    const uint _ngoals = 1
  );
  // Destructor
  ~Instance() {}

  // Simple feasibility check of instance
  bool is_valid(const int verbose = 0) const;

  // Check agents when reaching goals with cache
  uint update_on_reaching_goals_with_cache(
    std::vector<Config>& vertex_list,
    int remain_goals,
    uint& cache_access,
    uint& cache_hit
  );

  // Check agents when reaching goals without cache
  uint update_on_reaching_goals_without_cache(
    std::vector<Config>& vertex_list,
    int remain_goals
  );
};
