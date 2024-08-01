#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <GLES3/gl3.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <iostream>
#include <vector>
#include <chrono>

// Константы и переменные
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

EGLDisplay eglDisplay;
EGLSurface eglSurface;
EGLContext eglContext;
EGLConfig eglConfig;

GLuint program;
GLuint VAO, VBO;
GLuint attrPos, attrColor;
float angle = 0.0f;
std::chrono::steady_clock::time_point lastTime;
int frameCount = 0;

bool initializeEGL(SDL_Window* window) {
    eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (eglDisplay == EGL_NO_DISPLAY) {
        std::cerr << "Failed to get EGL display" << std::endl;
        return false;
    }

    if (!eglInitialize(eglDisplay, nullptr, nullptr)) {
        std::cerr << "Failed to initialize EGL" << std::endl;
        return false;
    }

    EGLint configAttribs[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,  // Используем EGL_OPENGL_ES2_BIT вместо EGL_OPENGL_ES3_BIT_KHR
        EGL_CONFORMANT, EGL_OPENGL_ES2_BIT,       // Используем EGL_OPENGL_ES2_BIT вместо EGL_OPENGL_ES3_BIT_KHR
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_BLUE_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_RED_SIZE, 8,
        EGL_DEPTH_SIZE, 24,
        EGL_NONE
    };

    EGLint numConfigs;
    if (!eglChooseConfig(eglDisplay, configAttribs, &eglConfig, 1, &numConfigs)) {
        std::cerr << "Failed to choose EGL config" << std::endl;
        return false;
    }

    EGLint contextAttribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 3,
        EGL_NONE
    };

    eglContext = eglCreateContext(eglDisplay, eglConfig, EGL_NO_CONTEXT, contextAttribs);
    if (eglContext == EGL_NO_CONTEXT) {
        std::cerr << "Failed to create EGL context" << std::endl;
        return false;
    }

    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    SDL_GetWindowWMInfo(window, &info);
#if defined(SDL_VIDEO_DRIVER_X11)
    EGLNativeWindowType nativeWindow = info.info.x11.window;
#elif defined(SDL_VIDEO_DRIVER_WINDOWS)
    EGLNativeWindowType nativeWindow = info.info.win.window;
#elif defined(SDL_VIDEO_DRIVER_WAYLAND)
    EGLNativeWindowType nativeWindow = info.info.wl.surface;
#else
    std::cerr << "Unsupported SDL window manager" << std::endl;
    return false;
#endif

    eglSurface = eglCreateWindowSurface(eglDisplay, eglConfig, nativeWindow, nullptr);
    if (eglSurface == EGL_NO_SURFACE) {
        std::cerr << "Failed to create EGL surface" << std::endl;
        return false;
    }

    if (!eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext)) {
        std::cerr << "Failed to make EGL context current" << std::endl;
        return false;
    }

    return true;
}

void renderOpenGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(program);

    angle += 0.01f;
    if (angle >= 360.0f) {
        angle -= 360.0f;
    }

    // Вращение и трансформация куба (например, с помощью матрицы модели)

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    eglSwapBuffers(eglDisplay, eglSurface);
}

void updateFPS() {
    auto currentTime = std::chrono::steady_clock::now();
    frameCount++;

    std::chrono::duration<float> elapsed = currentTime - lastTime;
    if (elapsed.count() >= 1.0f) {
        std::cout << "FPS: " << frameCount / elapsed.count() << std::endl;
        lastTime = currentTime;
        frameCount = 0;
    }
}

GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint compileStatus;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
    if (compileStatus == GL_FALSE) {
        GLint infoLogLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
        std::vector<GLchar> infoLog(infoLogLength);
        glGetShaderInfoLog(shader, infoLogLength, &infoLogLength, infoLog.data());
        std::cerr << "Shader compile error: " << infoLog.data() << std::endl;
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

GLuint createProgram(const char* vertexSource, const char* fragmentSource) {
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    GLint linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    if (linkStatus == GL_FALSE) {
        GLint infoLogLength;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
        std::vector<GLchar> infoLog(infoLogLength);
        glGetProgramInfoLog(program, infoLogLength, &infoLogLength, infoLog.data());
        std::cerr << "Program link error: " << infoLog.data() << std::endl;
        glDeleteProgram(program);
        return 0;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

bool handleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            return false;
        }
    }
    return true;
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("OpenGL ES 3.1 Example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    if (!initializeEGL(window)) {
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Определение шейдеров и создание программы
    const char* vertexSource = R"(
        #version 310 es
        uniform mat4 modelViewProjection;
        in vec4 aPos;
        in vec4 aColor;
        out vec4 fragColor;
        void main() {
            gl_Position = modelViewProjection * aPos;
            fragColor = aColor;
        }
    )";

    const char* fragmentSource = R"(
        #version 310 es
        precision mediump float;
        in vec4 fragColor;
        out vec4 color;
        void main() {
            color = fragColor;
        }
    )";

    program = createProgram(vertexSource, fragmentSource);
    if (program == 0) {
        eglDestroyContext(eglDisplay, eglContext);
        eglDestroySurface(eglDisplay, eglSurface);
        eglTerminate(eglDisplay);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

	// Вершинные данные для куба
float vertices[] = {
    // Позиции         // Цвета
    -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, // Красный
     0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, // Зеленый
     0.5f,  0.5f, -0.5f, 0.0f, 0.0f, 1.0f, // Синий
    -0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 0.0f, // Желтый
    
    -0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 1.0f, // Пурпурный
     0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 1.0f, // Циан
     0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f, // Белый
    -0.5f,  0.5f,  0.5f, 1.0f, 0.5f, 0.0f, // Оранжевый
    
    -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, // Красный
     0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, // Зеленый
     0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 1.0f, // Синий
    -0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 0.0f, // Желтый
    
    -0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 1.0f, // Пурпурный
     0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 1.0f, // Циан
     0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f, // Белый
    -0.5f,  0.5f,  0.5f, 1.0f, 0.5f, 0.0f  // Оранжевый
};

	 glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Устанавливаем атрибуты вершин
    attrPos = glGetAttribLocation(program, "aPos");
    attrColor = glGetAttribLocation(program, "aColor");

    glVertexAttribPointer(attrPos, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(attrPos);

    glVertexAttribPointer(attrColor, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(attrColor);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    lastTime = std::chrono::steady_clock::now();

    bool running = true;
    while (running) {
        running = handleEvents();
        renderOpenGL();
        updateFPS();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(program);

    eglDestroyContext(eglDisplay, eglContext);
    eglDestroySurface(eglDisplay, eglSurface);
    eglTerminate(eglDisplay);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}