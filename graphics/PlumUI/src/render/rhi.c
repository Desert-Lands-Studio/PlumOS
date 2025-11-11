#include "rhi.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static PlumRhi* default_rhi = NULL;

PlumRhi* plum_get_rhi_context(void) {
    return default_rhi;
}