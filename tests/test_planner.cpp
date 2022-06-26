#include <iostream>
#include <planner.hpp>

#include "gtest/gtest.h"

TEST(planner, load_dist_table)
{
  const auto scen_filename = "./assets/scen/random-32-32-10-random-1.scen";
  const auto map_filename = "./assets/map/random-32-32-10.map";
  const auto ins = Instance(scen_filename, map_filename, 3);

  const auto K = size(ins.G.V);
  auto dist_table = DistTable(ins.N, std::vector<int>(K, K));
  load_dist_table(dist_table, ins);

  ASSERT_EQ(dist_table[0][ins.goals[0]->id], 0);
  ASSERT_EQ(dist_table[0][ins.starts[0]->id], 16);
}

TEST(planner, solve)
{
  const auto scen_filename = "./assets/scen/random-32-32-10-random-1.scen";
  const auto map_filename = "./assets/map/random-32-32-10.map";
  const auto ins = Instance(scen_filename, map_filename, 3);

  solve(ins);
}
