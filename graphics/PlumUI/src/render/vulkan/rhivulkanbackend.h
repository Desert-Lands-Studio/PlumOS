#ifndef PLUM_RHI_VULKAN_BACKEND_H
#define PLUM_RHI_VULKAN_BACKEND_H

#include "rhi.h"
#include "pdk/pdktypes.h"

#ifdef __cplusplus
extern "C" {
#endif


#if defined(VK_USE_PLATFORM_XLIB_KHR)
VkSurfaceKHR create_vulkan_surface_x11(VkInstance instance, void* window_handle);
const char** get_x11_required_extensions(uint32_t* count);
VkPhysicalDevice select_x11_physical_device(VkInstance instance);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
VkSurfaceKHR create_vulkan_surface_wayland(VkInstance instance, struct wl_display* display, struct wl_surface* surface);
const char** get_wayland_required_extensions(uint32_t* count);
VkPhysicalDevice select_wayland_physical_device(VkInstance instance);
#elif defined(VK_USE_PLATFORM_WIN32_KHR)
VkSurfaceKHR create_vulkan_surface_win32(VkInstance instance, void* window_handle);
const char** get_win32_required_extensions(uint32_t* count);
VkPhysicalDevice select_win32_physical_device(VkInstance instance);
#endif

#ifdef __cplusplus
}
#endif

#endif /* PLUM_RHI_VULKAN_BACKEND_H */