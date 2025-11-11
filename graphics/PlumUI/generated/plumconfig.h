/* ── PlumX/PlumUI/src/core/plumconfig.h.in ─────────────────────────────────────────── */
#pragma once
/*  plumconfig.h
 *  ──────────────────────────────────────────
 *  Генерируется CMake-ом → build/generated/plumconfig.h
 *  Редактировать нужно ТОЛЬКО .in-файл и CMake-флаги.
 */

#ifndef PLUM_CONFIG_H
#define PLUM_CONFIG_H

/* ─── Версия ─────────────────────────────────────────────────────────── */
#define PLUM_CONF_VERSION_MAJOR  0
#define PLUM_CONF_VERSION_MINOR  2
#define PLUM_CONF_VERSION_PATCH  0
#define PLUM_CONF_VERSION_STRING "0.2.0"

/* ─── Тип сборки ─────────────────────────────────────────────────────── */
/* #undef PLUM_CONF_DEBUG */
/* #undef PLUM_CONF_RELEASE */

/* ─── Архитектура ────────────────────────────────────────────────────── */
/* #undef PLUM_ARCH_X86 */
/* #undef PLUM_ARCH_X86_64 */
/* #undef PLUM_ARCH_ARM */
#define PLUM_ARCH_ARM64
/* #undef PLUM_ARCH_RISCV64 */

/* ─── Платформы (PDK) ───────────────────────────────────────────────── */
/* #undef PLUM_PLATFORM_WIN32 */
/* #undef PLUM_PLATFORM_X11 */
#define PLUM_PLATFORM_WAYLAND
/* #undef PLUM_PLATFORM_COCOA */
/* #undef PLUM_PLATFORM_PANE */

/* ─── RHI-бэкэнды  (⚠ имена НЕ совпадают с enum) ────────────────────── */
#define PLUM_CONF_HAS_OPENGL
/* #undef PLUM_CONF_HAS_OPENGLES */
#define PLUM_CONF_HAS_VULKAN
/* #undef PLUM_CONF_HAS_METAL */
/* #undef PLUM_CONF_HAS_DIRECTX12 */
/* #undef PLUM_CONF_HAS_OPENCL */
/* #undef PLUM_CONF_HAS_PLUMPAINT */

#define PLUM_CONF_RHI_DEFAULT ""

/* ─── Подсистемы / фичи ─────────────────────────────────────────────── */
#define PLUM_ENABLE_FREETYPE 1
#define PLUM_ENABLE_HARFBUZZ 1
#define PLUM_ENABLE_ICU 1
#define PLUM_ENABLE_THREADS 1
#define PLUM_ENABLE_ASSERT 1
#define PLUM_ENABLE_LOG 1
#define PLUM_ENABLE_TRACING 0

/* ─── Компилятор / ОС ───────────────────────────────────────────────── */
#if defined(_MSC_VER)
#  define PLUM_COMPILER_MSVC 1
#elif defined(__clang__)
#  define PLUM_COMPILER_CLANG 1
#elif defined(__GNUC__)
#  define PLUM_COMPILER_GCC  1
#else
#  define PLUM_COMPILER_UNKNOWN 1
#endif

#if defined(_WIN32)
#  define PLUM_OS_WINDOWS 1
#elif defined(__APPLE__)
#  include <TargetConditionals.h>
#  if TARGET_OS_OSX
#    define PLUM_OS_MACOS 1
#  elif TARGET_OS_IOS
#    define PLUM_OS_IOS 1
#  else
#    define PLUM_OS_APPLE_UNKNOWN 1
#  endif
#elif defined(__linux__)
#  define PLUM_OS_LINUX 1
#elif defined(__FreeBSD__)
#  define PLUM_OS_FREEBSD 1
#else
#  define PLUM_OS_UNKNOWN 1
#endif

/* ─── DLL-экспорт/импорт ────────────────────────────────────────────── */
#if defined(PLUM_OS_WINDOWS) && defined(PLUM_BUILD_SHARED)
#  if defined(PlumUI_EXPORTS)
#    define PLUM_API __declspec(dllexport)
#  else
#    define PLUM_API __declspec(dllimport)
#  endif
#elif (defined(PLUM_COMPILER_CLANG) || defined(PLUM_COMPILER_GCC)) && defined(PLUM_BUILD_SHARED)
#  define PLUM_API __attribute__((visibility("default")))
#else
#  define PLUM_API
#endif

/* ─── Вспомогательные макросы ───────────────────────────────────────── */
#if defined(PLUM_CONF_DEBUG)
#  define PLUM_DEBUG 1
#else
#  define PLUM_DEBUG 0
#endif

#endif /* PLUM_CONFIG_H */
