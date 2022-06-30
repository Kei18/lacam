#pragma once

#include "graph.hpp"
#include "instance.hpp"
#include "utils.hpp"

struct DistTable {
  std::vector<std::vector<int> > table;
  int get(int i, int v_id) const { return table[i][v_id]; }
  int get(int i, Vertex* v) const { return table[i][v->id]; }

  DistTable(const Instance& ins);
};
