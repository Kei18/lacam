#pragma once
#include <vector>

struct Vertex {
  const int id;
  std::vector<Vertex*> neighbor;

  Vertex(int _id);
};
using Vertices = std::vector<Vertex*>;

struct Graph {
  Vertices V;
  Graph();
  ~Graph();
};

void load_graph(Graph& G, std::string& filename);
int get_num_vertices(const Graph& G);
