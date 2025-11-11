cmake_minimum_required(VERSION 3.20)
project(PlumUI VERSION 0.2.0 LANGUAGES C CXX)

if(APPLE)
    enable_language(OBJC OBJCXX)
    if(CMAKE_OBJCXX_COMPILER)
        set(CMAKE_OBJCXX_STANDARD 17)
        set(CMAKE_OBJCXX_STANDARD_REQUIRED ON)
    endif()
endif()

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
include(CheckIncludeFile)
include(CheckFunctionExists)

# ============================================================================
# ШАГ 1: ОПРЕДЕЛЕНИЕ ОПЦИЙ ПОЛЬЗОВАТЕЛЯ
# ============================================================================

option(BUILD_SHARED_LIBS "Build shared libraries" ON)

option(PLUM_ENABLE_THREADS  "Enable threading support" ON)
option(PLUM_ENABLE_ASSERT   "Enable runtime assertions" ON)
option(PLUM_ENABLE_LOG      "Enable logging" ON)
option(PLUM_ENABLE_FREETYPE "Enable FreeType text rendering" ON)
option(PLUM_ENABLE_HARFBUZZ "Enable HarfBuzz shaping" ON)
option(PLUM_ENABLE_ICU      "Enable ICU text features" ON)
option(PLUM_ENABLE_WIDGETS  "Build Widgets subsystem" ON)

# Опции RHI
option(PLUM_ENABLE_OPENGL    "Enable OpenGL backend" ON)
option(PLUM_ENABLE_OPENGLES  "Enable OpenGL ES backend" ON)
option(PLUM_ENABLE_VULKAN    "Enable Vulkan backend" ON)
option(PLUM_ENABLE_METAL     "Enable Metal backend" OFF)
option(PLUM_ENABLE_DIRECTX12 "Enable DirectX12 backend" OFF)

set(PLUM_DEFAULT_RHI "OpenGL" CACHE STRING "Default rendering backend")
set_property(CACHE PLUM_DEFAULT_RHI PROPERTY STRINGS OpenGL OpenGLES Vulkan Metal DirectX12)

# Опция PDK
set(PLUM_PDK "Auto" CACHE STRING "Choose PDK (Auto|Win32|X11|Wayland|Quartz|Pane)")
set_property(CACHE PLUM_PDK PROPERTY STRINGS Auto Win32 X11 Wayland Quartz Pane)

# ============================================================================
# ШАГ 2: АВТОВЫБОР И ПРОВЕРКА PDK
# ============================================================================

if(PLUM_PDK STREQUAL "Auto")
    if(WIN32)
        set(PLUM_PDK "Win32")
    elseif(APPLE)
        set(PLUM_PDK "Quartz")
    else()
        find_package(PkgConfig QUIET)
        if(PKG_CONFIG_FOUND)
            pkg_check_modules(WAYLAND_PROTOCOLS QUIET wayland-protocols>=1.31)
            pkg_check_modules(WAYLAND_CLIENT QUIET wayland-client>=1.22)
            if(WAYLAND_PROTOCOLS_FOUND AND WAYLAND_CLIENT_FOUND)
                set(PLUM_PDK "Wayland")
            else()
                set(PLUM_PDK "X11")
            endif()
        else()
            set(PLUM_PDK "X11")
        endif()
    endif()
endif()

# Проверка совместимости PDK с платформой
if(PLUM_PDK STREQUAL "Win32" AND NOT WIN32)
    message(FATAL_ERROR "Win32 PDK can only be used on Windows")
elseif(PLUM_PDK STREQUAL "Quartz" AND NOT APPLE)
    message(FATAL_ERROR "Quartz PDK can only be used on macOS")
elseif(PLUM_PDK MATCHES "Wayland|X11" AND WIN32)
    message(FATAL_ERROR "Wayland/X11 PDK cannot be used on Windows")
endif()

message(STATUS "Selected PDK: ${PLUM_PDK}")

# ============================================================================
# ШАГ 3: ПОИСК СИСТЕМНЫХ ЗАВИСИМОСТЕЙ
# ============================================================================

find_package(Threads REQUIRED)

# ============================================================================
# ШАГ 4: ПОИСК БИБЛИОТЕК RHI (RENDERING BACKENDS)
# ============================================================================

# OpenGL
if(PLUM_ENABLE_OPENGL)
    if(PLUM_PDK STREQUAL "X11" AND NOT WIN32)
        find_package(X11 REQUIRED)
    endif()
    
    find_package(OpenGL REQUIRED)
    
    # Создаем цель OpenGL если она не была создана find_package
    if(OpenGL_FOUND AND NOT TARGET OpenGL::OpenGL)
        add_library(OpenGL::OpenGL INTERFACE IMPORTED)
        if(WIN32)
            target_link_libraries(OpenGL::OpenGL INTERFACE opengl32)
            if(OPENGL_glu_FOUND)
                target_link_libraries(OpenGL::OpenGL INTERFACE glu32)
            endif()
        else()
            if(OPENGL_LIBRARIES)
                target_link_libraries(OpenGL::OpenGL INTERFACE ${OPENGL_LIBRARIES})
            else()
                target_link_libraries(OpenGL::OpenGL INTERFACE OpenGL::GL)
            endif()
        endif()
        if(OPENGL_INCLUDE_DIR)
            target_include_directories(OpenGL::OpenGL INTERFACE ${OPENGL_INCLUDE_DIR})
        endif()
    endif()
endif()

# OpenGL ES
if(PLUM_ENABLE_OPENGLES)
    if(PLUM_PDK STREQUAL "X11" AND NOT WIN32)
        find_package(X11 REQUIRED)
    endif()
    
    # Поиск OpenGL ES библиотек
    if(WIN32)
        find_path(OPENGLES2_INCLUDE_DIR GLES2/gl2.h)
        find_library(OPENGLES2_LIBRARY libGLESv2)
    else()
        find_package(PkgConfig QUIET)
        if(PKG_CONFIG_FOUND)
            pkg_check_modules(OPENGLES2 QUIET glesv2)
            if(OPENGLES2_FOUND)
                set(OPENGLES2_LIBRARIES ${OPENGLES2_LINK_LIBRARIES})
                set(OPENGLES2_INCLUDE_DIRS ${OPENGLES2_INCLUDE_DIRS})
            else()
                find_library(OPENGLES2_LIBRARY NAMES GLESv2)
                find_path(OPENGLES2_INCLUDE_DIR GLES2/gl2.h)
            endif()
        else()
            find_library(OPENGLES2_LIBRARY NAMES GLESv2)
            find_path(OPENGLES2_INCLUDE_DIR GLES2/gl2.h)
        endif()
    endif()
    
    if(NOT OPENGLES2_LIBRARY OR NOT OPENGLES2_INCLUDE_DIR)
        message(WARNING "OpenGL ES not found, disabling PLUM_ENABLE_OPENGLES")
        set(PLUM_ENABLE_OPENGLES OFF)
    else()
        add_library(OpenGLES::OpenGLES INTERFACE IMPORTED)
        target_link_libraries(OpenGLES::OpenGLES INTERFACE ${OPENGLES2_LIBRARY})
        if(OPENGLES2_INCLUDE_DIR)
            target_include_directories(OpenGLES::OpenGLES INTERFACE ${OPENGLES2_INCLUDE_DIR})
        endif()
    endif()
endif()

# Vulkan
if(PLUM_ENABLE_VULKAN)
    find_package(Vulkan QUIET)
    if(NOT Vulkan_FOUND)
        message(WARNING "Vulkan not found, disabling PLUM_ENABLE_VULKAN")
        set(PLUM_ENABLE_VULKAN OFF)
    endif()
endif()

# Metal (только для macOS)
if(PLUM_ENABLE_METAL AND NOT APPLE)
    message(WARNING "Metal backend is only available on macOS. Disabling Metal.")
    set(PLUM_ENABLE_METAL OFF)
endif()

# DirectX12 (только для Windows)
if(PLUM_ENABLE_DIRECTX12 AND NOT WIN32)
    message(WARNING "DirectX12 backend is only available on Windows. Disabling DirectX12.")
    set(PLUM_ENABLE_DIRECTX12 OFF)
endif()

# ============================================================================
# ШАГ 5: ПОИСК ТЕКСТОВЫХ БИБЛИОТЕК
# ============================================================================

# FreeType
if(PLUM_ENABLE_FREETYPE)
    find_package(PkgConfig QUIET)
    if(PKG_CONFIG_FOUND)
        pkg_check_modules(FREETYPE QUIET freetype2)
        if(FREETYPE_FOUND)
            message(STATUS "Found FreeType via pkg-config: ${FREETYPE_VERSION}")
            set(FREETYPE_LIBRARIES ${FREETYPE_LINK_LIBRARIES})
            set(FREETYPE_INCLUDE_DIRS ${FREETYPE_INCLUDE_DIRS})
        else()
            find_package(Freetype QUIET)
            if(Freetype_FOUND)
                set(FREETYPE_LIBRARIES Freetype::Freetype)
                message(STATUS "Found FreeType via CMake")
            else()
                message(WARNING "FreeType not found, disabling PLUM_ENABLE_FREETYPE")
                set(PLUM_ENABLE_FREETYPE OFF)
            endif()
        endif()
    else()
        find_package(Freetype QUIET)
        if(Freetype_FOUND)
            set(FREETYPE_LIBRARIES Freetype::Freetype)
            message(STATUS "Found FreeType via CMake")
        else()
            message(WARNING "FreeType not found, disabling PLUM_ENABLE_FREETYPE")
            set(PLUM_ENABLE_FREETYPE OFF)
        endif()
    endif()
endif()

# HarfBuzz
if(PLUM_ENABLE_HARFBUZZ)
    find_package(PkgConfig QUIET)
    if(PKG_CONFIG_FOUND)
        pkg_check_modules(HARFBUZZ QUIET harfbuzz)
        if(HARFBUZZ_FOUND)
            message(STATUS "Found HarfBuzz via pkg-config: ${HARFBUZZ_VERSION}")
            set(HARFBUZZ_LIBRARIES ${HARFBUZZ_LINK_LIBRARIES})
            set(HARFBUZZ_INCLUDE_DIRS ${HARFBUZZ_INCLUDE_DIRS})
        else()
            find_package(HarfBuzz QUIET)
            if(HarfBuzz_FOUND)
                set(HARFBUZZ_LIBRARIES HarfBuzz::HarfBuzz)
                message(STATUS "Found HarfBuzz via CMake")
            else()
                message(WARNING "HarfBuzz not found, disabling PLUM_ENABLE_HARFBUZZ")
                set(PLUM_ENABLE_HARFBUZZ OFF)
            endif()
        endif()
    else()
        find_package(HarfBuzz QUIET)
        if(HarfBuzz_FOUND)
            set(HARFBUZZ_LIBRARIES HarfBuzz::HarfBuzz)
            message(STATUS "Found HarfBuzz via CMake")
        else()
            message(WARNING "HarfBuzz not found, disabling PLUM_ENABLE_HARFBUZZ")
            set(PLUM_ENABLE_HARFBUZZ OFF)
        endif()
    endif()
endif()

# ICU
if(PLUM_ENABLE_ICU)
    find_package(PkgConfig QUIET)
    if(PKG_CONFIG_FOUND)
        pkg_check_modules(ICU QUIET icu-uc icu-io)
        if(ICU_FOUND)
            message(STATUS "Found ICU via pkg-config")
            set(ICU_LIBRARIES ${ICU_LINK_LIBRARIES})
        else()
            find_package(ICU QUIET)
            if(ICU_FOUND)
                set(ICU_LIBRARIES ICU::uc ICU::io)
                message(STATUS "Found ICU via CMake")
            else()
                message(WARNING "ICU not found, disabling PLUM_ENABLE_ICU")
                set(PLUM_ENABLE_ICU OFF)
            endif()
        endif()
    else()
        find_package(ICU QUIET)
        if(ICU_FOUND)
            set(ICU_LIBRARIES ICU::uc ICU::io)
            message(STATUS "Found ICU via CMake")
        else()
            message(WARNING "ICU not found, disabling PLUM_ENABLE_ICU")
            set(PLUM_ENABLE_ICU OFF)
        endif()
    endif()
endif()

# ============================================================================
# ШАГ 6: ПОИСК PDK-СПЕЦИФИЧНЫХ БИБЛИОТЕК
# ============================================================================

if(PLUM_PDK STREQUAL "Wayland")
    if(NOT WIN32)
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(WAYLAND REQUIRED wayland-client wayland-cursor)
        pkg_check_modules(WAYLAND_PROTOCOLS REQUIRED wayland-protocols>=1.31)
        find_program(WAYLAND_SCANNER_EXECUTABLE NAMES wayland-scanner REQUIRED)
        
        if(PLUM_ENABLE_OPENGL OR PLUM_ENABLE_OPENGLES)
            pkg_check_modules(EGL QUIET egl)
            if(NOT EGL_FOUND)
                message(WARNING "EGL not found, may affect OpenGL on Wayland")
            endif()
        endif()
    endif()
    
elseif(PLUM_PDK STREQUAL "X11")
    if(NOT WIN32)
        find_package(X11 REQUIRED)
    endif()
    
elseif(PLUM_PDK STREQUAL "Quartz" AND APPLE)
    find_library(COCOA_LIBRARY Cocoa)
    find_library(APPKIT_LIBRARY AppKit)
    find_library(FOUNDATION_LIBRARY Foundation)
    find_library(COREGRAPHICS_LIBRARY CoreGraphics)
    
elseif(PLUM_PDK STREQUAL "Win32" AND WIN32)
    # Системные библиотеки Windows уже доступны
endif()

# ============================================================================
# ШАГ 7: ГЕНЕРАЦИЯ КОНФИГУРАЦИОННЫХ ФАЙЛОВ
# ============================================================================

set(PLUM_CONF_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(PLUM_CONF_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(PLUM_CONF_VERSION_PATCH ${PROJECT_VERSION_PATCH})

set(PLUM_CONF_ENABLE_THREADS ${PLUM_ENABLE_THREADS})
set(PLUM_CONF_ENABLE_ASSERT ${PLUM_ENABLE_ASSERT})
set(PLUM_CONF_ENABLE_LOG ${PLUM_ENABLE_LOG})
set(PLUM_CONF_ENABLE_FREETYPE ${PLUM_ENABLE_FREETYPE})
set(PLUM_CONF_ENABLE_HARFBUZZ ${PLUM_ENABLE_HARFBUZZ})
set(PLUM_CONF_ENABLE_ICU ${PLUM_ENABLE_ICU})
set(PLUM_CONF_ENABLE_WIDGETS ${PLUM_ENABLE_WIDGETS})

if(CMAKE_SYSTEM_PROCESSOR MATCHES "i386|i686")
    set(PLUM_ARCH_X86 1)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "amd64|x86_64")
    set(PLUM_ARCH_X86_64 1)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "armv7")
    set(PLUM_ARCH_ARM 1)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
    set(PLUM_ARCH_ARM64 1)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "riscv64")
    set(PLUM_ARCH_RISCV64 1)
endif()

if(PLUM_PDK STREQUAL "Win32")
    set(PLUM_PLATFORM_WIN32 1)
elseif(PLUM_PDK STREQUAL "X11")
    set(PLUM_PLATFORM_X11 1)
elseif(PLUM_PDK STREQUAL "Wayland")
    set(PLUM_PLATFORM_WAYLAND 1)
elseif(PLUM_PDK STREQUAL "Quartz")
    set(PLUM_PLATFORM_QUARTZ 1)
elseif(PLUM_PDK STREQUAL "Pane")
    set(PLUM_PLATFORM_PANE 1)
endif()

set(PLUM_CONF_HAS_OPENGL ${PLUM_ENABLE_OPENGL})
set(PLUM_CONF_HAS_OPENGLES ${PLUM_ENABLE_OPENGLES})
set(PLUM_CONF_HAS_VULKAN ${PLUM_ENABLE_VULKAN})
set(PLUM_CONF_HAS_METAL ${PLUM_ENABLE_METAL})
set(PLUM_CONF_HAS_DIRECTX12 ${PLUM_ENABLE_DIRECTX12})

if(PLUM_DEFAULT_RHI STREQUAL "OpenGL")
    set(PLUM_CONF_DEFAULT_RHI "PLUM_RHI_OPENGL")
elseif(PLUM_DEFAULT_RHI STREQUAL "OpenGLES")
    set(PLUM_CONF_DEFAULT_RHI "PLUM_RHI_OPENGLES")
elseif(PLUM_DEFAULT_RHI STREQUAL "Vulkan")
    set(PLUM_CONF_DEFAULT_RHI "PLUM_RHI_VULKAN")
elseif(PLUM_DEFAULT_RHI STREQUAL "Metal")
    set(PLUM_CONF_DEFAULT_RHI "PLUM_RHI_METAL")
elseif(PLUM_DEFAULT_RHI STREQUAL "DirectX12")
    set(PLUM_CONF_DEFAULT_RHI "PLUM_RHI_DIRECTX12")
endif()

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/generated)

configure_file(src/core/plumconfig.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/generated/plumconfig.h @ONLY)

# ============================================================================
# ШАГ 8: ОПРЕДЕЛЕНИЕ ИСТОЧНИКОВ
# ============================================================================

set(PLUM_CORE_SOURCES
    src/core/plumallocator.c
    src/core/plumbuffer.c
    src/core/plum.c
    src/core/plumhash.c
    src/core/plumlist.c
    src/core/plummath.c
    src/core/plumobject.c
)

set(PLUM_PSK_SOURCES
    src/psk/pskboundingbox.c
    src/psk/pskpath.c
    src/psk/pskrenderer.c
    src/psk/pskrendernode.c
    src/psk/pskstroke.c
    src/psk/psktransform.c
    src/psk/gpu/pskgpubuffer.c
    src/psk/gpu/pskgpudevice.c
    src/psk/gpu/pskgpuimage.c
)

set(PLUM_RENDER_SOURCES
    src/render/loader.c
    src/render/rhi.c
    src/render/rhi_registry.c
)

set(PLUM_WIDGET_SOURCES
    # ========== Application ==========
    src/widgets/application/plumapplication.c

    # ========== Base widgets ==========
    src/widgets/base/widget/plumwidget.c

    # ========== Containers ==========
    src/widgets/containers/actionbar/plumactionbar.c 
    src/widgets/containers/aspectframe/plumaspectframe.c 
    src/widgets/containers/centerbox/plumcenterbox.c 
    src/widgets/containers/constraintlayout/plumconstraintlayout.c 
    src/widgets/containers/dialog/plumdialog.c
    src/widgets/containers/dockwidget/plumdockwidget.c
    src/widgets/containers/groupbox/plumgroupbox.c
    src/widgets/containers/scrollarea/plumscrollarea.c
    src/widgets/containers/splitter/plumsplitter.c
    src/widgets/containers/stackedwidget/plumstackedwidget.c
    src/widgets/containers/tabbar/plumtabbar.c
    src/widgets/containers/tabwidget/plumtabwidget.c

    # ========== Controls ==========
    src/widgets/controls/buttons/plumbutton.c
    src/widgets/controls/buttons/color_button/plumcolorbutton.c
    src/widgets/controls/buttons/file_button/plumfilebutton.c
    src/widgets/controls/buttons/font_button/plumfontbutton.c
    src/widgets/controls/buttons/plumcheckbox.c
    src/widgets/controls/buttons/plumradiobutton.c
    src/widgets/controls/displays/plumlabel.c
    src/widgets/controls/displays/progress/plumprogressbar.c
    src/widgets/controls/displays/editable_label/plumeditablelabel.c
    src/widgets/controls/input/plumentry.c
    src/widgets/controls/input/plumscrollbar.c
    src/widgets/controls/input/plumslider.c
    src/widgets/controls/input/plumspinbox.c
    src/widgets/controls/input/numeric/plumnumericstepper.c
    src/widgets/controls/input/search/plumsearchentry.c
    src/widgets/controls/selectors/plumdropdown.c
    src/widgets/controls/selectors/scale/plumscale.c
    src/widgets/controls/selectors/list/plumlistselector.c 

    # ========== Dialogs ==========
    src/widgets/dialogs/messagebox/plummessagebox.c
    src/widgets/dialogs/modaldialog/plummodaldialog.c

    # ========== Indicators ==========
    src/widgets/indicators/notification/plumnotification.c
    src/widgets/indicators/throbber/plumthrobber.c

    # ========== Layouts ==========
    src/widgets/layouts/plumlayout.c
    src/widgets/layouts/fixedlayout/plumfixedlayout.c
    src/widgets/layouts/flowlayout/plumflowlayout.c
    src/widgets/layouts/formlayout/plumformlayout.c
    src/widgets/layouts/gridlayout/plumgridlayout.c
    src/widgets/layouts/hboxlayout/plumhboxlayout.c
    src/widgets/layouts/vboxlayout/plumvboxlayout.c

    # ========== Navigation ==========
    src/widgets/navigation/menubar/plummenubar.c
    src/widgets/navigation/popupmenu/plumpopupmenu.c
    src/widgets/navigation/statusbar/plumstatusbar.c
    src/widgets/navigation/toolbar/plumtoolbar.c

    # ========== Views ==========
    src/widgets/views/canvas/plumcanvas.c
    src/widgets/views/iconview/plumiconview.c
    src/widgets/views/webview/plumwebview.c

    # ========== Windows ==========
    src/widgets/windows/plumwindow.c
)

# ────────────────────── PDK выбор ──────────────────────────────
set(PLATFORM_SOURCES "")
set(PLATFORM_GENERATED_SOURCES "")

if(PLUM_PDK STREQUAL "Win32")
    list(APPEND PLATFORM_SOURCES
        src/pdk/platform/WDM/pdkcursor-win32.c
        src/pdk/platform/WDM/pdkdevice-win32.c
        src/pdk/platform/WDM/pdkdisplay-win32.c
        src/pdk/platform/WDM/pdkwindow-win32.c
    )
elseif(PLUM_PDK STREQUAL "X11")
    if(NOT WIN32)
        list(APPEND PLATFORM_SOURCES
            src/pdk/platform/X11/pdkcursor-x11.c
            src/pdk/platform/X11/pdkdevice-x11.c
            src/pdk/platform/X11/pdkdisplay-x11.c
            src/pdk/platform/X11/pdkwindow-x11.c
        )
    endif()
elseif(PLUM_PDK STREQUAL "Wayland")
    if(NOT WIN32)
        list(APPEND PLATFORM_SOURCES
            src/pdk/platform/Wayland/pdkcursor-wayland.c
            src/pdk/platform/Wayland/pdkdevice-wayland.c
            src/pdk/platform/Wayland/pdkdisplay-wayland.c
            src/pdk/platform/Wayland/pdkwindow-wayland.c
        )
        
        if(WAYLAND_PROTOCOLS_FOUND)
            set(WAYLAND_PROTOCOLS_DIR "${WAYLAND_PROTOCOLS_PREFIX}/share/wayland-protocols")
            
            add_custom_command(
                OUTPUT 
                    ${CMAKE_CURRENT_BINARY_DIR}/generated/xdg-shell-protocol.c
                    ${CMAKE_CURRENT_BINARY_DIR}/generated/xdg-shell-client-protocol.h
                COMMAND ${WAYLAND_SCANNER_EXECUTABLE} private-code
                    ${WAYLAND_PROTOCOLS_DIR}/stable/xdg-shell/xdg-shell.xml
                    ${CMAKE_CURRENT_BINARY_DIR}/generated/xdg-shell-protocol.c
                COMMAND ${WAYLAND_SCANNER_EXECUTABLE} client-header
                    ${WAYLAND_PROTOCOLS_DIR}/stable/xdg-shell/xdg-shell.xml
                    ${CMAKE_CURRENT_BINARY_DIR}/generated/xdg-shell-client-protocol.h
                DEPENDS ${WAYLAND_PROTOCOLS_DIR}/stable/xdg-shell/xdg-shell.xml
                COMMENT "Generating xdg-shell protocol"
            )
            
            list(APPEND PLATFORM_GENERATED_SOURCES
                ${CMAKE_CURRENT_BINARY_DIR}/generated/xdg-shell-protocol.c
            )
        endif()
    endif()
elseif(PLUM_PDK STREQUAL "Quartz")
    if(APPLE)
        list(APPEND PLATFORM_SOURCES
            src/pdk/platform/Cocoa/pdkwindow-cocoa.mm
            src/pdk/platform/Cocoa/pdkapplication-cocoa.mm
            src/pdk/platform/Cocoa/pdkcursor-cocoa.mm
            src/pdk/platform/Cocoa/pdkdevice-cocoa.mm
            src/pdk/platform/Cocoa/pdkdisplay-cocoa.mm
        )
    endif()
endif()

# ────────────────────── RHI бэкенды ────────────────────────────
if(PLUM_ENABLE_OPENGL)
    list(APPEND PLUM_RENDER_SOURCES src/render/opengl/rhiglbackend.c)

    if(PLUM_PDK STREQUAL "Win32")
        list(APPEND PLUM_RENDER_SOURCES src/render/opengl/platform/win32/glcontext_wgl.c)
    elseif(PLUM_PDK STREQUAL "Wayland")
        if(NOT WIN32)
            list(APPEND PLUM_RENDER_SOURCES src/render/opengl/platform/wayland/glcontext_egl.c)
        endif()
    elseif(PLUM_PDK STREQUAL "X11")
        if(NOT WIN32)
            list(APPEND PLUM_RENDER_SOURCES src/render/opengl/platform/x11/glcontext_glx.c)
        endif()
    elseif(PLUM_PDK STREQUAL "Quartz")
        if(APPLE)
            list(APPEND PLUM_RENDER_SOURCES src/render/opengl/platform/quartz/glcontext_cgl.c)
        endif()
    elseif(PLUM_PDK STREQUAL "Pane")
        list(APPEND PLUM_RENDER_SOURCES src/render/opengl/platform/pane/glcontext_plumpane.c)
    endif()
endif()

if(PLUM_ENABLE_OPENGLES)
    list(APPEND PLUM_RENDER_SOURCES src/render/opengles/rhi_gles_backend.c)

    if(PLUM_PDK STREQUAL "Win32")
        list(APPEND PLUM_RENDER_SOURCES src/render/opengles/platform/win32/glescontext_win32.c)
    elseif(PLUM_PDK STREQUAL "Wayland")
        if(NOT WIN32)
            list(APPEND PLUM_RENDER_SOURCES src/render/opengles/platform/wayland/glescontext_wayland.c)
        endif()
    elseif(PLUM_PDK STREQUAL "X11")
        if(NOT WIN32)
            list(APPEND PLUM_RENDER_SOURCES src/render/opengles/platform/x11/glescontext_x11.c)
        endif()
    elseif(PLUM_PDK STREQUAL "Quartz")
        if(APPLE)
            list(APPEND PLUM_RENDER_SOURCES src/render/opengles/platform/quartz/glescontext_macos.c)
        endif()
    elseif(PLUM_PDK STREQUAL "Pane")
        list(APPEND PLUM_RENDER_SOURCES src/render/opengles/platform/pane/glescontext_plumpane.c)
    endif()
endif()

if(PLUM_ENABLE_VULKAN)
    list(APPEND PLUM_RENDER_SOURCES src/render/vulkan/rhivulkanbackend.c)

    if(PLUM_PDK STREQUAL "Win32")
        list(APPEND PLUM_RENDER_SOURCES src/render/vulkan/platform/win32/vulkan_win32.c)
    elseif(PLUM_PDK STREQUAL "Wayland")
        if(NOT WIN32)
            list(APPEND PLUM_RENDER_SOURCES src/render/vulkan/platform/wayland/vulkan_wayland.c)
        endif()
    elseif(PLUM_PDK STREQUAL "X11")
        if(NOT WIN32)
            list(APPEND PLUM_RENDER_SOURCES src/render/vulkan/platform/x11/vulkan_x11.c)
        endif()
    elseif(PLUM_PDK STREQUAL "Quartz")
        if(APPLE)
            list(APPEND PLUM_RENDER_SOURCES src/render/vulkan/platform/quartz/vulkan_macos.c)
        endif()
    elseif(PLUM_PDK STREQUAL "Pane")
        list(APPEND PLUM_RENDER_SOURCES src/render/vulkan/platform/pane/vulkan_plumpane.c)
    endif()
endif()

if(PLUM_ENABLE_METAL)
    if(APPLE)
        list(APPEND PLUM_RENDER_SOURCES src/render/metal/rhimetalbackend.mm)
    endif()
endif()

if(PLUM_ENABLE_DIRECTX12)
    if(WIN32)
        list(APPEND PLUM_RENDER_SOURCES src/render/directx/rhid3d12backend.c)
    endif()
endif()

# ============================================================================
# ШАГ 9: СОЗДАНИЕ И ЛИНКОВКА БИБЛИОТЕК
# ============================================================================

# Общая конфигурация include директорий для всех целей
set(PLUM_COMMON_INCLUDES
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/generated>
)

# Core Library
add_library(PlumCore ${PLUM_CORE_SOURCES})
target_include_directories(PlumCore
    PUBLIC
        ${PLUM_COMMON_INCLUDES}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core
        ${CMAKE_CURRENT_BINARY_DIR}
)

target_compile_definitions(PlumCore PRIVATE
    $<$<BOOL:${BUILD_SHARED_LIBS}>:PLUM_BUILD_SHARED>
)

# ЛИНКОВКА Core Library
target_link_libraries(PlumCore PUBLIC Threads::Threads)

# Текстовые зависимости для Core
if(PLUM_ENABLE_FREETYPE AND FREETYPE_LIBRARIES)
    target_include_directories(PlumCore PRIVATE ${FREETYPE_INCLUDE_DIRS})
    target_link_libraries(PlumCore PUBLIC ${FREETYPE_LIBRARIES})
endif()

if(PLUM_ENABLE_HARFBUZZ AND HARFBUZZ_LIBRARIES)
    target_include_directories(PlumCore PRIVATE ${HARFBUZZ_INCLUDE_DIRS})
    target_link_libraries(PlumCore PUBLIC ${HARFBUZZ_LIBRARIES})
endif()

if(PLUM_ENABLE_ICU AND ICU_LIBRARIES)
    target_link_libraries(PlumCore PUBLIC ${ICU_LIBRARIES})
endif()

# PSK Library
add_library(PlumPSK ${PLUM_PSK_SOURCES})
target_include_directories(PlumPSK
    PUBLIC
        ${PLUM_COMMON_INCLUDES}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core
        ${CMAKE_CURRENT_SOURCE_DIR}/src/psk
        ${CMAKE_CURRENT_BINARY_DIR}
)

target_compile_definitions(PlumPSK PRIVATE
    $<$<BOOL:${BUILD_SHARED_LIBS}>:PLUM_BUILD_SHARED>
)

# ЛИНКОВКА PSK Library
target_link_libraries(PlumPSK PUBLIC PlumCore)

# Render Library
add_library(PlumRender ${PLUM_RENDER_SOURCES})
target_include_directories(PlumRender
    PUBLIC
        ${PLUM_COMMON_INCLUDES}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core
        ${CMAKE_CURRENT_SOURCE_DIR}/src/render
        ${CMAKE_CURRENT_BINARY_DIR}
)

target_compile_definitions(PlumRender PRIVATE
    $<$<BOOL:${BUILD_SHARED_LIBS}>:PLUM_BUILD_SHARED>
)

# ЛИНКОВКА Render Library
target_link_libraries(PlumRender PUBLIC PlumCore PlumPSK)

# RHI зависимости для Render
if(PLUM_ENABLE_OPENGL)
    target_link_libraries(PlumRender PRIVATE OpenGL::OpenGL)
    
    if(NOT WIN32 AND NOT APPLE)
        target_link_libraries(PlumRender PRIVATE OpenGL::GL)
    endif()

    if(PLUM_PDK STREQUAL "Wayland" AND EGL_FOUND)
        target_link_libraries(PlumRender PRIVATE ${EGL_LIBRARIES})
        if(EGL_INCLUDE_DIRS)
            target_include_directories(PlumRender PRIVATE ${EGL_INCLUDE_DIRS})
        endif()
    endif()
endif()

if(PLUM_ENABLE_OPENGLES)
    target_link_libraries(PlumRender PRIVATE OpenGLES::OpenGLES)
    
    if(PLUM_PDK STREQUAL "Wayland" AND EGL_FOUND)
        target_link_libraries(PlumRender PRIVATE ${EGL_LIBRARIES})
        if(EGL_INCLUDE_DIRS)
            target_include_directories(PlumRender PRIVATE ${EGL_INCLUDE_DIRS})
        endif()
    endif()
endif()

if(PLUM_ENABLE_VULKAN)
    target_link_libraries(PlumRender PRIVATE Vulkan::Vulkan)
endif()

if(PLUM_ENABLE_DIRECTX12 AND WIN32)
    target_link_libraries(PlumRender PRIVATE d3d12 dxgi d3dcompiler)
endif()

# PDK Library (общие + платформенные источники)
add_library(PlumPDK ${PLATFORM_SOURCES} ${PLATFORM_GENERATED_SOURCES})
target_include_directories(PlumPDK
    PUBLIC
        ${PLUM_COMMON_INCLUDES}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core
        ${CMAKE_CURRENT_SOURCE_DIR}/src/pdk
        ${CMAKE_CURRENT_BINARY_DIR}
)

target_compile_definitions(PlumPDK PRIVATE
    $<$<BOOL:${BUILD_SHARED_LIBS}>:PLUM_BUILD_SHARED>
)

# ЛИНКОВКА PDK Library
target_link_libraries(PlumPDK PUBLIC PlumCore)

# PDK-специфичные зависимости
if(PLUM_PDK STREQUAL "Wayland" AND NOT WIN32)
    target_include_directories(PlumPDK PRIVATE ${WAYLAND_INCLUDE_DIRS})
    target_link_libraries(PlumPDK PRIVATE ${WAYLAND_LIBRARIES})
    
elseif(PLUM_PDK STREQUAL "X11" AND NOT WIN32)
    target_include_directories(PlumPDK PRIVATE ${X11_INCLUDE_DIR})
    target_link_libraries(PlumPDK PRIVATE ${X11_LIBRARIES})
    
elseif(PLUM_PDK STREQUAL "Quartz" AND APPLE)
    target_link_libraries(PlumPDK PRIVATE
        ${COCOA_LIBRARY}
        ${APPKIT_LIBRARY}
        ${FOUNDATION_LIBRARY}
        ${COREGRAPHICS_LIBRARY}
    )
    
elseif(PLUM_PDK STREQUAL "Win32" AND WIN32)
    target_link_libraries(PlumPDK PRIVATE
        user32
        gdi32
        shell32
        dwmapi
        shcore
    )
endif()

# Widgets Library (если включена)
if(PLUM_ENABLE_WIDGETS)
    add_library(PlumWidgets ${PLUM_WIDGET_SOURCES})
    target_include_directories(PlumWidgets
        PUBLIC
            ${PLUM_COMMON_INCLUDES}
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/src/core
            ${CMAKE_CURRENT_SOURCE_DIR}/src/render
            ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets
            ${CMAKE_CURRENT_BINARY_DIR}
    )
    
    target_compile_definitions(PlumWidgets PRIVATE
        $<$<BOOL:${BUILD_SHARED_LIBS}>:PLUM_BUILD_SHARED>
    )
    
    # ЛИНКОВКА Widgets Library
    target_link_libraries(PlumWidgets PUBLIC PlumCore PlumPSK PlumRender PlumPDK)
endif()

# Umbrella Library
add_library(PlumUI INTERFACE)
target_link_libraries(PlumUI INTERFACE PlumCore PlumPSK PlumRender PlumPDK)
if(PLUM_ENABLE_WIDGETS)
    target_link_libraries(PlumUI INTERFACE PlumWidgets)
endif()

# ────────────────────── macOS Framework ───────────────────────
if(APPLE AND BUILD_SHARED_LIBS)
    set_target_properties(PlumCore PlumPSK PlumRender PlumPDK PROPERTIES
        FRAMEWORK TRUE
        FRAMEWORK_VERSION A
        MACOSX_FRAMEWORK_IDENTIFIER com.plumui
        MACOSX_FRAMEWORK_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_FRAMEWORK_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    )
    if(PLUM_ENABLE_WIDGETS)
        set_target_properties(PlumWidgets PROPERTIES
            FRAMEWORK TRUE
            FRAMEWORK_VERSION A
            MACOSX_FRAMEWORK_IDENTIFIER com.plumui.widgets
            MACOSX_FRAMEWORK_BUNDLE_VERSION ${PROJECT_VERSION}
            MACOSX_FRAMEWORK_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        )
    endif()
endif()

# ============================================================================
# ШАГ 10: УСТАНОВКА И УПАКОВКА
# ============================================================================

install(TARGETS PlumCore PlumPSK PlumRender PlumPDK
    EXPORT PlumUITargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

if(PLUM_ENABLE_WIDGETS)
    install(TARGETS PlumWidgets EXPORT PlumUITargets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
endif()

install(TARGETS PlumUI EXPORT PlumUITargets)

install(DIRECTORY src/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/PlumUI
    FILES_MATCHING 
        PATTERN "*.h"
        PATTERN "*.hpp"
        PATTERN "generated" EXCLUDE
        PATTERN "*.in" EXCLUDE
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/generated/plumconfig.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/PlumUI
)

configure_package_config_file(
    cmake/PlumUIConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/PlumUIConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/PlumUI
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/PlumUIConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/PlumUIConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/PlumUIConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/PlumUI
)

install(EXPORT PlumUITargets
    NAMESPACE PlumUI::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/PlumUI
)

# ────────────────────── Windows test executable ────────────────────────
if(WIN32 AND (PLUM_ENABLE_OPENGL OR PLUM_ENABLE_OPENGLES OR PLUM_ENABLE_VULKAN OR PLUM_ENABLE_DIRECTX12))
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/examples/win32_test.c)
        add_executable(win32_test examples/win32_test.c)
        target_link_libraries(win32_test PlumUI)

        if(BUILD_SHARED_LIBS)
            add_custom_command(TARGET win32_test POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:PlumCore> $<TARGET_FILE_DIR:win32_test>
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:PlumPSK> $<TARGET_FILE_DIR:win32_test>
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:PlumRender> $<TARGET_FILE_DIR:win32_test>
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:PlumPDK> $<TARGET_FILE_DIR:win32_test>
                COMMENT "Copying PlumUI DLLs for Windows test"
            )
        endif()
    endif()
endif()

# Добавляем примеры
add_subdirectory(examples/hello_world)
add_subdirectory(examples/demo_selector) 

# ────────────────────── Сводка ────────────────────────────────
set(ENABLED_RHI_LIST "")
if(PLUM_ENABLE_OPENGL)
    list(APPEND ENABLED_RHI_LIST "OpenGL")
endif()
if(PLUM_ENABLE_OPENGLES)
    list(APPEND ENABLED_RHI_LIST "OpenGL ES")
endif()
if(PLUM_ENABLE_VULKAN)
    list(APPEND ENABLED_RHI_LIST "Vulkan")
endif()
if(PLUM_ENABLE_METAL)
    list(APPEND ENABLED_RHI_LIST "Metal")
endif()
if(PLUM_ENABLE_DIRECTX12)
    list(APPEND ENABLED_RHI_LIST "DirectX12")
endif()

if(NOT ENABLED_RHI_LIST)
    set(ENABLED_RHI_LIST_STR "None")
else()
    string(REPLACE ";" ", " ENABLED_RHI_LIST_STR "${ENABLED_RHI_LIST}")
endif()

message(STATUS "")
message(STATUS "PlumUI configuration summary:")
message(STATUS "  Version:        ${PROJECT_VERSION}")
message(STATUS "  Platform:       ${CMAKE_SYSTEM_NAME}")
message(STATUS "  Architecture:   ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "  Compiler:       ${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER_VERSION}")
message(STATUS "  Build Type:     ${CMAKE_BUILD_TYPE}")
message(STATUS "  Install Prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "")
message(STATUS "  Enabled RHI:    ${ENABLED_RHI_LIST_STR}")
message(STATUS "  Enabled PDK:    ${PLUM_PDK}")
message(STATUS "  Widgets:        ${PLUM_ENABLE_WIDGETS}")
message(STATUS "  Default RHI:    ${PLUM_DEFAULT_RHI}")
message(STATUS "  Shared Libs:    ${BUILD_SHARED_LIBS}")
message(STATUS "")
message(STATUS "  Text Features:")
message(STATUS "    FreeType:     ${PLUM_ENABLE_FREETYPE}")
message(STATUS "    HarfBuzz:     ${PLUM_ENABLE_HARFBUZZ}")
message(STATUS "    ICU:          ${PLUM_ENABLE_ICU}")
message(STATUS "")
message(STATUS "  System Features:")
message(STATUS "    Threads:      ${PLUM_ENABLE_THREADS}")
message(STATUS "    Assertions:   ${PLUM_ENABLE_ASSERT}")
message(STATUS "    Logging:      ${PLUM_ENABLE_LOG}")
message(STATUS "===========================================================")