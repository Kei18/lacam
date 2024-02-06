#include <lacam.hpp>

#include "gtest/gtest.h"

TEST(dist_table, init)
{
  const auto map_filename = "./assets/random-32-32-10.map";
  const auto ins = Instance(map_filename, 3);
  auto dist_table = DistTable(ins);

  ASSERT_EQ(dist_table.get(0, ins.goals[0]), 0);
  ASSERT_EQ(dist_table.get(0, ins.starts[0]), 16);
}
