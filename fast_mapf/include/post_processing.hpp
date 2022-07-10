#pragma once
#include "instance.hpp"
#include "utils.hpp"

bool is_feasible_solution(const Instance& ins, const Solution& solution,
                          const int verbose = 0);
int get_makespan(const Solution& solution);
int get_path_cost(const Solution& solution, int i);
int get_sum_of_costs(const Solution& solution);
void print_stats(const int verbose, const Instance& ins,
                 const Solution& solution, const double comp_time_ms);
void make_log(const Instance& ins, const Solution& solution,
              const std::string& output_name, const double comp_time_ms,
              const std::string& map_name, const bool log_short = false);
