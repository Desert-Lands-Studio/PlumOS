#include <SDL2/SDL.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <iostream>
#include <chrono>

// Размеры окна
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

// Временные переменные для FPS
std::chrono::steady_clock::time_point lastTime;
int frameCount = 0;

// Вершины и индексы куба
const GLfloat vertices[] = {
    -0.5f, -0.5f, -0.5f,  // 0
    -0.5f, -0.5f,  0.5f,  // 1
     0.5f, -0.5f,  0.5f,  // 2
     0.5f, -0.5f, -0.5f,  // 3
    -0.5f,  0.5f, -0.5f,  // 4
    -0.5f,  0.5f,  0.5f,  // 5
     0.5f,  0.5f,  0.5f,  // 6
     0.5f,  0.5f, -0.5f   // 7
};

const GLubyte indices[] = {
    0, 1, 2,  2, 3, 0,  // Bottom
    4, 5, 6,  6, 7, 4,  // Top
    0, 1, 5,  5, 4, 0,  // Front
    2, 3, 7,  7, 6, 2,  // Back
    0, 3, 7,  7, 4, 0,  // Left
    1, 2, 6,  6, 5, 1   // Right
};

GLuint vertexBuffer, indexBuffer, program;
GLint attrPos, uniMVP;
float angle = 0.0f;

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
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Обновляем матрицу модели
    angle += 0.01f;
    if (angle > 360.0f) angle -= 360.0f;
    
    float mvpMatrix[16] = {
        cosf(angle), -sinf(angle), 0.0f, 0.0f,
        sinf(angle),  cosf(angle), 0.0f, 0.0f,
        0.0f,        0.0f,       1.0f, 0.0f,
        0.0f,        0.0f,       0.0f, 1.0f
    };
    
    glUniformMatrix4fv(uniMVP, 1, GL_FALSE, mvpMatrix);

    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glVertexAttribPointer(attrPos, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(attrPos);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, (void*)0);
}

void updateFPS() {
    using namespace std::chrono;
    static auto start = steady_clock::now();
    frameCount++;
    auto now = steady_clock::now();
    auto duration = duration_cast<seconds>(now - start).count();
    if (duration >= 1) {
        std::cout << "FPS: " << frameCount / duration << std::endl;
        frameCount = 0;
        start = now;
    }
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("OpenGL ES 3.1 Example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
    if (window == nullptr) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    if (glContext == nullptr) {
        std::cerr << "SDL_GL_CreateContext Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    EGLDisplay eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (eglDisplay == EGL_NO_DISPLAY) {
        std::cerr << "EGL Error: No display found" << std::endl;
        SDL_GL_DeleteContext(glContext);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    if (!eglInitialize(eglDisplay, nullptr, nullptr)) {
        std::cerr << "EGL Error: Failed to initialize" << std::endl;
        SDL_GL_DeleteContext(glContext);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    EGLConfig eglConfig;
    EGLint numConfigs;
    EGLint eglAttribs[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR,
        EGL_CONFORMANT, EGL_OPENGL_ES3_BIT_KHR,
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_NONE
    };

    if (!eglChooseConfig(eglDisplay, eglAttribs, &eglConfig, 1, &numConfigs)) {
        std::cerr << "EGL Error: Failed to choose config" << std::endl;
        eglTerminate(eglDisplay);
        SDL_GL_DeleteContext(glContext);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    EGLSurface eglSurface = eglCreateWindowSurface(eglDisplay, eglConfig, (EGLNativeWindowType)window, nullptr);
    if (eglSurface == EGL_NO_SURFACE) {
        std::cerr << "EGL Error: Failed to create surface" << std::endl;
        eglTerminate(eglDisplay);
        SDL_GL_DeleteContext(glContext);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    EGLContext eglContext = eglCreateContext(eglDisplay, eglConfig, EGL_NO_CONTEXT, nullptr);
    if (eglContext == EGL_NO_CONTEXT) {
        std::cerr << "EGL Error: Failed to create context" << std::endl;
        eglDestroySurface(eglDisplay, eglSurface);
        eglTerminate(eglDisplay);
        SDL_GL_DeleteContext(glContext);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    if (!eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext)) {
        std::cerr << "EGL Error: Failed to make context current" << std::endl;
        eglDestroyContext(eglDisplay, eglContext);
        eglDestroySurface(eglDisplay, eglSurface);
        eglTerminate(eglDisplay);
        SDL_GL_DeleteContext(glContext);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    // Шейдеры
    const char* vertexShaderSource = R"(
        #version 300 es
        in vec3 aPos;
        uniform mat4 uMVP;
        void main() {
            gl_Position = uMVP * vec4(aPos, 1.0);
        }
    )";

    const char* fragmentShaderSource = R"(
        #version 300 es
        out vec4 FragColor;
        void main() {
            FragColor = vec4(1.0, 0.0, 0.0, 1.0);
        }
    )";

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    glUseProgram(program);

    attrPos = glGetAttribLocation(program, "aPos");
    uniMVP = glGetUniformLocation(program, "uMVP");

    // Создание буферов
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glEnable(GL_DEPTH_TEST);

    lastTime = std::chrono::steady_clock::now();

    bool running = true;
    while (running) {
        running = handleEvents();
        renderOpenGL();
        eglSwapBuffers(eglDisplay, eglSurface);
        updateFPS();
    }

    eglDestroyContext(eglDisplay, eglContext);
    eglDestroySurface(eglDisplay, eglSurface);
    eglTerminate(eglDisplay);
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
