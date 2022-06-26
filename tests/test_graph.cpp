#include <graph.hpp>
#include <iostream>

#include "gtest/gtest.h"

TEST(Graph, load_graph)
{
  auto G = Graph();
  std::string filename = "./assets/map/random-32-32-10.map";
  load_graph(G, filename);
  ASSERT_TRUE(get_num_vertices(G) == 922);
  ASSERT_TRUE(size(G.V[0]->neighbor) == 2);
  ASSERT_TRUE(G.V[0]->neighbor[0]->id == 1);
  ASSERT_TRUE(G.V[0]->neighbor[1]->id == 32);
}
