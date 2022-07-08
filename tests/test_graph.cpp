#include <fast_mapf.hpp>

#include "gtest/gtest.h"

TEST(Graph, load_graph)
{
  const std::string filename = "./assets/map/random-32-32-10.map";
  auto G = Graph(filename);
  ASSERT_EQ(G.get_num_vertices(), 922);
  ASSERT_EQ(G.V[0]->neighbor.size(), 2);
  ASSERT_EQ(G.V[0]->neighbor[0]->id, 1);
  ASSERT_EQ(G.V[0]->neighbor[1]->id, 32);
  ASSERT_EQ(G.width, 32);
  ASSERT_EQ(G.height, 32);
}
