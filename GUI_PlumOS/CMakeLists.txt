cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(GUI_PlumOS)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "Configurations" FORCE)
set(CMAKE_BUILD_TYPE "Release")

set(DEPENDENCIES_DIR "${CMAKE_SOURCE_DIR}/dependencies")
set(ARCHITECTURE "x64")

set(DEPENDENCIES 
    SDL2
    IMGUI
    VULKAN
    CURL
    ZLIB
    PNG
)

foreach(dep ${DEPENDENCIES})
    set(${dep}_DIR "${DEPENDENCIES_DIR}/${dep}")
    set(${dep}_INCLUDE_DIR "${${dep}_DIR}/include")
    set(${dep}_LIB_DIR "${${dep}_DIR}/lib/${ARCHITECTURE}")

    if(EXISTS "${${dep}_DIR}" AND EXISTS "${${dep}_INCLUDE_DIR}" AND EXISTS "${${dep}_LIB_DIR}")
        message(STATUS "${dep} found at ${${dep}_DIR}")
    else()
        message(WARNING "${dep} directory not found at ${${dep}_DIR}")
    endif()
endforeach()

set(IMGUI_SOURCES
    ${IMGUI_INCLUDE_DIR}/imgui.cpp
    ${IMGUI_INCLUDE_DIR}/imgui_draw.cpp
    ${IMGUI_INCLUDE_DIR}/imgui_tables.cpp
    ${IMGUI_INCLUDE_DIR}/imgui_widgets.cpp
    ${IMGUI_INCLUDE_DIR}/backends/imgui_impl_sdl2.cpp
    ${IMGUI_INCLUDE_DIR}/backends/imgui_impl_vulkan.cpp
)

add_executable(GUI_PlumOS src/main.cpp ${IMGUI_SOURCES})

if(WIN32)
    message(STATUS "Configuring for Windows")

    include_directories(
        ${IMGUI_INCLUDE_DIR}
        ${IMGUI_INCLUDE_DIR}/backends
        ${SDL2_INCLUDE_DIR}
        ${VULKAN_INCLUDE_DIR}
        ${CURL_INCLUDE_DIR}
        ${ZLIB_INCLUDE_DIR}
        ${PNG_INCLUDE_DIR}
    )

    target_link_libraries(GUI_PlumOS PRIVATE
        ${SDL2_LIB_DIR}/SDL2.lib
        ${VULKAN_LIB_DIR}/Vulkan-1.lib
        ${IMGUI_LIB_DIR}/imgui.lib
        ${CURL_LIB_DIR}/libcurl.lib
        ${ZLIB_LIB_DIR}/zlib.lib
        ${PNG_LIB_DIR}/libpng16.lib
    )

else()
    message(STATUS "Configuring for FreeBSD")

    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SDL2 REQUIRED sdl2)
    pkg_check_modules(Vulkan REQUIRED vulkan)
    pkg_check_modules(CURL REQUIRED libcurl)
    pkg_check_modules(ZLIB REQUIRED zlib)
    pkg_check_modules(PNG REQUIRED libpng)
    pkg_check_modules(IMGUI REQUIRED imgui)

    include_directories(
        ${IMGUI_INCLUDE_DIR}
        ${IMGUI_INCLUDE_DIR}/backends
        ${SDL2_INCLUDE_DIR}
        ${VULKAN_INCLUDE_DIR}
        ${CURL_INCLUDE_DIR}
        ${ZLIB_INCLUDE_DIR}
        ${PNG_INCLUDE_DIR}
    )

    target_link_libraries(GUI_PlumOS PRIVATE
        ${SDL2_LIBRARIES}
        ${Vulkan_LIBRARIES}
        ${IMGUI_LIBRARIES}
        ${CURL_LIBRARIES}
        ${ZLIB_LIBRARIES}
        ${PNG_LIBRARIES}
    )
endif()

# Каталоги для линковки
link_directories(
    ${SDL2_LIB_DIR}
    ${VULKAN_LIB_DIR}
    ${CURL_LIB_DIR}
    ${ZLIB_LIB_DIR}
    ${PNG_LIB_DIR}
    ${IMGUI_LIB_DIR}
)
