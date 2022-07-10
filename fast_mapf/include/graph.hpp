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
