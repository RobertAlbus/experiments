include(CMakeLists.vcpkg.txt)

cmake_minimum_required(VERSION 3.27)
project(cpp-experiments)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


set(flags -Ofast -g)

# ----------------
# Experiments

# Smoke Test
add_executable(SmokeTest main.cpp)
target_compile_options(SmokeTest PRIVATE ${flags})

add_executable(FloatVsDouble misc/FloatVsDouble.cpp)
target_compile_options(SmokeTest PRIVATE ${flags})

add_executable(AbstractGraph AbstractGraph/main.cpp AbstractGraph/Graph.cpp)
target_compile_options(AbstractGraph PRIVATE ${flags})

add_executable(ThreadSync misc/threadSync.cpp)
target_compile_options(ThreadSync PRIVATE ${flags})

add_executable(PersistentWorkers misc/persistentWorkers.cpp)
target_compile_options(PersistentWorkers PRIVATE ${flags})

add_executable(AsyncQueues WorkerPool/AsyncQueues.cpp)
target_compile_options(AsyncQueues PRIVATE ${flags})

# ----------------
# rxcpp

find_package(rxcpp CONFIG REQUIRED)

add_executable(RxCpp rxcpp/OscGen/main.cpp)
target_compile_options(RxCpp PRIVATE -Wdeprecated-declarations ${flags})
target_include_directories(RxCpp PUBLIC ${PROJECT_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/include)


