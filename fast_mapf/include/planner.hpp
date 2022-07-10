#pragma once

#include "dist_table.hpp"
#include "graph.hpp"
#include "instance.hpp"
#include "utils.hpp"

struct Constraint {
  std::vector<int> who;
  Vertices where;
  const int depth;
  Constraint();
  Constraint(Constraint* parent, int i, Vertex* v);
  ~Constraint();
};

struct Node {
  const Config C;
  Node* parent;
  const int depth;

  // for low-level search
  std::vector<float> priorities;
  std::vector<int> order;
  std::queue<Constraint*> search_tree;

  Node(Config _C, DistTable& D, Node* _parent = nullptr);
  ~Node();
};

struct Agent {
  const int id;
  Vertex* v_now;   // current location
  Vertex* v_next;  // next location
  Agent(int _id) : id(_id), v_now(nullptr), v_next(nullptr) {}
};

using Nodes = std::vector<Node*>;
using Candidates = std::vector<std::array<Vertex*, 5> >;
using Agents = std::vector<Agent*>;

struct Planner {
  const Instance* ins;
  const Deadline* deadline;
  std::mt19937* MT;
  const int verbose;

  // solver utils
  const int N;  // number of agents
  const int V_size;
  DistTable D;
  Candidates C_next;
  std::vector<float> tie_breakers;
  Agents A;
  Agents occupied_now;
  Agents occupied_next;

  Planner(const Instance* _ins, const Deadline* _deadline, std::mt19937* _MT,
          int _verbose = 0);
  Solution solve();
  bool set_new_config(Node* S, Constraint* M);
  bool funcPIBT(Agent* ai, Agent* aj = nullptr);
};

Solution solve(const Instance& ins, const int verbose = 0,
               const Deadline* deadline = nullptr, std::mt19937* MT = nullptr);
