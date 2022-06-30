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
  ~Graph();
};

void load_graph(Graph& G, const std::string& filename);
int get_num_vertices(const Graph& G);
bool is_same_config(const Config& C1, const Config& C2);
