#include <stdio.h>
#include <stdlib.h>
#include "core/plum.h"

int main() {
    printf("=== Simple PlumUI Test ===\n\n");
    
    setenv("PLUM_FORCE_RHI", "OpenGL", 1);
    
    printf("1. Testing initialization...\n");
    if (!plum_init()) {
        printf("❌ plum_init() failed\n");
        return -1;
    }
    printf("✅ plum_init() successful\n");
    
    printf("2. Testing RHI context...\n");
    PlumRhi* rhi = plum_get_rhi_context();
    if (!rhi) {
        printf("❌ No RHI context\n");
    } else {
        printf("✅ RHI context obtained: %s\n", rhi->info->name);
    }
    
    printf("\n3. Cleaning up...\n");
    plum_quit();
    
    printf("\n🎉 Basic test completed!\n");
    return 0;
}