#pragma once
#include "graph.hpp"
#include "utils.hpp"

struct Instance {
  Graph G;
  Config starts;
  Config goals;
  const int N;

  Instance(const std::string& scen_filename, const std::string& map_filename,
           const int _N = 1);
};

using Solution = std::vector<Config>;
bool is_valid_instance(const Instance& ins, const int verbose = 0);
bool is_feasible_solution(const Instance& ins, const Solution& solution,
                          const int verbose = 0);
int get_makespan(const Solution& solution);
int get_path_cost(const Solution& solution, int i);
int get_sum_of_costs(const Solution& solution);
