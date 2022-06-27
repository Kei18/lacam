#pragma once

#include <queue>

#include "graph.hpp"
#include "instance.hpp"
#include "utils.hpp"

using DistTable = std::vector<std::vector<int>>;

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
  ~Constraint() { std::cout << size(who) << std::endl; }
};

struct Node {
  const Config C;
  const float cost;
  const std::string id;
  const Node* parent;

  // for low-level search
  const std::vector<int> order;
  std::queue<Constraint*> search_tree;

  Node(Config _C, DistTable& dist_table, Node* _parent = nullptr);
  ~Node();
};
using Nodes = std::vector<Node*>;

void load_dist_table(DistTable& dist_table, const Instance& ins);
float get_cost(Config& C, DistTable& dist_table);
std::string get_id(Config& C);
void solve(const Instance& ins);
