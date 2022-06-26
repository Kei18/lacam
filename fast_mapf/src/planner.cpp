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

float get_cost(Config& C, DistTable& dist_table)
{
  float cost = 0;
  const auto N = size(C);
  for (int i = 0; i < N; ++i) cost += dist_table[i][C[i]->id];
  return cost;
}

std::string get_id(Config& C)
{
  std::string id = "";
  for (auto v : C) id += std::to_string(v->id) + "-";
  return id;
}

Node::Node(Config _C, DistTable& dist_table, Node* _parent)
    : C(_C), cost(get_cost(_C, dist_table)), id(get_id(_C)), parent(_parent)
{
}

bool is_same_config(const Config& C1, const Config& C2)
{
  const auto N = size(C1);
  for (int i = 0; i < N; ++i) {
    if (C1[i]->id != C2[i]->id) return false;
  }
  return true;
}

void solve(const Instance& ins)
{
  // create distance table
  const auto K = size(ins.G.V);
  auto dist_table = DistTable(ins.N, std::vector<int>(K, K));
  load_dist_table(dist_table, ins);

  // setup search lists
  auto cmp = [](Node* a, Node* b) { return a->cost > b->cost; };
  std::priority_queue<Node*, Nodes, decltype(cmp)> OPEN(cmp);
  std::unordered_map<std::string, Node*> EXPLORED;

  // insert initial node
  auto S = new Node(ins.starts, dist_table);
  OPEN.push(S);
  EXPLORED[S->id] = S;

  // best first search
  bool solved = false;
  while (!OPEN.empty()) {
    // do not pop here!
    S = OPEN.top();

    // check goal condition
    if (is_same_config(S->C, ins.goals)) {
      solved = true;
      break;
    }

    // expand
  }

  // memory management
  for (auto p : EXPLORED) delete p.second;
}
