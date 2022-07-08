#include <fast_mapf.hpp>
#include <iostream>

#include "gtest/gtest.h"

TEST(Instance, initialize)
{
  const auto scen_filename = "./assets/scen/random-32-32-10-random-1.scen";
  const auto map_filename = "./assets/map/random-32-32-10.map";
  const auto ins = Instance(scen_filename, map_filename, 3);

  ASSERT_EQ(size(ins.starts), 3);
  ASSERT_EQ(size(ins.goals), 3);
  ASSERT_EQ(ins.starts[0]->id, 203);
  ASSERT_EQ(ins.goals[0]->id, 583);
}
