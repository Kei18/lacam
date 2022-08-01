/*
 * utility functions
 */
#pragma once

#include <array>
#include <chrono>
#include <climits>
#include <fstream>
#include <iostream>
#include <numeric>
#include <queue>
#include <random>
#include <regex>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>

using Time = std::chrono::steady_clock;

template <typename Head, typename... Tail>
void info(const int level, const int verbose, Head&& head, Tail&&... tail);

void info(const int level, const int verbose);

template <typename Head, typename... Tail>
void info(const int level, const int verbose, Head&& head, Tail&&... tail)
{
  if (verbose < level) return;
  std::cout << head;
  info(level, verbose, std::forward<Tail>(tail)...);
}

// time manager
struct Deadline {
  const Time::time_point t_s;
  const double time_limit_ms;

  Deadline(double _time_limit_ms = 0);
  double elapsed_ms() const;
  double elapsed_ns() const;
};

double elapsed_ms(const Deadline* deadline);
double elapsed_ns(const Deadline* deadline);
bool is_expired(const Deadline* deadline);

float get_random_float(std::mt19937* MT, float from = 0, float to = 1);
