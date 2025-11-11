#include <stdlib.h>
#include "pireon_core.h"

/* ��������� �������� ���������� */
typedef struct PrDevicePayload {
    PrAdapter adapter;
} PrDevicePayload;

PrResult PRAPI_CALL
prCreateDevice(PrAdapter                 adapter,
    const struct PrDeviceDesc*/*unused*/,
    PrDevice* out_device)
{
    if (!out_device) return PR_ERROR_INVALID_ARG;

    PrDevicePayload* dev = (PrDevicePayload*)malloc(sizeof(*dev));
    if (!dev) return PR_ERROR_OUT_OF_MEMORY;

    dev->adapter = adapter;
    out_device->_ = (uint64_t)dev;
    return PR_SUCCESS;
}

void PRAPI_CALL
prDestroyDevice(PrDevice dev)
{
    if (!dev._) return;
    free((void*)dev._);
}

PrQueue PRAPI_CALL
prGetDeviceQueue(PrDevice  dev,
    uint32_t /*queue_family*/,
    uint32_t /*index*/)
{
    /* TODO: ����� ������� ������ �������; ������ ���� ��������� */
    (void)dev;
    return (PrQueue) { ._ = 0x1234 };
}
