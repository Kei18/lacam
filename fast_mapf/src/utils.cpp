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

bool is_expired(const Deadline* deadline)
{
  if (deadline == nullptr) return false;
  return deadline->elapsed_ms() > deadline->time_limit_ms;
}
