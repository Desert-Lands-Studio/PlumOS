#ifndef PIREON_PLUMPANE_H
#define PIREON_PLUMPANE_H

#include "pireon_platform.h"
#include "pireon_types.h"

�� ������������� ����� � PlumPane compositor */
PR_DEFINE_HANDLE(PrPaneSurface);

typedef struct PrPaneSurfaceCreateInfo {
    uint64_t pane_window_id; � �. */
} PrPaneSurfaceCreateInfo;

PR_EXTERN_C PRAPI_ATTR
PrResult PRAPI_CALL prCreatePaneSurface(PrInstance                       instance,
    const PrPaneSurfaceCreateInfo* info,
    PrPaneSurface* surface);

#endif 