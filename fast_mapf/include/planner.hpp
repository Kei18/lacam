#pragma once
#include <queue>
#include <string>
#include <unordered_map>

#include "instance.hpp"

using DistTable = std::vector<std::vector<int>>;

struct Node {
  const Config C;
  const float cost;
  const std::string id;
  const Node* parent;
  Node(Config _C, DistTable& dist_table, Node* _parent = nullptr);
};

using Nodes = std::vector<Node*>;

void load_dist_table(DistTable& dist_table, const Instance& ins);
float get_cost(Config& C, DistTable& dist_table);
std::string get_id(Config& C);
bool is_same_config(const Config& C1, const Config& C2);
void solve(const Instance& ins);
