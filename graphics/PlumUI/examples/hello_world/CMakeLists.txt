cmake_minimum_required(VERSION 3.20)
project(PlumUITest LANGUAGES C)

add_executable(plumui_hello_world test_x11.c)

target_include_directories(plumui_hello_world PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}/generated
)

target_link_libraries(plumui_hello_world PlumUI)

if(PLUM_PDK STREQUAL "X11")
    target_link_libraries(plumui_hello_world ${X11_LIBRARIES})
    target_include_directories(plumui_hello_world PRIVATE ${X11_INCLUDE_DIR})
endif()

if(PLUM_ENABLE_OPENGL)
    target_link_libraries(plumui_hello_world OpenGL::OpenGL)
endif()