/*
 * utility functions
 */
#pragma once

#include <algorithm>
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
#include <unordered_set>
#include <vector>
#include <fmt/core.h>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

using Time = std::chrono::steady_clock;

// Lacam info struct, keep it here for now
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

// Time manager
struct Deadline {
  Time::time_point t_s;
  const double time_limit_ms;

  Deadline(double _time_limit_ms = 0);
  bool reset();
  double elapsed_ms() const;
  double elapsed_ns() const;
};

double elapsed_ms(const Deadline* deadline);
double elapsed_ns(const Deadline* deadline);
bool is_expired(const Deadline* deadline);

float get_random_float(std::mt19937* MT, float from = 0, float to = 1);
int get_random_int(std::mt19937* MT, int from, int to);

struct Vertex {
  const int id;         // index for V in Graph
  const int index;      // index for U (width * y + x) in Graph
  const int width;      // indth of graph
  bool cargo = false;   // indicate cargo vertex
  std::vector<Vertex*> neighbor;

  Vertex(int _id, int _index, int _width);

  bool operator==(const Vertex& other) const {
    return id == other.id && index == other.index;
  }

  // Reload << operator
  friend std::ostream& operator<<(std::ostream& os, const Vertex& v) {
    int x = v.index % v.width;
    int y = v.index / v.width;
    os << "(" << x << ", " << y << ")";
    return os;
  }
};

// Locations for all agents
using Vertices = std::vector<Vertex*>;
using Config = std::vector<Vertex*>;
// Solution: a sequence of configurations
using Solution = std::vector<Config>;

// Overload the << for Config
inline std::ostream& operator<<(std::ostream& os, const Config& config) {
  os << "[";
  for (size_t i = 0; i < config.size(); ++i) {
    if (config[i]) {  // Check if the pointer is not null
      os << *config[i];  // Use the overloaded << for Vertex
    }
    if (i < config.size() - 1) {
      os << ", ";  // Add a comma between elements
    }
  }
  os << "]";
  return os;
}

// Overload the spdlog for Vertex
template <>
struct fmt::formatter<Vertex> {
  constexpr auto parse(fmt::format_parse_context& ctx) {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const Vertex& v, FormatContext& ctx) {
    std::ostringstream oss;
    oss << v;  // Utilizing the existing operator<< overload
    return fmt::format_to(ctx.out(), "{}", oss.str());
  }
};

// Overload the spdlog for Vertices
template <>
struct fmt::formatter<std::vector<Vertex*>> {
  template <typename ParseContext>
  constexpr auto parse(ParseContext& ctx) {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const std::vector<Vertex*>& vec, FormatContext& ctx) {
    fmt::memory_buffer out;
    for (const auto& elem : vec) {
      if (elem != nullptr) {
        std::ostringstream oss;
        oss << *elem;  // Use the overloaded << operator for Vertex
        fmt::format_to(std::back_inserter(out), "{} ", oss.str());
      }
      else {
        fmt::format_to(std::back_inserter(out), "null ");
      }
    }
    return fmt::format_to(ctx.out(), "{}", to_string(out));
  }
};


// Cache Type
enum class CacheType {
  NONE,
  LRU,
  FIFO,
  RANDOM
};

inline bool is_cache(CacheType cache_type) {
  return cache_type != CacheType::NONE;
}
