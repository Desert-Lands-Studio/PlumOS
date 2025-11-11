#ifndef PIREON_ICD_H
#define PIREON_ICD_H

#include "pireon_types.h"

typedef struct PrIcdDispatchTable {
    PrResult(PRAPI_CALL* CreateInstance)(const char*, uint32_t, PrInstance*);
    void      (PRAPI_CALL* DestroyInstance)(PrInstance);
    ������������� ���� ����� API� */
} PrIcdDispatchTable;
