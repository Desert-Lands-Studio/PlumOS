#ifndef PIREON_CORE_H
#define PIREON_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "pr_platform.h"

#define VK_MAKE_API_VERSION(variant, major, minor, patch) \
    ((((uint32_t)(variant)) << 29U) | (((uint32_t)(major)) << 22U) | (((uint32_t)(minor)) << 12U) | ((uint32_t)(patch)))


#endif 