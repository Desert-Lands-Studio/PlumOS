cmake_minimum_required(VERSION 3.20)
project(PlumUIDemo LANGUAGES C)

add_executable(plumui_demo demo_selector.c)

target_include_directories(plumui_demo PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
    ${CMAKE_CURRENT_BINARY_DIR}/../../generated
)

target_link_libraries(plumui_demo PlumUI)

if(PLUM_PDK STREQUAL "X11")
    target_link_libraries(plumui_demo ${X11_LIBRARIES})
    target_include_directories(plumui_demo PRIVATE ${X11_INCLUDE_DIR})
endif()

if(PLUM_ENABLE_OPENGL)
    target_link_libraries(plumui_demo OpenGL::OpenGL)
endif()