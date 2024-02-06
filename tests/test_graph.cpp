#include <lacam.hpp>
#include "gtest/gtest.h"

TEST(Graph, load_graph)
{
  const std::string filename = "../assets/test-8-8.map";
  auto test = spdlog::stderr_color_mt("test");
  auto MT = std::mt19937(0);
  test->set_level(spdlog::level::debug);

  auto G = Graph(filename, test, 10, 5, 10, true, &MT);

  // Test graph paras
  ASSERT_EQ(G.size(), 36);
  ASSERT_EQ(G.U.size(), 64);
  ASSERT_EQ(G.width, 8);
  ASSERT_EQ(G.height, 8);
  ASSERT_EQ(G.unloading_ports.size(), 1);
  ASSERT_EQ(G.cargo_vertices.size(), 3);
  ASSERT_EQ(G.cache->node_id.size(), 3);
  ASSERT_EQ(G.goals_list.size(), 10);

  // Test normal block
  ASSERT_EQ(G.V[0]->neighbor.size(), 2);
  ASSERT_EQ(G.V[0]->neighbor[0]->id, 1);
  ASSERT_EQ(G.V[0]->neighbor[1]->id, 6);

  // Test unloading port block
  ASSERT_EQ(G.unloading_ports[0]->neighbor.size(), 3);
  ASSERT_EQ(G.unloading_ports[0]->neighbor[0]->id, 13);
  ASSERT_EQ(G.unloading_ports[0]->neighbor[1]->id, 18);
  ASSERT_EQ(G.unloading_ports[0]->neighbor[2]->id, 6);

  // Test cargo block
  ASSERT_EQ(G.cargo_vertices[0]->neighbor.size(), 2);
  ASSERT_EQ(G.cargo_vertices[0]->neighbor[0]->id, 11);
  ASSERT_EQ(G.cargo_vertices[0]->neighbor[1]->id, 4);

  // Test cache block
  ASSERT_EQ(G.cache->node_id[0]->neighbor.size(), 2);
  ASSERT_EQ(G.cache->node_id[0]->neighbor[0]->id, 8);
  ASSERT_EQ(G.cache->node_id[0]->neighbor[1]->id, 3);
}
