cmake_minimum_required(VERSION 3.15)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
project(SmokeTestProject C)

# Set target flags per compiler
if (MSVC)
    add_compile_options(/W4 /WX)
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} )
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} )
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} )
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR} )
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR} )
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR} )
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR} )
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR} )
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR} )
else()
	add_compile_options(-Wall -Wextra -Werror)
endif()

# =========================
# bench.dll (Shared Library)
# =========================
add_library(bench SHARED bench.c)

# =========================
# smoketest.exe
# =========================
add_executable(smoketest smoketest.c)
add_executable(nbody nbody.c)

if (NOT MSVC)
    target_link_libraries(nbody PRIVATE m)
	target_link_libraries(bench PRIVATE m)
endif()

# Link smoketest with bench library
target_link_libraries(smoketest PRIVATE bench)

# Equivalent of `make run`
add_custom_target(run
    COMMAND smoketest
    DEPENDS smoketest
    USES_TERMINAL
)

add_custom_target(nbody-run
    COMMAND nbody
    DEPENDS nbody
    USES_TERMINAL
)
