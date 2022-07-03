#include <fast_mapf.hpp>
#include <iostream>

#include "gtest/gtest.h"

TEST(planner, solve)
{
  const auto scen_filename = "./assets/scen/random-32-32-10-random-1.scen";
  const auto map_filename = "./assets/map/random-32-32-10.map";
  const auto ins = Instance(scen_filename, map_filename, 3);

  auto solution = solve(ins);
  ASSERT_TRUE(is_feasible_solution(ins, solution));
}
