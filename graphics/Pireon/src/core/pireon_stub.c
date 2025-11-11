#include "plum/pireon_core.h"
#include "plum/pireon_plumpane.h"

PrResult prCreatePaneSurface(PrInstance, const PrPaneSurfaceCreateInfo*, PrPaneSurface* out)
{
    *out = (PrPaneSurface){ ._ = 77 }; return PR_SUCCESS;
}

PrResult prAcquireNextSurfaceImage(PrDevice dev,
    PrPaneSurface surf,
    uint64_t timeout,
    uint32_t* idx)
{
    (void)dev; (void)surf; (void)timeout;
    if (idx) *idx = 0;
    return PR_SUCCESS;
}

PrResult prPresentSurfaceImage(PrQueue q,
    PrPaneSurface surf,
    uint32_t idx)
{
    (void)q; (void)surf; (void)idx;
    return PR_SUCCESS;
}