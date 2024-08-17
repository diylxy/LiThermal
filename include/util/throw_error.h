#pragma once

#include <sstream>
#include <string>

template <typename E>
struct throw_error {
  throw_error() = default;

  explicit throw_error(const std::string &s) {
    ss << s;
  }

  ~throw_error() noexcept(false) {
    throw E(ss.str());
  }

  template <class T>
  throw_error &operator<<(const T &val) {
    ss << val;
    return *this;
  }

  std::ostringstream ss;
};