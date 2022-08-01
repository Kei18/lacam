/*
 * graph definition
 */
#pragma once
#include "utils.hpp"

struct Vertex {
  const int id;     // index for V in Graph
  const int index;  // index for U, width * y + x, in Graph
  std::vector<Vertex*> neighbor;

  Vertex(int _id, int _index);
};
using Vertices = std::vector<Vertex*>;
using Config = std::vector<Vertex*>;  // a set of locations for all agents

struct Graph {
  Vertices V;  // without nullptr
  Vertices U;  // with nullptr
  int width;   // grid width
  int height;  // grid height
  Graph();
  Graph(const std::string& filename);  // taking map filename
  ~Graph();

  int size() const;  // the number of vertices
};

bool is_same_config(
    const Config& C1,
    const Config& C2);  // check equivalence of two configurations

// hash function of configuration
// c.f.
// https://stackoverflow.com/questions/10405030/c-unordered-map-fail-when-used-with-a-vector-as-key
struct ConfigHasher {
  int operator()(const Config& C) const;
};
