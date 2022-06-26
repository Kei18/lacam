#include "../include/planner.hpp"

void load_dist_table(DistTable& dist_table, const Instance& ins)
{
  for (int i = 0; i < ins.N; ++i) {
    // breadth first search
    std::queue<Vertex*> OPEN;
    auto n = ins.goals[i];
    OPEN.push(n);
    dist_table[i][n->id] = 0;
    while (!OPEN.empty()) {
      n = OPEN.front();
      OPEN.pop();
      const int d_n = dist_table[i][n->id];
      for (auto m : n->neighbor) {
        const int d_m = dist_table[i][m->id];
        if (d_n + 1 >= d_m) continue;
        dist_table[i][m->id] = d_n + 1;
        OPEN.push(m);
      }
    }
  }
}

void solve(const Instance& ins)
{
  const auto K = size(ins.G.V);
  auto dist_table = DistTable(ins.N, std::vector<int>(K, K));
  load_dist_table(dist_table, ins);
}
