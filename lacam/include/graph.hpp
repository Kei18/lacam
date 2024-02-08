/*
 * graph definition
 */
#pragma once
#include "utils.hpp"
#include "cache.hpp"

struct Graph {
  Vertices V;                             // without nullptr
  Vertices U;                             // with nullptr, i.e., |U| = width * height
  Vertices unloading_ports;               // unloading port
  Cache* cache;                            // cache
  Vertices cargo_vertices;
  Vertices goals_list;                    // goals list: length [ngoals], maximum [k] different goals in any [m] length sublist 

  int width;                              // grid width
  int height;                             // grid height
  uint goals_cnt = 0;                     // used for get next goals
  std::mt19937* randomSeed;

  std::shared_ptr<spdlog::logger> logger;

  // Instructor with cache
  Graph(const std::string& filename, std::shared_ptr<spdlog::logger> _logger, uint goals_m, uint goals_k, uint ngoals, CacheType cache_type, std::mt19937* _randomSeed = 0);
  // Destructor
  ~Graph();

  int size() const;                       // the number of vertices, |V|
  Vertex* random_target_vertex();
  void fill_goals_list(uint goals_m, uint goals_k, uint ngoals);
  Vertex* get_next_goal();
};

bool is_same_config(const Config& C1, const Config& C2);          // Check equivalence of two configurations
bool is_reach_at_least_one(const Config& C1, const Config& C2);   // Check if the solution reached at least one goal

// hash function of configuration
// c.f.
// https://stackoverflow.com/questions/10405030/c-unordered-map-fail-when-used-with-a-vector-as-key
struct ConfigHasher {
  uint operator()(const Config& C) const;
};
