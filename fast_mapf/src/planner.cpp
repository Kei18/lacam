#include "../include/planner.hpp"

#include <algorithm>

float get_cost(Config& C, const DistTable& dist_table)
{
  float cost = 0;
  const auto N = size(C);
  for (auto i = 0; i < N; ++i) cost += dist_table.get(i, C[i]);
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

Node::Node(Config _C, const DistTable& dist_table, std::string _id = "",
           Node* _parent = nullptr)
    : C(_C),
      cost(get_cost(_C, dist_table)),
      id(_id == "" ? get_id(_C) : _id),
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

Solution solve(const Instance& ins)
{
  const auto N = ins.N;
  const auto K = size(ins.G.V);

  // create distance table
  const auto dist_table = DistTable(ins);

  // setup PIBT
  auto occupied_now = Agents(K, nullptr);
  auto occupied_next = Agents(K, nullptr);
  Agents A(N, nullptr);
  for (auto i = 0; i < N; ++i) A[i] = new Agent(i);

  // setup search lists
  auto cmp = [](Node* a, Node* b) { return a->cost > b->cost; };
  std::priority_queue<Node*, Nodes, decltype(cmp)> OPEN(cmp);
  std::unordered_map<std::string, Node*> EXPLORED;
  std::vector<Constraint*> GC;  // garbage collection for constraint

  // insert initial node
  auto S = new Node(ins.starts, dist_table);
  OPEN.push(S);
  EXPLORED[S->id] = S;

  // best first search
  int loop_cnt = 0;
  std::vector<Config> solution;

  while (!OPEN.empty()) {
    loop_cnt += 1;

    // do not pop here!
    S = OPEN.top();

    // check goal condition
    if (is_same_config(S->C, ins.goals)) {
      // backtrack
      while (S != nullptr) {
        solution.push_back(S->C);
        S = S->parent;
      }
      std::reverse(solution.begin(), solution.end());
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
      // move to neighbor
      for (auto u : S->C[i]->neighbor)
        S->search_tree.push(new Constraint(M, i, u));
      // stay motion
      S->search_tree.push(new Constraint(M, i, S->C[i]));
    }

    // create successor for high-level search by PIBT
    {
      // setup occupied_now
      for (auto a : A) {
        // clear previous cache
        if (a->v_now != nullptr && occupied_now[a->v_now->id] == a) {
          occupied_now[a->v_now->id] = nullptr;
        }
        if (a->v_next != nullptr) {
          occupied_next[a->v_next->id] = nullptr;
          a->v_next = nullptr;
        }

        // set occupied now
        a->v_now = S->C[a->id];
        occupied_now[a->id] = a;
      }

      // setup constraint
      bool invalid = false;
      for (auto k = 0; k < M->depth; ++k) {
        const auto i = M->who[k];        // agent
        const auto l = M->where[k]->id;  // loc

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
        A[i]->v_next = M->where[k];
        occupied_next[M->where[k]->id] = A[M->who[k]];
      }
      if (invalid) continue;

      // run PIBT
      for (auto k : S->order) {
        auto a = A[k];
        if (a->v_next == nullptr) {
          if (!funcPIBT(a, nullptr, occupied_now, occupied_next, dist_table)) {
            invalid = true;
            break;
          }
        }
      }
      if (invalid) continue;

      // create new configuration
      auto C = Config(N, nullptr);
      for (auto a : A) C[a->id] = a->v_next;

      // check explored list
      auto S_new_id = get_id(C);
      if (EXPLORED.find(S_new_id) != EXPLORED.end()) continue;

      // insert new search node
      auto S_new = new Node(C, dist_table, S_new_id, S);
      OPEN.push(S_new);
      EXPLORED[S_new->id] = S_new;
    }
  }

  // memory management
  for (auto a : A) delete a;
  for (auto M : GC) delete M;
  for (auto p : EXPLORED) delete p.second;

  return solution;
}

bool funcPIBT(Agent* ai, Agent* aj, Agents& occupied_now, Agents& occupied_next,
              const DistTable& dist_table)
{
  // compare two nodes
  auto cmp = [&](Vertex* const v, Vertex* const u) {
    int d_v = dist_table.get(ai->id, v);
    int d_u = dist_table.get(ai->id, u);
    if (d_v != d_u) return d_v < d_u;
    // tie break
    if (occupied_now[v->id] != nullptr && occupied_now[u->id] == nullptr)
      return false;
    if (occupied_now[v->id] == nullptr && occupied_now[u->id] != nullptr)
      return true;
    return false;
  };

  // get candidates
  auto C = ai->v_now->neighbor;
  C.push_back(ai->v_now);
  // sort
  std::sort(C.begin(), C.end(), cmp);

  for (auto u : C) {
    // avoid vertex conflicts
    if (occupied_next[u->id] != nullptr) continue;
    // avoid swap conflicts
    if (aj != nullptr && u == aj->v_now) continue;

    // reserve
    occupied_next[u->id] = ai;
    ai->v_next = u;

    auto ak = occupied_now[u->id];

    // avoid conflicts with constraints
    if (ak != nullptr && ak->v_next == ai->v_now) {
      continue;
    }

    // priority inheritance
    if (ak != nullptr && ak->v_next == nullptr) {
      if (!funcPIBT(ak, ai, occupied_now, occupied_next, dist_table))
        continue;  // replanning
    }
    // success to plan next one step
    return true;
  }

  // failed to secure node
  occupied_next[ai->v_now->id] = ai;
  ai->v_next = ai->v_now;
  return false;
}

bool is_valid(const Instance& ins, const Solution& solution)
{
  if (solution.empty()) return false;

  // check start
  if (!is_same_config(solution.front(), ins.starts)) return false;

  // check goal
  if (!is_same_config(solution.back(), ins.goals)) return false;

  for (auto t = 1; t < size(solution); ++t) {
    for (auto i = 0; i < ins.N; ++i) {
      auto v_i_from = solution[t - 1][i];
      auto v_i_to = solution[t][i];
      // check connectivity
      if (v_i_from != v_i_to &&
          std::find(v_i_to->neighbor.begin(), v_i_to->neighbor.end(),
                    v_i_from) == v_i_to->neighbor.end())
        return false;
      // check conflicts
      for (auto j = i + 1; j < ins.N; ++j) {
        auto v_j_from = solution[t - 1][j];
        auto v_j_to = solution[t][j];
        // vertex conflicts
        if (v_j_to == v_i_to) return false;
        // swap conflicts
        if (v_j_to == v_i_from && v_j_from == v_i_to) return false;
      }
    }
  }

  return true;
}
