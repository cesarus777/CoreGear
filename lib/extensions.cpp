#include "coregear/extensions.hpp"

const std::unordered_map<cg::extensions_t, std::string>
    cg::enabled_extensions_t::ExtToStr = {
#define ENUM_ITEM(enumerator, str, num) {extensions_t::enumerator, #str},
        CG_ALL_EXTENSIONS(ENUM_ITEM)
#undef ENUM_ITEM
};

const std::unordered_map<std::string, cg::extensions_t>
    cg::enabled_extensions_t::StrToExt = {
#define ENUM_ITEM(enumerator, str, num) {#str, extensions_t::enumerator},
        CG_ALL_EXTENSIONS(ENUM_ITEM)
#undef ENUM_ITEM
};
