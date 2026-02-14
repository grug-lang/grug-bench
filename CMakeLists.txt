cmake_minimum_required(VERSION 3.15)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
project(SmokeTestProject C)

# Set target flags per compiler
if (MSVC)
    add_compile_options(/W4 /WX)
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

# Link smoketest with bench library
target_link_libraries(smoketest PRIVATE bench)

# Equivalent of `make run`
add_custom_target(run
    COMMAND smoketest
    DEPENDS smoketest
)
