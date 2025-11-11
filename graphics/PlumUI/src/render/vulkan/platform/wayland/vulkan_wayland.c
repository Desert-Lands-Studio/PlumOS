#include "render/vulkan/rhivulkanbackend.h"
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_wayland.h>
#include <wayland-client.h>
#include <stdlib.h>
#include <stdio.h>
#include "pdk/pdkwindow.h"

struct PdkWindow {
    struct wl_display* display;
    struct wl_surface* surface;
    struct xdg_surface* xdg_surface;
    struct xdg_toplevel* xdg_toplevel;
    struct wl_compositor* compositor;
    struct xdg_wm_base* xdg_wm_base;
    int width;
    int height;
    char* title;
};

VkSurfaceKHR create_vulkan_surface_wayland(VkInstance instance, void* window_handle) {
    if (!instance || !window_handle) return VK_NULL_HANDLE;

    struct PdkWindow* win = (struct PdkWindow*)window_handle;
    if (!win->display || !win->surface) {
        fprintf(stderr, "❌ Wayland: Missing display or surface in PdkWindow\n");
        return VK_NULL_HANDLE;
    }

    VkWaylandSurfaceCreateInfoKHR create_info = {
        .sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
        .pNext = NULL,
        .flags = 0,
        .display = win->display,
        .surface = win->surface
    };

    VkSurfaceKHR surface;
    VkResult result = vkCreateWaylandSurfaceKHR(instance, &create_info, NULL, &surface);
    if (result != VK_SUCCESS) {
        fprintf(stderr, "❌ Failed to create Wayland Vulkan surface: %d\n", result);
        return VK_NULL_HANDLE;
    }

    printf("✅ Vulkan Wayland surface created\n");
    return surface;
}

const char** get_wayland_required_extensions(uint32_t* count) {
    static const char* extensions[] = {
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME,
        NULL
    };
    *count = 2;
    return extensions;
}

VkPhysicalDevice select_wayland_physical_device(VkInstance instance) {
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(instance, &device_count, NULL);
    if (device_count == 0) return VK_NULL_HANDLE;

    VkPhysicalDevice* devices = malloc(sizeof(VkPhysicalDevice) * device_count);
    vkEnumeratePhysicalDevices(instance, &device_count, devices);

    for (uint32_t i = 0; i < device_count; ++i) {
        uint32_t queue_family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(devices[i], &queue_family_count, NULL);
        VkQueueFamilyProperties* queue_families = malloc(sizeof(VkQueueFamilyProperties) * queue_family_count);
        vkGetPhysicalDeviceQueueFamilyProperties(devices[i], &queue_family_count, queue_families);

        for (uint32_t j = 0; j < queue_family_count; ++j) {
            if (queue_families[j].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                free(queue_families);
                free(devices);
                return devices[i];
            }
        }
        free(queue_families);
    }

    free(devices);
    return VK_NULL_HANDLE;
}