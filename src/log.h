#pragma once
#include <string>

class LOG {
 public:
  static void Error(const std::string& message);
  static void Error(int error_code);
};
