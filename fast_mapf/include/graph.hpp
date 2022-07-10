#pragma once
#include "utils.hpp"

struct Vertex {
  const int id;
  const int index;
  std::vector<Vertex*> neighbor;

  Vertex(int _id, int _index);
};
using Vertices = std::vector<Vertex*>;
using Config = std::vector<Vertex*>;

struct Graph {
  Vertices V;
  Vertices U;
  int width;
  int height;
  Graph();
  Graph(const std::string& filename);
  ~Graph();

  int size() const;
};

bool is_same_config(const Config& C1, const Config& C2);

// c.f.
// https://stackoverflow.com/questions/10405030/c-unordered-map-fail-when-used-with-a-vector-as-key
struct ConfigHasher {
  int operator()(const Config& C) const;
};
