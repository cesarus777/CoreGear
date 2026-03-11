#pragma once

#define CG_ASSERT(cond, ...)                                                   \
  cg::assert((cond), #cond, __FILE__, __LINE__ __VA_OPT__(, __VA_ARGS__))
