# ----------------
# Experiments

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Smoke Test
add_executable(SmokeTest main.cpp)
target_compile_options(SmokeTest PRIVATE -Ofast)
