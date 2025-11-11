#ifndef PIREON_TYPES_H
#define PIREON_TYPES_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

    
#define PR_DEFINE_HANDLE(name) typedef struct name##_T { uint64_t _; } name;

    PR_DEFINE_HANDLE(PrInstance)
        PR_DEFINE_HANDLE(PrAdapter)
        PR_DEFINE_HANDLE(PrDevice)
        PR_DEFINE_HANDLE(PrQueue)
        PR_DEFINE_HANDLE(PrFence)
        PR_DEFINE_HANDLE(PrSemaphore)
        PR_DEFINE_HANDLE(PrBuffer)
        PR_DEFINE_HANDLE(PrImage)
        PR_DEFINE_HANDLE(PrView)
        PR_DEFINE_HANDLE(PrPipeline)
        PR_DEFINE_HANDLE(PrCommandPool)
        PR_DEFINE_HANDLE(PrCommandBuffer)

        
        typedef enum PrResult {
        PR_SUCCESS = 0,
        PR_ERROR_OUT_OF_MEMORY = -1,
        PR_ERROR_DEVICE_LOST = -2,
        PR_ERROR_INVALID_ARG = -3,
        
    } PrResult;

#ifdef __cplusplus
}
#endif
#endif 