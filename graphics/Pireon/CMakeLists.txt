cmake_minimum_required(VERSION 3.12)
project(Pireon LANGUAGES C)

option(PIREON_BUILD_TESTS "Build unit tests" ON)

# ───── ядро + загрузчик ─────────────────────────────────────────
add_library(pireon SHARED
    # core
    src/core/pireon_instance.c
    src/core/pireon_device.c
    src/core/pireon_stub.c          # пока заглушка swap-chain
    # loader
    src/loader/pireon_loader.c
)

# публичные заголовки
target_include_directories(pireon
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include          # <plum/pireon.h>
        ${CMAKE_CURRENT_SOURCE_DIR}/include/plum     # прямой путь
)

target_compile_features(pireon PUBLIC c_std_17)
set_target_properties(pireon PROPERTIES OUTPUT_NAME "pireon")

# ───── null-driver плагин ──────────────────────────────────────
add_library(pireon_null_drv SHARED
    src/drivers/null/pireon_null_device.c
)

target_include_directories(pireon_null_drv PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/plum
)

set_target_properties(pireon_null_drv PROPERTIES
    OUTPUT_NAME "pireon_null"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

# ───── примеры / тесты (если есть) ─────────────────────────────
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/examples)
    add_subdirectory(examples)
endif()

if(PIREON_BUILD_TESTS AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tests)
    enable_testing()
    add_subdirectory(tests)
endif()
