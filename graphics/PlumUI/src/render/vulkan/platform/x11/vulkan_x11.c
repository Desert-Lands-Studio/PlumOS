#ifdef VK_USE_PLATFORM_XLIB_KHR

#include "rhivulkanbackend.h"
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_xlib.h>
#include <X11/Xlib.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

VkSurfaceKHR create_vulkan_surface_x11(VkInstance instance, void* window_handle) {
    if (!instance || !window_handle) {
        return VK_NULL_HANDLE;
    }

    
    Display* display = XOpenDisplay(NULL);
    if (!display) {
        fprintf(stderr, "Failed to open X11 display\n");
        return VK_NULL_HANDLE;
    }
    
    
    VkXlibSurfaceCreateInfoKHR create_info = {
        .sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
        .pNext = NULL,
        .flags = 0,
        .dpy = display,
        .window = (Window)(uintptr_t)window_handle
    };
    
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkResult result = vkCreateXlibSurfaceKHR(instance, &create_info, NULL, &surface);
    
    if (result != VK_SUCCESS) {
        fprintf(stderr, "Failed to create X11 Vulkan surface: %d\n", result);
        XCloseDisplay(display);
        return VK_NULL_HANDLE;
    }
    
    
    
    printf("✅ X11 Vulkan surface created successfully\n");
    return surface;
}

const char** get_x11_required_extensions(uint32_t* count) {
    static const char* extensions[] = {
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_XLIB_SURFACE_EXTENSION_NAME,
        NULL
    };
    
    *count = 2;
    return extensions;
}

VkPhysicalDevice select_x11_physical_device(VkInstance instance) {
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

#endif 