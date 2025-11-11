/* src/loader/pireon_loader.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pireon_core.h"
#include "pireon_icd.h"

#if defined(_WIN32)
#include <windows.h>
#define DL_HANDLE    HMODULE
#define dl_open(p)   LoadLibraryA(p)
#define dl_sym       GetProcAddress
#define dl_close     FreeLibrary
#else
#include <dlfcn.h>
#define DL_HANDLE    void*
#define dl_open(p)   dlopen(p, RTLD_NOW|RTLD_LOCAL)
#define dl_sym       dlsym
#define dl_close     dlclose
#endif

/* � ������ �����: ������� �������� � */
static PrIcdDispatchTable g_drv = { 0 };
static DL_HANDLE          g_lib = NULL;

/* � ���������� ������������� � */
static int prLoadDriver(void)
{
    if (g_lib) return 1;          /* ��� �������� */

    const char* drvName = getenv("PR_DRIVER");
    if (!drvName || !*drvName)
        drvName = "pireon_null";  /* fallback */

#if defined(_WIN32)
    char libFile[MAX_PATH];
    sprintf_s(libFile, MAX_PATH, "%s.dll", drvName);
#else
    char libFile[256];
    snprintf(libFile, sizeof(libFile), "lib%s.so", drvName);
#endif

    g_lib = dl_open(libFile);
    if (!g_lib) {
        fprintf(stderr, "[pireon] cannot load %s\n", libFile);
        return 0;
    }

    /* ���� ������� */
    PrResult(PRAPI_CALL * enumFn)(PrIcdDispatchTable*) =
        (void*)dl_sym(g_lib, "pr_icdEnumerateFunctions");
    if (!enumFn || enumFn(&g_drv) != PR_SUCCESS) {
        fprintf(stderr, "[pireon] %s missing dispatch table\n", libFile);
        dl_close(g_lib);
        g_lib = NULL;
        return 0;
    }
    return 1;
}

/* � ��������� ������� API � */
PrResult PRAPI_ATTR PRAPI_CALL
prCreateInstance(const char* app, uint32_t ver, PrInstance* inst)
{
    if (!prLoadDriver()) return PR_ERROR_INVALID_ARG;
    return g_drv.CreateInstance(app, ver, inst);
}

void PRAPI_ATTR PRAPI_CALL
prDestroyInstance(PrInstance inst)
{
    if (!g_drv.DestroyInstance) return;
    g_drv.DestroyInstance(inst);
}

/* TODO: �������� ��������� ������� ��������� �� ���� ����� API */
/* � */
