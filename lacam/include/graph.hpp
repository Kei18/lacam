/*
 * graph definition
 */
#pragma once
#include "utils.hpp"
#include "cache.hpp"

struct Graph {
  Vertices V;               // without nullptr
  Vertices U;               // with nullptr, i.e., |U| = width * height
  Vertices unloading_ports; // unloading port
  Cache cache;              // cache
  CargoVertices C;
  TargetVertices G;         // with nullptr, goal candidate
  int width;                // grid width
  int height;               // grid height
  std::mt19937* randomSeed;

  std::shared_ptr<spdlog::logger> logger;

  Graph(std::shared_ptr<spdlog::logger> _logger);
  Graph(const std::string& filename, std::shared_ptr<spdlog::logger> _logger, std::mt19937* _randomSeed = 0);
  ~Graph();

  int size() const;    // the number of vertices, |V|
  Vertex* random_target_vertex();
};

bool is_same_config(const Config& C1, const Config& C2);  // check equivalence of two configurations

bool is_reach_at_least_one(const Config& C1, const Config& C2);

// hash function of configuration
// c.f.
// https://stackoverflow.com/questions/10405030/c-unordered-map-fail-when-used-with-a-vector-as-key
struct ConfigHasher {
  uint operator()(const Config& C) const;
};
