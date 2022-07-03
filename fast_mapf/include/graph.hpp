#pragma once
#include "utils.hpp"

struct Vertex {
  const int id;
  std::vector<Vertex*> neighbor;

  Vertex(int _id);
};
using Vertices = std::vector<Vertex*>;
using Config = std::vector<Vertex*>;

struct Graph {
  Vertices V;
  int width;
  int height;
  Graph();
  Graph(const std::string& filename);
  ~Graph();

  int size() const;
  int get_num_vertices() const;
};

bool is_same_config(const Config& C1, const Config& C2);
