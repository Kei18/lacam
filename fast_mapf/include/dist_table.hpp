#pragma once

#include "graph.hpp"
#include "instance.hpp"
#include "utils.hpp"

struct DistTable {
  std::vector<std::vector<int> > table;
  std::vector<std::queue<Vertex*> > OPEN;

  int get(int i, int v_id);
  int get(int i, Vertex* v);

  DistTable(const Instance& ins);
  DistTable(const Instance* ins);

  void setup(const Instance* ins);
};

int get_makespan_lower_bound(const Instance& ins, DistTable& D);
int get_sum_of_costs_lower_bound(const Instance& ins, DistTable& D);
