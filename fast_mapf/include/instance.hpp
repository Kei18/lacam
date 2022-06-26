#pragma once
#include "graph.hpp"

struct Instance {
  Graph G;
  Vertices starts;
  Vertices goals;

  Instance();
};
