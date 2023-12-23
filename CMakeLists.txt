include(CMakeLists.vcpkg.txt)

cmake_minimum_required(VERSION 3.27)
project(cpp-experiments)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#--------------------------------
# SIMD detect/enable

list(APPEND COMMON_ARCH_FLAGS)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "(x86)|(X86)|(amd64)|(AMD64)")
        list(APPEND COMMON_ARCH_FLAGS "-msse")
    endif()

    if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
        list(APPEND COMMON_ARCH_FLAGS "-mavx")
    endif()
endif()


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

add_executable(ReinterpretCast misc/ReinterpretCast.cpp)
target_compile_options(ReinterpretCast PRIVATE)

add_executable(WorkerDelegation WorkerPool/WorkerDelegation.cpp)
target_compile_options(WorkerDelegation PRIVATE ${flags})


# ----------------
# rxcpp

find_package(rxcpp CONFIG REQUIRED)

add_executable(RxCppOscGen rxcpp/OscGen/main.cpp)
target_compile_options(RxCppOscGen PRIVATE -Wdeprecated-declarations ${flags})
target_include_directories(RxCppOscGen PUBLIC ${PROJECT_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/include)

add_executable(RxCppSideEffectTiming rxcpp/SideEffectTiming/main.cpp)
target_compile_options(RxCppSideEffectTiming PRIVATE ${flags})
target_include_directories(RxCppSideEffectTiming PUBLIC ${PROJECT_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/include)

add_executable(RxCppFunctionStreams rxcpp/FunctionStreams/main.cpp)
target_compile_options(RxCppFunctionStreams PRIVATE ${flags})
target_include_directories(RxCppFunctionStreams PUBLIC ${PROJECT_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/include)

# ----------------
# zero mq

find_package(cppzmq CONFIG REQUIRED)

add_executable(ZmqSockets zmq/sockets/main.cpp)
target_compile_options(ZmqSockets PRIVATE ${flags})
target_include_directories(ZmqSockets PUBLIC ${PROJECT_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/include)
target_link_libraries(ZmqSockets PUBLIC cppzmq)


# ----------------
# assembly

# LPF Coeffs non-optimized
add_executable(AssemblyLpfCoeffs assembly/LpfCoeffs/main.cpp)
target_compile_options(AssemblyLpfCoeffs PRIVATE -save-temps ${COMMON_ARCH_FLAGS})
# build/CMakeFiles/AssemblyLpfCoeffs.dir/assembly/LpfCoeffs/main.cpp.s

# LPF Coeffs optimized
add_executable(AssemblyLpfCoeffsOptimized assembly/LpfCoeffs/main.cpp)
target_compile_options(AssemblyLpfCoeffsOptimized PRIVATE -O3 -save-temps ${COMMON_ARCH_FLAGS})
# build/CMakeFiles/AssemblyLpfCoeffsOptimized.dir/assembly/LpfCoeffs/main.cpp.s


