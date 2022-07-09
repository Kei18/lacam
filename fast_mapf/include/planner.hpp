#pragma once

#include <random>

#include "dist_table.hpp"
#include "graph.hpp"
#include "instance.hpp"
#include "utils.hpp"

struct Constraint {
  std::vector<int> who;
  Vertices where;
  const int depth;
  Constraint() : who(std::vector<int>()), where(Vertices()), depth(0) {}
  Constraint(Constraint* parent, int i, Vertex* v)
      : who(parent->who), where(parent->where), depth(parent->depth + 1)
  {
    who.push_back(i);
    where.push_back(v);
  }
  ~Constraint() {}
};

struct Node {
  const Config C;
  const float cost;
  const std::string id;
  Node* parent;
  const int depth;

  // for low-level search
  const std::vector<float> priorities;
  const std::vector<int> order;
  std::queue<Constraint*> search_tree;

  Node(Config _C, const DistTable& dist_table, std::string _id, Node* _parent);
  ~Node();
};
using Nodes = std::vector<Node*>;
using Candidates = std::vector<std::array<Vertex*, 5> >;

struct Agent {
  int id;
  Vertex* v_now;   // current location
  Vertex* v_next;  // next location
  Agent(int _id) : id(_id), v_now(nullptr), v_next(nullptr) {}
};
using Agents = std::vector<Agent*>;

struct Planner {
  const Instance* ins;
  const Deadline* deadline;
  std::mt19937* MT;
  const int verbose;

  // solver utils
  const int N;  // number of agents
  const int V_size;
  const DistTable D;
  Candidates C_next;
  std::vector<float> tie_breakers;
  Agents A;
  Agents occupied_now;
  Agents occupied_next;

  Planner(const Instance* _ins, const Deadline* _deadline, std::mt19937* _MT,
          int _verbose = 0);
  Solution solve();
  bool set_new_config(Node* S, Constraint* M);
  bool funcPIBT(Agent* ai, Agent* aj);
};

float get_cost(Config& C, const DistTable& dist_table);
std::vector<int> get_order(Config& C, const std::vector<float>& priorities);
std::vector<float> get_priorities(Config& C, const DistTable& dist_table,
                                  Node* _parent);
std::string get_id(Config& C);
Solution solve(const Instance& ins, const int verbose = 0,
               const Deadline* deadline = nullptr, std::mt19937* MT = nullptr);
