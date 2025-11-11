
#include "pdkwindow.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

struct PdkWindow {
    HWND hwnd;
    HINSTANCE hinstance;
    int width, height;
    const char* title;
};

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
            
        case WM_SIZE:
            
            break;
            
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
            EndPaint(hwnd, &ps);
            return 0;
        }
        
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

PdkWindow* pdk_window_create(const char* title, int width, int height) {
    HINSTANCE hInstance = GetModuleHandle(NULL);
    
    
    WNDCLASSEX wc = {
        .cbSize = sizeof(WNDCLASSEX),
        .style = CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc = WndProc,
        .hInstance = hInstance,
        .hCursor = LoadCursor(NULL, IDC_ARROW),
        .hbrBackground = (HBRUSH)(COLOR_WINDOW + 1),
        .lpszClassName = L"PlumWindowClass"
    };
    
    if (!RegisterClassEx(&wc)) {
        fprintf(stderr, "Failed to register window class\n");
        return NULL;
    }
    
    
    HWND hwnd = CreateWindowEx(
        0,
        L"PlumWindowClass",
        L"PlumUI Window",  
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        width, height,
        NULL, NULL, hInstance, NULL
    );
    
    if (!hwnd) {
        fprintf(stderr, "Failed to create window\n");
        return NULL;
    }
    
    PdkWindow* window = malloc(sizeof(PdkWindow));
    if (!window) {
        DestroyWindow(hwnd);
        return NULL;
    }
    
    window->hwnd = hwnd;
    window->hinstance = hInstance;
    window->width = width;
    window->height = height;
    window->title = title;
    
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    
    printf("✅ Win32 window created: %s (%dx%d)\n", title, width, height);
    return window;
}

void pdk_window_destroy(PdkWindow* window) {
    if (window) {
        if (window->hwnd) {
            DestroyWindow(window->hwnd);
        }
        free(window);
    }
}

void* pdk_window_get_native_handle(PdkWindow* window) {
    return window ? (void*)window->hwnd : NULL;
}

void pdk_window_poll_events(PdkWindow* window) {
    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        
        if (msg.message == WM_QUIT) {
            
            break;
        }
    }
}

int pdk_window_should_close(PdkWindow* window) {
    
    MSG msg;
    if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
        return msg.message == WM_QUIT;
    }
    return 0;
}