#pragma once

#include "graph.hpp"
#include "instance.hpp"
#include "utils.hpp"

struct DistTable {
  std::vector<std::vector<int> > table;
  int get(int i, int v_id) const { return table[i][v_id]; }
  int get(int i, Vertex* v) const { return table[i][v->id]; }

  DistTable(const Instance& ins);
  DistTable(const Instance* ins);

  void setup(const Instance* ins);
};

int get_makespan_lower_bound(const Instance& ins, const DistTable& dist_table);
int get_sum_of_costs_lower_bound(const Instance& ins,
                                 const DistTable& dist_table);
