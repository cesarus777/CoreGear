#pragma once

#include <prs/assert.hpp>

#include <ostream>

namespace prs {

constexpr size_t hex_byte_width = 2;

template <std::integral T>
constexpr size_t hex_width = sizeof(T) * hex_byte_width;

class logger_t final {
  std::ostream *logger;

public:
  logger_t(std::ostream &os) : logger(&os) {}

  template <typename T> logger_t &operator<<(T &&arg) {
    PRS_ASSERT(logger);
    *logger << std::forward<T>(arg);
    return *this;
  }

  operator bool() const {
    PRS_ASSERT(logger);
    return static_cast<bool>(*logger);
  }

  auto exceptions() const {
    PRS_ASSERT(logger);
    return logger->exceptions();
  }

  void exceptions(std::ios_base::iostate except) const {
    PRS_ASSERT(logger);
    logger->exceptions(except);
  }

  logger_t &flush() {
    PRS_ASSERT(logger);
    logger->flush();
    return *this;
  }
};

} // namespace prs
