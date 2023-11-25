# ----------------
# Experiments

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Smoke Test
add_executable(SmokeTest main.cpp)
target_compile_options(SmokeTest PRIVATE -Ofast)

add_executable(FloatVsDouble misc/FloatVsDouble.cpp)
target_compile_options(SmokeTest PRIVATE -Ofast)

add_executable(AbstractGraph AbstractGraph/main.cpp AbstractGraph/Graph.cpp)
target_compile_options(AbstractGraph PRIVATE -Ofast)

add_executable(ThreadSync misc/threadSync.cpp)
target_compile_options(ThreadSync PRIVATE -Ofast)

add_executable(PersistentWorkers misc/persistentWorkers.cpp)
target_compile_options(PersistentWorkers PRIVATE -Ofast)
