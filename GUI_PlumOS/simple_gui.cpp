#include <SDL2/SDL.h>
#include <GLES3/gl3.h>
#include <EGL/egl.h>
#include <iostream>
#include <vector>
#include <chrono>

// Параметры окна
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

// Параметры вращающегося куба
float angle = 0.0f;

// Временные переменные для расчета FPS
std::chrono::steady_clock::time_point lastTime;
int frameCount = 0;

// Прототипы функций
bool handleEvents();
void renderOpenGL();
void updateFPS();

// Функции для компиляции шейдеров и создания программы
GLuint compileShader(GLenum type, const char* source);
GLuint createProgram(const char* vertexSource, const char* fragmentSource);

GLuint program;
GLuint VAO, VBO;
GLint attrPos, attrColor;

bool handleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            return false;
        }
    }
    return true;
}

void renderOpenGL() {
    // Очистка экрана
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Используем шейдерную программу
    glUseProgram(program);

    // Вращаем куб
    angle += 0.01f;
    if (angle >= 360.0f) {
        angle -= 360.0f;
    }

    // Устанавливаем матрицу модели
    // Создаем матрицу вращения (для простоты не включена)
    // Для полноценного вращения вам нужно установить правильные матрицы
    // glUniformMatrix4fv(mvpMatrixLocation, 1, GL_FALSE, &mvpMatrix[0][0]);

    // Рендеринг куба
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    // Обновляем экран
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

    // Настройка EGL
    EGLDisplay eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (eglDisplay == EGL_NO_DISPLAY) {
        std::cerr << "Failed to get EGL display" << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    if (!eglInitialize(eglDisplay, nullptr, nullptr)) {
        std::cerr << "Failed to initialize EGL" << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    EGLint configAttribs[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT, // Используем OpenGL ES 3.0
        EGL_CONFORMANT, EGL_OPENGL_ES3_BIT,
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_NONE
    };

    EGLConfig eglConfig;
    EGLint numConfigs;
    if (!eglChooseConfig(eglDisplay, configAttribs, &eglConfig, 1, &numConfigs)) {
        std::cerr << "Failed to choose EGL config" << std::endl;
        eglTerminate(eglDisplay);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    EGLSurface eglSurface = eglCreateWindowSurface(eglDisplay, eglConfig, window, nullptr);
    if (eglSurface == EGL_NO_SURFACE) {
        std::cerr << "Failed to create EGL surface" << std::endl;
        eglTerminate(eglDisplay);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    EGLint contextAttribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 3, // OpenGL ES 3.x
        EGL_NONE
    };

    EGLContext eglContext = eglCreateContext(eglDisplay, eglConfig, EGL_NO_CONTEXT, contextAttribs);
    if (eglContext == EGL_NO_CONTEXT) {
        std::cerr << "Failed to create EGL context" << std::endl;
        eglDestroySurface(eglDisplay, eglSurface);
        eglTerminate(eglDisplay);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    if (!eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext)) {
        std::cerr << "Failed to make EGL context current" << std::endl;
        eglDestroyContext(eglDisplay, eglContext);
        eglDestroySurface(eglDisplay, eglSurface);
        eglTerminate(eglDisplay);
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
        std::cerr << "Failed to create program" << std::endl;
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

	GLuint VBO, VAO;
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