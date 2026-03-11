#pragma once

#include <format>
#include <stdexcept>

namespace cg {

class base_exception : public std::runtime_error {
public:
  template <typename... args_t>
  base_exception(args_t &&...args)
      : runtime_error(std::forward<args_t>(args)...) {}
};

class input_error : public base_exception {
public:
  template <typename... args_t>
  input_error(args_t &&...args)
      : base_exception(std::forward<args_t>(args)...) {}
};

class unknown_argument : public input_error {
public:
  unknown_argument(std::string_view arg)
      : input_error(std::format("Unknown argument: {}", arg)) {}
};

class invalid_argument : public input_error {
public:
  invalid_argument(std::string_view arg, std::string_view val,
                   std::string_view explanation)
      : input_error([=] {
          auto mandatory_msg =
              std::format("Invalid value for '{}': {}.", arg, val);
          if (explanation.empty())
            return mandatory_msg;

          return std::format("{} {}", mandatory_msg, explanation);
        }()) {}
};

class io_error : public base_exception {
public:
  template <typename... args_t>
  io_error(args_t &&...args) : base_exception(std::forward<args_t>(args)...) {}
};

class cant_open_file_error : public io_error {
public:
  template <typename filename_t>
  cant_open_file_error(filename_t &&filename)
      : io_error(std::format("Can't open file: {}.",
                             std::forward<filename_t>(filename))) {}
};

class unknown_extension : public base_exception {
public:
  unknown_extension(std::string_view ext_str)
      : base_exception(std::format("Unknown RISC-V extension: {}.", ext_str)) {}
};

} // namespace cg
