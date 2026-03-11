#pragma once

#include "coregear/exceptions.hpp"

#include <algorithm>
#include <set>
#include <string_view>
#include <unordered_map>

namespace cg {

#define CG_ALL_EXTENSIONS(X)                                                  \
  X(I, i, 0)                                                                   \
  X(C, c, 1)                                                                   \
  X(M, m, 2)

enum class extensions_t : size_t {
#define ENUM_ITEM(enumerator, str, num) enumerator = num,
  CG_ALL_EXTENSIONS(ENUM_ITEM)
#undef ENUM_ITEM
};

#define ADD_ONE(enumerator, str, num) +1

constexpr size_t num_supported_extensions = CG_ALL_EXTENSIONS(ADD_ONE);

class enabled_extensions_t final {
  using extensions_storage_t = std::set<extensions_t>;
  extensions_storage_t en_exts;

public:
  const static std::unordered_map<extensions_t, std::string> ExtToStr;
  const static std::unordered_map<std::string, extensions_t> StrToExt;

  enabled_extensions_t() = default;

  enabled_extensions_t(std::string_view extentions_str) {
    // TODO: support multichar extensions
    std::ranges::transform(extentions_str,
                           std::inserter(en_exts, en_exts.end()), [](char c) {
                             try {
                               return StrToExt.at(std::string(1, c));
                             } catch (const std::out_of_range &e) {
                               throw unknown_extension(std::string(1, c));
                             }
                           });
  }

  auto begin() const { return en_exts.begin(); }
  auto end() const { return en_exts.end(); }

  auto size() const noexcept { return en_exts.size(); }
};

} // namespace cg
