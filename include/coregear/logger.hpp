#pragma once

#include "coregear/assert.hpp"

#include <ostream>

namespace cg {

constexpr size_t hex_byte_width = 2;

template <std::integral T>
constexpr size_t hex_width = sizeof(T) * hex_byte_width;

class logger_t final {
  std::ostream *logger;

public:
  logger_t(std::ostream &os) : logger(&os) {}

  template <typename T> logger_t &operator<<(T &&arg) {
    CG_ASSERT(logger);
    *logger << std::forward<T>(arg);
    return *this;
  }

  operator bool() const {
    CG_ASSERT(logger);
    return static_cast<bool>(*logger);
  }

  auto exceptions() const {
    CG_ASSERT(logger);
    return logger->exceptions();
  }

  void exceptions(std::ios_base::iostate except) const {
    CG_ASSERT(logger);
    logger->exceptions(except);
  }

  logger_t &flush() {
    CG_ASSERT(logger);
    logger->flush();
    return *this;
  }
};

} // namespace cg
