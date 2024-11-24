#pragma once

#ifdef __cplusplus
extern "C" {
#define PRS_NOEXCEPT noexcept
#else
#define PRS_NOEXCEPT
#endif

int run_prs(int argc, char *argv[]) PRS_NOEXCEPT;

#ifdef __cplusplus
}
#endif
