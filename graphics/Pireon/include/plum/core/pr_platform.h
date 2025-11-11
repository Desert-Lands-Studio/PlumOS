#ifndef PIREON_PLATFORM_H
#define PIREON_PLATFORM_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
#define PR_EXTERN_C extern "C"
#else
#define PR_EXTERN_C
#endif

#if defined(_WIN32)
#define PRAPI_ATTR
#define PRAPI_CALL __stdcall
#else
#define PRAPI_ATTR __attribute__((visibility("default")))
#define PRAPI_CALL
#endif

typedef uint32_t PrBool32;

#endif 