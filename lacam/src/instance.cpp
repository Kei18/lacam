#include "../include/instance.hpp"

Instance::Instance(
  const std::string& map_filename,
  std::mt19937* MT,
  std::shared_ptr<spdlog::logger> _logger,
  uint goals_m,
  uint goals_k,
  CacheType cache_type,
  const uint _nagents,
  const uint _ngoals)
  : graph(Graph(map_filename, _logger, goals_m, goals_k, _ngoals, cache_type, MT)),
  starts(Config()),
  goals(Config()),
  nagents(_nagents),
  ngoals(_ngoals),
  logger(std::move(_logger))
{
  const auto K = graph.size();

  // set agents random start potition
  auto s_indexes = std::vector<int>(K);
  std::iota(s_indexes.begin(), s_indexes.end(), 0);
  std::shuffle(s_indexes.begin(), s_indexes.end(), *MT);
  int i = 0;
  while (true) {
    if (i >= K) return;
    starts.push_back(graph.V[s_indexes[i]]);
    if (starts.size() == nagents) break;
    ++i;
  }

  // set goals
  int j = 0;
  while (true) {
    if (j >= K) return;
    Vertex* goal = graph.get_next_goal();
    goals.push_back(goal);
    cargo_goals.push_back(goal);
    bit_status.push_back(0);      // At the begining, the cache is empty, all agents should at status 0
    cargo_cnts.push_back(0);
    if (goals.size() == nagents) break;
    ++j;
  }
}

bool Instance::is_valid(const int verbose) const
{
  if (nagents != starts.size() || nagents != goals.size()) {
    info(1, verbose, "invalid N, check instance");
    return false;
  }
  return true;
}

uint Instance::update_on_reaching_goals_with_cache(
  std::vector<Config>& vertex_list,
  int remain_goals,
  uint& cache_access,
  uint& cache_hit)
{
  logger->debug("Remain goals:  {}", remain_goals);
  int step = vertex_list.size() - 1;
  logger->debug("Step length:   {}", step);
  logger->debug("Solution ends: {}", vertex_list[step]);
  int reached_count = 0;

  // TODO: assign goals to closed free agents

  // Update steps
  for (size_t i = 0; i < vertex_list[step].size(); i++) {
    cargo_cnts[i] += step;
  }

  // First, we check vertex which status will released lock
  for (size_t j = 0; j < vertex_list[step].size(); ++j) {
    if (vertex_list[step][j] == goals[j]) {
      // Status 1 finished. ==> Status 4
      // Agent has moved to cache cargo target.
      // Update cache lock info, directly move back to unloading port.
      if (bit_status[j] == 1) {
        logger->debug(
          "Agent {} status 1 -> status 4, reach cached cargo {} at cache "
          "block {}, return to unloading port",
          j, *cargo_goals[j], *goals[j]);
        bit_status[j] = 4;
        graph.cache->update_cargo_from_cache(cargo_goals[j], goals[j]);
      }
      // Status 2 finished. ==> Status 4
      // Agent has bring uncached cargo back to cache.
      // Update cache, move to unloading port.
      else if (bit_status[j] == 2) {
        logger->debug(
          "Agent {} status 2 -> status 4, bring cargo {} to cache block "
          "{}, then return to unloading port",
          j, *cargo_goals[j], *goals[j]);
        bit_status[j] = 4;
        graph.cache->update_cargo_into_cache(cargo_goals[j], goals[j]);
      }

      // Update goals and steps
      goals[j] = graph.unloading_ports[0];
    }
  }

  // Second, we check vertex which status will require (or not require) lock
  for (size_t j = 0; j < vertex_list[step].size(); ++j) {
    if (bit_status[j] == 0) {
      // Status 0 finished.
      if (vertex_list[step][j] == goals[j]) {
        // Agent has moved to warehouse cargo target
        Vertex* goal = graph.cache->try_insert_cache(cargo_goals[j], graph.unloading_ports[0]);
        // Cache is full, directly get back to unloading port.
        // ==> Status 3
        if (goal == graph.unloading_ports[0]) {
          logger->debug(
            "Agent {} status 0 -> status 3, reach warehouse cargo {}, cache "
            "is full, go back to unloading port",
            j, *cargo_goals[j]);
          bit_status[j] = 3;
        }
        // Find empty cache block, go and insert cargo into cache, -> Status 2
        else {
          logger->debug(
            "Agent {} status 0 -> status 2, reach warehouse cargo {}, find "
            "cache block to insert, go to cache block {}",
            j, *cargo_goals[j], *goal);
          bit_status[j] = 2;
        }
        // Update goals and steps
        goals[j] = goal;
      }
      // Status 0 yet not finished
      else {
        // Check if the cargo has been cached during the period
        Vertex* goal = graph.cache->try_cache_cargo(cargo_goals[j]);
        if (goal != cargo_goals[j]) {
          // We find cached cargo, go to cache
          // ==> Status 1
          logger->debug(
            "Agent {} cache hit while moving to ware house to get cargo. Go to cache {}, status 0 -> status 1",
            j, *cargo_goals[j], *goal);
          cache_access++;
          cache_hit++;
          bit_status[j] = 1;
          // Update goals and steps
          goals[j] = goal;
        }
        // Otherwise, we do nothing for cache miss situation
      }
    }
    else if (bit_status[j] == 3) {
      // Status 3 finished.
      // Agent has back to unloading port, assigned with new cargo target
      if (vertex_list[step][j] == goals[j]) {
        if (remain_goals > 0) {
          // Update statistics.
          // Otherwise we still let agent go to fetch new cargo, but we do
          // not update the statistics.
          remain_goals--;
          reached_count++;
          // Record finished cargo steps
          cargo_steps.push_back(cargo_cnts[j]);
          cargo_cnts[j] = 0;
        }

        logger->debug("Agent {} has bring cargo {} to unloading port", j, *cargo_goals[j]);

        // Generate new cargo goal
        Vertex* cargo = graph.get_next_goal();
        cargo_goals[j] = cargo;
        Vertex* goal = graph.cache->try_cache_cargo(cargo);

        // Cache hit, go to cache to get cached cargo
        // ==> Status 1
        if (cargo != goal) {
          logger->debug(
            "Agent {} assigned with new cargo {}, cache hit. Go to cache {}, "
            "status 3 -> status 1",
            j, *cargo_goals[j], *goal);
          cache_access++;
          cache_hit++;
          bit_status[j] = 1;
        }
        // Cache miss, go to warehouse to get cargo
        // ==> Status 0
        else {
          logger->debug(
            "Agent {} assigned with new cargo {}, cache miss. Go to "
            "warehouse, status 3 -> status 0",
            j, *cargo_goals[j]);
          cache_access++;
          bit_status[j] = 0;
        }
        // update goal
        goals[j] = goal;
      }
      // Agent has yet not back to unloading port, we check if there is an empty
      // cache block to insert
      else {
        Vertex* goal = graph.cache->try_insert_cache(cargo_goals[j], graph.unloading_ports[0]);
        // Check if the cache is available during the period
        if (goal != graph.unloading_ports[0]) {
          logger->debug(
            "Agent {} status 3 -> status 2, find cache block to insert during the moving, go to cache block {}",
            j, *cargo_goals[j], *goal);
          bit_status[j] = 2;
          goals[j] = goal;
        }
        // Otherwise, we do nothing if the cache miss
      }
    }
    else if (bit_status[j] == 4) {
      // Status 4 finished.
      // We only check status 4 if it is finished
      if (vertex_list[step][j] == goals[j]) {
        if (remain_goals > 0) {
          // Update statistics.
          // Otherwise we still let agent go to fetch new cargo, but we do
          // not update the statistics.
          remain_goals--;
          reached_count++;
          // Record finished cargo steps
          cargo_steps.push_back(cargo_cnts[j]);
          cargo_cnts[j] = 0;
        }

        logger->debug("Agent {} has bring cargo {} to unloading port", j, *cargo_goals[j]);

        // Generate new cargo goal
        Vertex* cargo = graph.get_next_goal();
        cargo_goals[j] = cargo;
        Vertex* goal = graph.cache->try_cache_cargo(cargo);

        // Cache hit, go to cache to get cached cargo
        // ==> Status 1
        if (cargo != goal) {
          logger->debug(
            "Agent {} assigned with new cargo {}, cache hit. Go to cache {}, "
            "status 4 -> status 1",
            j, *cargo_goals[j], *goal);
          cache_access++;
          cache_hit++;
          bit_status[j] = 1;
        }
        // Cache miss, go to warehouse to get cargo
        // ==> Status 0
        else {
          logger->debug(
            "Agent {} assigned with new cargo {}, cache miss. Go to "
            "warehouse, status 4 -> status 0",
            j, *cargo_goals[j]);
          cache_access++;
          bit_status[j] = 0;
        }
        // update goal
        goals[j] = goal;
      }
    }
  }

  starts = vertex_list[step];
  logger->debug("Ends: {}", starts);
  return reached_count;
}

uint Instance::update_on_reaching_goals_without_cache(
  std::vector<Config>& vertex_list,
  int remain_goals)
{
  logger->debug("Remain goals:  {}", remain_goals);
  int step = vertex_list.size() - 1;
  logger->debug("Step length:   {}", step);
  logger->debug("Solution ends: {}", vertex_list[step]);
  int reached_count = 0;

  for (size_t j = 0; j < vertex_list[step].size(); ++j) {
    // Update steps
    cargo_cnts[j] += step;
    if (vertex_list[step][j] == goals[j]) {
      if (goals[j] == graph.unloading_ports[0]) {
        if (remain_goals > 0) {
          // Update statistics.
          // Otherwise we still let agent go to fetch new cargo, but we do
          // not update the statistics.
          remain_goals--;
          reached_count++;
          // Recorded finished cargo steps
          cargo_steps.push_back(cargo_cnts[j]);
          cargo_cnts[j] = 0;
        }
        goals[j] = graph.get_next_goal();
      }
      else {
        goals[j] = graph.unloading_ports[0];
      }
    }
  }

  starts = vertex_list[step];
  logger->debug("Ends: {}", starts);
  return reached_count;
}

std::vector<uint> Instance::compute_percentiles() const {
  logger->debug("cargo step size: {}", cargo_steps.size());
  std::vector<uint> sorted_steps(cargo_steps);

  // Calculate indices for the percentiles
  auto idx = [&](double p) -> size_t {
    return static_cast<size_t>(std::floor((p * sorted_steps.size()) / 100.0));
    };

  std::vector<uint> percentiles;
  std::vector<double> required_percentiles = { 0, 25, 50, 75, 90, 95, 99, 100 };
  for (double p : required_percentiles) {
    size_t i = idx(p);
    // Partially sort to find the ith smallest element (percentile)
    std::nth_element(sorted_steps.begin(), sorted_steps.begin() + i, sorted_steps.end());
    percentiles.push_back(sorted_steps[i]);
  }

  return percentiles;
}