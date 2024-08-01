#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <GL/glew.h>
#include <vulkan/vulkan.h>
#include <iostream>
#include <vector>

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

enum RenderMode {
    OpenGLMode,
    VulkanMode
};

// Обработка событий SDL
bool handleEvents(RenderMode &mode) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            return false;
        }
        if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_SPACE) {
                mode = (mode == OpenGLMode) ? VulkanMode : OpenGLMode;
            }
        }
    }
    return true;
}

// Рендеринг с использованием OpenGL
void renderOpenGL() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Рендеринг простого прямоугольника
    glBegin(GL_QUADS);
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex2f(-0.5f, -0.5f);
    glVertex2f( 0.5f, -0.5f);
    glVertex2f( 0.5f,  0.5f);
    glVertex2f(-0.5f,  0.5f);
    glEnd();
}

// Рендеринг с использованием Vulkan (пока не реализован)
void renderVulkan(VkInstance instance, VkSurfaceKHR surface) {
    // Пример создания VkDevice и других объектов Vulkan
    VkDevice device;
    VkPhysicalDevice physicalDevice;
    VkSwapchainKHR swapChain;

    // Найти подходящее физическое устройство
    VkPhysicalDevice physicalDevices[10];
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices);

    // Выбор первого доступного физического устройства
    physicalDevice = physicalDevices[0];

    // Создание логического устройства
    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    // Установите параметры createInfo здесь

    vkCreateDevice(physicalDevice, &createInfo, nullptr, &device);

    // Настройка swapchain
    VkSwapchainCreateInfoKHR swapChainCreateInfo = {};
    swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    // Установите параметры swapChainCreateInfo здесь

    vkCreateSwapchainKHR(device, &swapChainCreateInfo, nullptr, &swapChain);

    // Рендеринг с использованием Vulkan
    // Напишите код для рендеринга с использованием созданных объектов

    std::cout << "Рендеринг с использованием Vulkan (реализован частично)" << std::endl;

    // Очистка ресурсов Vulkan
    vkDestroySwapchainKHR(device, swapChain, nullptr);
    vkDestroyDevice(device, nullptr);
}

int main(int argc, char* argv[]) {
    // Инициализация SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "Ошибка SDL_Init: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Создание окна
    SDL_Window* window = SDL_CreateWindow("Пример OpenGL и Vulkan", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_VULKAN);
    if (window == nullptr) {
        std::cerr << "Ошибка SDL_CreateWindow: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    // Настройка OpenGL
    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    if (glContext == nullptr) {
        std::cerr << "Ошибка SDL_GL_CreateContext: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Ошибка glewInit" << std::endl;
        SDL_GL_DeleteContext(glContext);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Настройка Vulkan
    VkInstance instance;
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Vulkan";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    unsigned int extensionCount = 0;
    if (!SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, nullptr)) {
        std::cerr << "Ошибка SDL_Vulkan_GetInstanceExtensions: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    std::vector<const char*> extensions(extensionCount);
    if (!SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, extensions.data())) {
        std::cerr << "Ошибка SDL_Vulkan_GetInstanceExtensions: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
        std::cerr << "Не удалось создать экземпляр Vulkan" << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    VkSurfaceKHR surface;
    if (!SDL_Vulkan_CreateSurface(window, instance, &surface)) {
        std::cerr << "Ошибка SDL_Vulkan_CreateSurface: " << SDL_GetError() << std::endl;
        vkDestroyInstance(instance, nullptr);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    RenderMode mode = OpenGLMode;
    bool running = true;
    while (running) {
        running = handleEvents(mode);
        if (mode == OpenGLMode) {
            SDL_GL_MakeCurrent(window, glContext);
            renderOpenGL();
            SDL_GL_SwapWindow(window);
        } else {
            renderVulkan(instance, surface);
        }
    }

    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
