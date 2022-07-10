#include "../include/planner.hpp"

#include <algorithm>
#include <random>

std::string get_id(Config& C)
{
  std::string id = "";
  for (auto v : C) id += std::to_string(v->id) + "-";
  return id;
}

std::vector<float> get_priorities(Config& C, const DistTable& dist_table,
                                  Node* parent)
{
  const auto N = C.size();
  auto P = std::vector<float>(C.size(), 0);
  if (parent == nullptr) {
    // initialize
    for (auto i = 0; i < N; ++i) P[i] = dist_table.get(i, C[i]) / N;
  } else {
    // dynamic priorities from PIBT
    for (auto i = 0; i < N; ++i) {
      if (dist_table.get(i, C[i]) != 0) {
        P[i] = parent->priorities[i] + 1;
      } else {
        P[i] = parent->priorities[i] - (int)parent->priorities[i];
      }
    }
  }
  return P;
}

std::vector<int> get_order(Config& C, const std::vector<float>& priorities)
{
  std::vector<int> A(C.size());
  std::iota(A.begin(), A.end(), 0);
  std::sort(A.begin(), A.end(),
            [&](int i, int j) { return priorities[i] > priorities[j]; });
  return A;
}

Node::Node(Config _C, const DistTable& dist_table, std::string _id = "",
           Node* _parent = nullptr)
    : C(_C),
      id(_id == "" ? get_id(_C) : _id),
      parent(_parent),
      depth(_parent == nullptr ? 0 : _parent->depth + 1),
      priorities(get_priorities(_C, dist_table, _parent)),
      order(get_order(_C, priorities)),
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

Planner::Planner(const Instance* _ins, const Deadline* _deadline,
                 std::mt19937* _MT, int _verbose)
    : ins(_ins),
      deadline(_deadline),
      MT(_MT),
      verbose(_verbose),
      N(ins->N),
      V_size(ins->G.size()),
      D(DistTable(ins)),
      C_next(Candidates(N, std::array<Vertex*, 5>())),
      tie_breakers(std::vector<float>(V_size, 0)),
      A(Agents(N, nullptr)),
      occupied_now(Agents(V_size, nullptr)),
      occupied_next(Agents(V_size, nullptr))
{
}

Solution Planner::solve()
{
  info(1, verbose, "elapsed:", elapsed_ms(deadline), "\tstart search");

  // setup agents
  for (auto i = 0; i < N; ++i) A[i] = new Agent(i);

  // setup search queues
  std::stack<Node*> OPEN;
  std::unordered_map<std::string, Node*> EXPLORED;
  std::vector<Constraint*> GC;  // garbage collection for constraint

  // insert initial node
  auto S = new Node(ins->starts, D);
  OPEN.push(S);
  EXPLORED[S->id] = S;

  // best first search
  int loop_cnt = 0;
  std::vector<Config> solution;

  while (!OPEN.empty() && !is_expired(deadline)) {
    loop_cnt += 1;

    // do not pop here!
    S = OPEN.top();

    // check goal condition
    if (is_same_config(S->C, ins->goals)) {
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
      auto C = S->C[i]->neighbor;
      C.push_back(S->C[i]);
      std::shuffle(C.begin(), C.end(), *MT);  // randomize
      for (auto u : C) S->search_tree.push(new Constraint(M, i, u));
    }

    // create successor for high-level search by PIBT
    if (!set_new_config(S, M)) continue;

    // create new configuration
    auto C = Config(N, nullptr);
    for (auto a : A) C[a->id] = a->v_next;

    // check explored list
    auto S_new_id = get_id(C);
    auto iter = EXPLORED.find(S_new_id);
    if (iter != EXPLORED.end()) {
      if (iter->second != S->parent) OPEN.push(iter->second);
      continue;
    }

    // insert new search node
    auto S_new = new Node(C, D, S_new_id, S);
    OPEN.push(S_new);
    EXPLORED[S_new->id] = S_new;
  }

  info(1, verbose, "elapsed:", elapsed_ms(deadline), "\texpanded:", loop_cnt,
       "\texplored:", EXPLORED.size());
  // memory management
  for (auto a : A) delete a;
  for (auto M : GC) delete M;
  for (auto p : EXPLORED) delete p.second;

  return solution;
}

bool Planner::set_new_config(Node* S, Constraint* M)
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
    occupied_now[a->v_now->id] = a;
  }

  // add constraints
  for (auto k = 0; k < M->depth; ++k) {
    const auto i = M->who[k];        // agent
    const auto l = M->where[k]->id;  // loc

    // check vertex collision
    if (occupied_next[l] != nullptr) return false;
    // check swap collision
    auto l_pre = S->C[i]->id;
    if (occupied_next[l_pre] != nullptr && occupied_now[l] != nullptr &&
        occupied_next[l_pre]->id == occupied_now[l]->id)
      return false;

    // set occupied_next
    A[i]->v_next = M->where[k];
    occupied_next[l] = A[i];
  }

  // add usual PIBT
  for (auto k : S->order) {
    auto a = A[k];
    if (a->v_next == nullptr && !funcPIBT(a, nullptr)) return false;
  }
  return true;
}

bool Planner::funcPIBT(Agent* ai, Agent* aj)
{
  const auto i = ai->id;
  const auto K = ai->v_now->neighbor.size();

  // get candidates
  for (auto k = 0; k < K; ++k) {
    auto u = ai->v_now->neighbor[k];
    C_next[i][k] = u;
    if (MT != nullptr) tie_breakers[u->id] = get_random_float(MT);
  }
  C_next[i][K] = ai->v_now;
  if (MT != nullptr) tie_breakers[ai->v_now->id] = get_random_float(MT);
  for (auto k = K + 1; k < 5; ++k) C_next[i][k] = nullptr;

  // sort
  std::sort(C_next[i].begin(), C_next[i].end(),
            [&](Vertex* const v, Vertex* const u) {
              if (v != nullptr && u == nullptr) return true;
              if (v == nullptr && u != nullptr) return false;
              if (v != nullptr && u != nullptr) {
                auto d_v = D.get(ai->id, v);
                auto d_u = D.get(ai->id, u);
                if (d_v != d_u) return d_v < d_u;
                return tie_breakers[v->id] < tie_breakers[u->id];
              }
              return false;
            });

  for (auto u : C_next[i]) {
    if (u == nullptr) break;

    // avoid vertex conflicts
    if (occupied_next[u->id] != nullptr) continue;
    // avoid swap conflicts
    if (aj != nullptr && u == aj->v_now) continue;

    auto ak = occupied_now[u->id];

    // avoid swap confilicts with constraints
    if (ak != nullptr && ak->v_next == ai->v_now) continue;

    // reserve
    occupied_next[u->id] = ai;
    ai->v_next = u;

    // empty or stay
    if (ak == nullptr || u == ai->v_now) return true;

    // priority inheritance
    if (ak->v_next == nullptr && !funcPIBT(ak, ai)) continue;

    // success to plan next one step
    return true;
  }

  // failed to secure node
  occupied_next[ai->v_now->id] = ai;
  ai->v_next = ai->v_now;
  return false;
}

Solution solve(const Instance& ins, const int verbose, const Deadline* deadline,
               std::mt19937* MT)
{
  auto planner = Planner(&ins, deadline, MT, verbose);
  return planner.solve();
}
