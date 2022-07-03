#pragma once
#include "instance.hpp"
#include "utils.hpp"

void print_stats(const int verbose, const Instance& ins,
                 const Solution& solution, const double comp_time_ms);
void make_log(const Instance& ins, const Solution& solution,
              const std::string& output_name, const double comp_time_ms,
              const std::string& map_name);
