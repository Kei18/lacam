#include "../include/dist_table.hpp"

#include <climits>

DistTable::DistTable(const Instance& ins)
    : K(ins.G.V.size()), table(ins.N, std::vector<int>(K, K))
{
  setup(&ins);
}

DistTable::DistTable(const Instance* ins)
    : K(ins->G.V.size()), table(ins->N, std::vector<int>(K, K))
{
  setup(ins);
}

void DistTable::setup(const Instance* ins)
{
  for (int i = 0; i < ins->N; ++i) {
    OPEN.push_back(std::queue<Vertex*>());
    auto n = ins->goals[i];
    OPEN[i].push(n);
    table[i][n->id] = 0;
  }
}

int DistTable::get(int i, int v_id)
{
  if (table[i][v_id] < K) return table[i][v_id];

  while (!OPEN[i].empty()) {
    auto n = OPEN[i].front();
    OPEN[i].pop();
    const int d_n = table[i][n->id];
    for (auto& m : n->neighbor) {
      const int d_m = table[i][m->id];
      if (d_n + 1 >= d_m) continue;
      table[i][m->id] = d_n + 1;
      OPEN[i].push(m);
    }
    if (n->id == v_id) return d_n;
  }
  return K;
}

int DistTable::get(int i, Vertex* v) { return get(i, v->id); }

int get_makespan_lower_bound(const Instance& ins, DistTable& dist_table)
{
  int c = 0;
  for (auto i = 0; i < ins.N; ++i) {
    c = std::max(c, dist_table.get(i, ins.starts[i]));
  }
  return c;
}

int get_sum_of_costs_lower_bound(const Instance& ins, DistTable& dist_table)
{
  int c = 0;
  for (auto i = 0; i < ins.N; ++i) {
    c += dist_table.get(i, ins.starts[i]);
  }
  return c;
}
