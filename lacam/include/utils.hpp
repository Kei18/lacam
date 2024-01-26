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
int get_random_int(std::mt19937* MT, int from, int to);

struct Vertex {
  const int id;       // index for V in Graph
  const int index;    // index for U (width * y + x) in Graph
  const int width;    // indth of graph
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

struct CargoVertex {
  const int index;    // index for cargo (width * y + x) in Graph
  CargoVertex(int _index);
};

using Vertices = std::vector<Vertex*>;
using CargoVertices = std::vector<CargoVertex*>;
using TargetVertices = std::unordered_set<Vertex*>;
using Config = std::vector<Vertex*>;  // locations for all agents

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
