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

std::vector<int> get_order(Config& C, DistTable& dist_table)
{
  std::vector<int> A(size(C));
  std::iota(A.begin(), A.end(), 0);
  auto cmp = [&](int i, int j) {
    return dist_table[i][C[i]->id] < dist_table[j][C[j]->id];
  };
  std::sort(A.begin(), A.end(), cmp);
  return A;
}

Node::Node(Config _C, DistTable& dist_table, Node* _parent)
    : C(_C),
      cost(get_cost(_C, dist_table)),
      id(get_id(_C)),
      parent(_parent),
      order(get_order(_C, dist_table)),
      search_tree(std::queue<Constraint*>())
{
  search_tree.push(new Constraint());
}

Node::~Node()
{
  while (!search_tree.empty()) {
    delete search_tree.front();
    search_tree.pop();
  }
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

    // search end
    if (S->search_tree.empty()) {
      OPEN.pop();
      continue;
    }

    // create successor for low-level search
    auto M = S->search_tree.front();
    S->search_tree.pop();
    if (M->depth < ins.N) {
      auto i = S->order[M->depth];
      for (auto u : S->C[i]->neighbor)
        S->search_tree.push(new Constraint(M, i, u));
      S->search_tree.push(new Constraint(M, i, S->C[i]));
    }
    // create successor for high-level search (by PIBT)

    delete M;
    break;
  }

  // memory management
  for (auto p : EXPLORED) delete p.second;
}
