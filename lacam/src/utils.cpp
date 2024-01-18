#include "../include/utils.hpp"

void info(const int level, const int verbose) { std::cout << std::endl; }

Deadline::Deadline(double _time_limit_ms)
  : t_s(Time::now()), time_limit_ms(_time_limit_ms)
{
}

double Deadline::elapsed_ms() const
{
  return std::chrono::duration_cast<std::chrono::milliseconds>(Time::now() -
    t_s)
    .count();
}

double Deadline::elapsed_ns() const
{
  return std::chrono::duration_cast<std::chrono::nanoseconds>(Time::now() - t_s)
    .count();
}

double elapsed_ms(const Deadline* deadline)
{
  if (deadline == nullptr) return 0;
  return deadline->elapsed_ms();
}

double elapsed_ns(const Deadline* deadline)
{
  if (deadline == nullptr) return 0;
  return deadline->elapsed_ns();
}

bool is_expired(const Deadline* deadline)
{
  if (deadline == nullptr) return false;
  return deadline->elapsed_ms() > deadline->time_limit_ms;
}

float get_random_float(std::mt19937* MT, float from, float to)
{
  std::uniform_real_distribution<float> r(from, to);
  return r(*MT);
}

int get_random_int(std::mt19937* MT, int from, int to)
{
  std::uniform_int_distribution<int> dist(from, to);
  return dist(*MT);
}
