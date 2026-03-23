#pragma once

#define CG_UNREACHABLE(...)                                                    \
  cg::unreachable(__FILE__, __LINE__ __VA_OPT__(, __VA_ARGS__))
