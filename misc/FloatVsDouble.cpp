/*
 * /////////
 * // cpp-experiments
 *
 * Copyright (C) 2023 Rob W. Albus
 * All rights reserved.
 *
 */

#include <chrono>
#include <iostream>

int main(int argc, char *argv[]) {
  const int iterations = 1000000;

    // Measure float multiplication
    auto start = std::chrono::high_resolution_clock::now();
    float f = 1.0f;
    for (int i = 0; i < iterations; ++i) {
        f *= 1.000001f;
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto float_duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "Time taken for float multiplication: " << float_duration << " microseconds\n";

    // Measure double multiplication
    start = std::chrono::high_resolution_clock::now();
    double d = 1.0;
    for (int i = 0; i < iterations; ++i) {
        d *= 1.000001f;
    }
    end = std::chrono::high_resolution_clock::now();
    auto double_duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "Time taken for double multiplication: " << double_duration << " microseconds\n";

    // Measure long double multiplication
    start = std::chrono::high_resolution_clock::now();
    long double ld = 1.0L;
    for (int i = 0; i < iterations; ++i) {
        ld *= 1.000001f;
    }
    end = std::chrono::high_resolution_clock::now();
    auto long_double_duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "Time taken for long double multiplication: " << long_double_duration << " microseconds\n";

    return 0;
  
}
