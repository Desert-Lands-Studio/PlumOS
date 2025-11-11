#include "rhivulkanbackend.h"
#include <vulkan/vulkan_win32.h>
#include <windows.h>

VkSurfaceKHR create_vulkan_surface_win32(VkInstance instance, void* window_handle) {
    VkWin32SurfaceCreateInfoKHR create_info = {
        .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
        .hinstance = GetModuleHandle(NULL),
        .hwnd = (HWND)window_handle
    };

    VkSurfaceKHR surface;
    if (vkCreateWin32SurfaceKHR(instance, &create_info, NULL, &surface) != VK_SUCCESS) {
        return VK_NULL_HANDLE;
    }

    return surface;
}


const char** get_win32_required_extensions(uint32_t* count) {
    static const char* extensions[] = {
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME  
    };
    
    *count = 3;
    return extensions;
}


VkPhysicalDevice select_win32_physical_device(VkInstance instance) {
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(instance, &device_count, NULL);
    
    if (device_count == 0) {
        return VK_NULL_HANDLE;
    }

    VkPhysicalDevice* devices = malloc(sizeof(VkPhysicalDevice) * device_count);
    vkEnumeratePhysicalDevices(instance, &device_count, devices);

    
    VkPhysicalDevice selected_device = VK_NULL_HANDLE;
    int highest_score = -1;

    for (uint32_t i = 0; i < device_count; i++) {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(devices[i], &props);

        int score = 0;
        
        
        if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            score += 1000;
        } else if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
            score += 500;
        }

        
        score += props.apiVersion;

        if (score > highest_score) {
            highest_score = score;
            selected_device = devices[i];
        }
    }

    free(devices);
    return selected_device;
}