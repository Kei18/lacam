#include "../include/dist_table.hpp"

DistTable::DistTable(const Instance& ins)
    : table(ins.N, std::vector<int>(size(ins.G.V), INT_MAX))
{
  for (int i = 0; i < ins.N; ++i) {
    // breadth first search
    std::queue<Vertex*> OPEN;
    auto n = ins.goals[i];
    OPEN.push(n);
    table[i][n->id] = 0;
    while (!OPEN.empty()) {
      n = OPEN.front();
      OPEN.pop();
      const int d_n = table[i][n->id];
      for (auto m : n->neighbor) {
        const int d_m = table[i][m->id];
        if (d_n + 1 >= d_m) continue;
        table[i][m->id] = d_n + 1;
        OPEN.push(m);
      }
    }
  }
}
