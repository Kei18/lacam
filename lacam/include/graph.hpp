/*
 * graph definition
 */
#pragma once
#include "utils.hpp"

struct Vertex {
  const int id;     // index for V in Graph
  const int index;  // index for U (width * y + x) in Graph
  std::vector<Vertex*> neighbor;

  Vertex(int _id, int _index);
};

struct CargoVertex {
  const int index;  // index for cargo (width * y + x) in Graph
  CargoVertex(int _index);
};

using Vertices = std::vector<Vertex*>;
using CargoVertices = std::vector<CargoVertex*>;
using TargetVertices = std::unordered_set<Vertex*>;
using Config = std::vector<Vertex*>;  // locations for all agents

struct Graph {
  Vertices V;  // without nullptr
  Vertices U;  // with nullptr, i.e., |U| = width * height
  CargoVertices C;
  TargetVertices G;  // with nullptr, goal candidate
  int width;   // grid width
  int height;  // grid height
  std::mt19937* randomSeed;

  Graph();
  Graph(const std::string& filename, std::mt19937* _randomSeed = 0);
  ~Graph();

  int size() const;  // the number of vertices, |V|
  Vertex* random_target_vertex();
};

bool is_same_config(
  const Config& C1,
  const Config& C2);  // check equivalence of two configurations

// hash function of configuration
// c.f.
// https://stackoverflow.com/questions/10405030/c-unordered-map-fail-when-used-with-a-vector-as-key
struct ConfigHasher {
  uint operator()(const Config& C) const;
};

std::ostream& operator<<(std::ostream& os, const Vertex* v);
