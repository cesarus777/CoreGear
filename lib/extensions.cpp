#include "prs/extensions.hpp"

const std::unordered_map<prs::extensions_t, std::string>
    prs::enabled_extensions_t::ExtToStr = {
#define ENUM_ITEM(enumerator, str, num) {extensions_t::enumerator, #str},
        PRS_ALL_EXTENSIONS(ENUM_ITEM)
#undef ENUM_ITEM
};

const std::unordered_map<std::string, prs::extensions_t>
    prs::enabled_extensions_t::StrToExt = {
#define ENUM_ITEM(enumerator, str, num) {#str, extensions_t::enumerator},
        PRS_ALL_EXTENSIONS(ENUM_ITEM)
#undef ENUM_ITEM
};
