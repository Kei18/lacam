#pragma once
#include <queue>

#include "instance.hpp"

using DistTable = std::vector<std::vector<int>>;

void load_dist_table(DistTable& dist_table, const Instance& ins);
void solve(const Instance& ins);
