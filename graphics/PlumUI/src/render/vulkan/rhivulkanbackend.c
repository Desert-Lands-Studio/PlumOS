
#include "rhivulkanbackend.h"
#include <vulkan/vulkan.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>


#if defined(VK_USE_PLATFORM_XLIB_KHR)
#include <vulkan/vulkan_xlib.h>
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
#include <vulkan/vulkan_wayland.h>
#elif defined(VK_USE_PLATFORM_WIN32_KHR)
#include <vulkan/vulkan_win32.h>
#endif

typedef struct {
    PlumRhi base;
    VkInstance instance;
    VkPhysicalDevice physical_device;
    VkDevice device;
    VkQueue queue;
    VkSurfaceKHR surface;
} VulkanBackend;

static bool is_vulkan_available(void) {
    VkApplicationInfo app_info = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "PlumVulkanCheck",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "PlumUI",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_3
    };
    
    VkInstanceCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &app_info
    };
    
    VkInstance instance;
    VkResult result = vkCreateInstance(&create_info, NULL, &instance);
    if (result == VK_SUCCESS) {
        vkDestroyInstance(instance, NULL);
        return true;
    }
    return false;
}

static void destroy_vulkan(PlumRhi* rhi) {
    if (!rhi) return;
    VulkanBackend* backend = (VulkanBackend*)rhi;
    
    if (backend->surface != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(backend->instance, backend->surface, NULL);
    }
    if (backend->device) {
        vkDestroyDevice(backend->device, NULL);
    }
    if (backend->instance) {
        vkDestroyInstance(backend->instance, NULL);
    }
    free(backend);
}

static PlumRhi* create_vulkan(const PlumRhiVersion* requested_version) {
    printf("Creating Vulkan backend, requested version: %d.%d\n", 
           requested_version->major, requested_version->minor);
    
    
    uint32_t api_version;
    vkEnumerateInstanceVersion(&api_version);
    
    
    uint32_t requested_api = VK_MAKE_VERSION(requested_version->major, 
                                           requested_version->minor, 0);
    if (requested_api > api_version) {
        printf("Requested Vulkan %d.%d not available, using highest supported\n",
               requested_version->major, requested_version->minor);
        requested_api = api_version;
    }
    
    VulkanBackend* backend = (VulkanBackend*)malloc(sizeof(VulkanBackend));
    if (!backend) return NULL;
    
    VkApplicationInfo app_info = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "PlumUI Application",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "PlumUI",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = requested_api
    };
    
    
    const char* extensions[] = {
        VK_KHR_SURFACE_EXTENSION_NAME,
#if defined(VK_USE_PLATFORM_WIN32_KHR)
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
        VK_KHR_XLIB_SURFACE_EXTENSION_NAME,
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
        VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME,
#endif
        NULL
    };
    
    uint32_t extension_count = 2; 
    
    VkInstanceCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &app_info,
        .enabledExtensionCount = extension_count,
        .ppEnabledExtensionNames = extensions
    };
    
    if (vkCreateInstance(&create_info, NULL, &backend->instance) != VK_SUCCESS) {
        fprintf(stderr, "Failed to create Vulkan instance\n");
        free(backend);
        return NULL;
    }
    
    
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(backend->instance, &device_count, NULL);
    
    if (device_count == 0) {
        fprintf(stderr, "No Vulkan devices found\n");
        vkDestroyInstance(backend->instance, NULL);
        free(backend);
        return NULL;
    }
    
    VkPhysicalDevice* devices = malloc(sizeof(VkPhysicalDevice) * device_count);
    vkEnumeratePhysicalDevices(backend->instance, &device_count, devices);
    backend->physical_device = devices[0]; 
    free(devices);
    
    
    float queue_priority = 1.0f;
    VkDeviceQueueCreateInfo queue_create_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = 0,
        .queueCount = 1,
        .pQueuePriorities = &queue_priority
    };
    
    const char* device_extensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    
    VkDeviceCreateInfo device_create_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queue_create_info,
        .enabledExtensionCount = 1,
        .ppEnabledExtensionNames = device_extensions
    };
    
    if (vkCreateDevice(backend->physical_device, &device_create_info, 
                      NULL, &backend->device) != VK_SUCCESS) {
        fprintf(stderr, "Failed to create Vulkan device\n");
        vkDestroyInstance(backend->instance, NULL);
        free(backend);
        return NULL;
    }
    
    vkGetDeviceQueue(backend->device, 0, 0, &backend->queue);
    backend->surface = VK_NULL_HANDLE;
    
    static const PlumRhiInfo g_vk_info = {
        PLUM_RHI_VULKAN,
        "Vulkan",
        {1, 3},
        85,
        is_vulkan_available,
        create_vulkan,
        destroy_vulkan
    };
    
    backend->base.info = &g_vk_info;
    backend->base.version.major = VK_VERSION_MAJOR(api_version);
    backend->base.version.minor = VK_VERSION_MINOR(api_version);
    
    printf("✅ Vulkan backend created successfully: Version %d.%d\n",
           backend->base.version.major, backend->base.version.minor);
    
    return &backend->base;
}

__attribute__((constructor))
static void register_vulkan(void) {
    static const PlumRhiInfo g_vk_info = {
        PLUM_RHI_VULKAN,
        "Vulkan", 
        {1, 3},
        85,
        is_vulkan_available,
        create_vulkan,
        destroy_vulkan
    };
    plum_rhi_register(&g_vk_info);
}