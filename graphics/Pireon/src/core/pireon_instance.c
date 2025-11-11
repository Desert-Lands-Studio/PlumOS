#include <stdlib.h>
#include <string.h>
#include "pireon_core.h"

/* ���������� �������� */
typedef struct PrInstancePayload {
    char     app_name[64];
    uint32_t app_version;
} PrInstancePayload;

PrResult PRAPI_CALL
prCreateInstance(const char* app_name,
    uint32_t    app_version,
    PrInstance* out_instance)
{
    if (!out_instance) return PR_ERROR_INVALID_ARG;

    PrInstancePayload* p = (PrInstancePayload*)malloc(sizeof(*p));
    if (!p) return PR_ERROR_OUT_OF_MEMORY;

    strncpy(p->app_name, app_name ? app_name : "unknown", sizeof(p->app_name) - 1);
    p->app_name[sizeof(p->app_name) - 1] = 0;
    p->app_version = app_version;

    /* ����� ��������� � ������ */
    out_instance->_ = (uint64_t)p;
    return PR_SUCCESS;
}

void PRAPI_CALL
prDestroyInstance(PrInstance inst)
{
    if (!inst._) return;
    free((void*)inst._);
}

PrResult PRAPI_CALL
prEnumerateAdapters(PrInstance      inst,
    uint32_t* adapter_count,
    PrAdapter* adapters)
{
    /* TODO: ����� � ��������� �������� GPU-��������� */
    const uint32_t kDummyCount = 1;

    if (adapter_count) *adapter_count = kDummyCount;
    if (adapters && kDummyCount > 0)
        adapters[0]._ = 0xCAFEF00D;   /* ��������� ID */

    (void)inst; /* ���� �� ������������ */
    return PR_SUCCESS;
}
