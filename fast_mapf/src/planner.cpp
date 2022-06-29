#include "../include/planner.hpp"

float get_cost(Config& C, const DistTable& dist_table)
{
  float cost = 0;
  const auto N = size(C);
  for (int i = 0; i < N; ++i) cost += dist_table.get(i, C[i]);
  return cost;
}

std::string get_id(Config& C)
{
  std::string id = "";
  for (auto v : C) id += std::to_string(v->id) + "-";
  return id;
}

std::vector<int> get_order(Config& C, const DistTable& dist_table)
{
  std::vector<int> A(size(C));
  std::iota(A.begin(), A.end(), 0);
  auto cmp = [&](int i, int j) {
    return dist_table.get(i, C[i]) > dist_table.get(j, C[j]);
  };
  std::sort(A.begin(), A.end(), cmp);
  return A;
}

Node::Node(Config _C, const DistTable& dist_table, Node* _parent)
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
  const auto N = ins.N;
  const auto K = size(ins.G.V);

  // create distance table
  const auto dist_table = DistTable(ins);

  // setup PIBT
  auto occupied_now = Agents(K, nullptr);
  auto occupied_next = Agents(K, nullptr);
  Agents A(N, nullptr);
  for (int i = 0; i < N; ++i) A[i] = new Agent(i);

  // setup search lists
  auto cmp = [](Node* a, Node* b) { return a->cost < b->cost; };
  std::priority_queue<Node*, Nodes, decltype(cmp)> OPEN(cmp);
  std::unordered_map<std::string, Node*> EXPLORED;
  std::vector<Constraint*> GC;  // garbage collection for constraint

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
    GC.push_back(M);
    S->search_tree.pop();
    if (M->depth < N) {
      auto i = S->order[M->depth];
      for (auto u : S->C[i]->neighbor)
        S->search_tree.push(new Constraint(M, i, u));
      S->search_tree.push(new Constraint(M, i, S->C[i]));
    }

    // create successor for high-level search (by PIBT)
    {
      bool invalid = false;
      // setup occupied_now
      for (auto a : A) {
        a->v_now = S->C[a->id];
        a->v_next = nullptr;
        occupied_now[a->id] = a;
      }
      // setup occupied_next
      for (int k = 0; k < M->depth; ++k) {
        const auto i = M->who[k];        // agent
        const auto l = M->where[k]->id;  // loc
        A[i]->v_next = M->where[k];
        // check vertex collision
        if (occupied_next[i] != nullptr) {
          invalid = true;
          break;
        }
        // check swap collision
        auto l_pre = S->C[i]->id;
        if (occupied_next[l_pre] != nullptr && occupied_now[l] != nullptr &&
            occupied_next[l_pre]->id == occupied_now[l]->id) {
          invalid = true;
          break;
        }
        // set occupied_next
        occupied_next[M->where[k]->id] = A[M->who[k]];
      }
      if (invalid) continue;

      // run PIBT
      for (auto k : S->order) {
        auto a = A[k];
        if (a->v_next == nullptr) {
          // funcPIBT(a, nullptr, occupied_now, occupied_next, dist_table);
        }
      }
    }
    break;
  }

  // memory management
  for (auto a : A) delete a;
  for (auto M : GC) delete M;
  for (auto p : EXPLORED) delete p.second;
}
