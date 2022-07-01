#include "../include/utils.hpp"

void info(const int level, const int verbose, const std::string& msg)
{
  if (verbose < level) return;
  std::cout << msg << std::endl;
}
