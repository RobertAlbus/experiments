#include <chrono>
#include <cmath>
#include <tuple>
#include <array>
#include <vector>
#include <iostream>

std::tuple<float, float, float, float, float> calculateBiquadCoeffs(float cutoff, float fs, float Q) {
    const float omega = 2.0 * M_PI * cutoff / fs;
    const float alpha = std::sin(omega) / (2.0 * Q);
    const float cos_omega = std::cos(omega);

    float b0 = (1.0 - cos_omega) / 2.0;
    float b1 = 1.0 - cos_omega;
    float b2 = (1.0 - cos_omega) / 2.0;
    float a0 = 1.0 + alpha;
    float a1 = -2.0 * cos_omega;
    float a2 = 1.0 - alpha;

    b0 /= a0;
    b1 /= a0;
    b2 /= a0;
    a1 /= a0;
    a2 /= a0;

    return {b0, b1, b2, a1, a2};
}

int main() {
    printf("\n111");
    const int durationSamples = 100000;
    const int numFilters = 800;
    const int iterations = durationSamples * numFilters;
    float sampleRate = 48000;
    float audioDurationMs = ((float)durationSamples) / sampleRate * 1000;
    float cutoff = 3000;
    float Q = 1.414;

    // const size_t iterations_size_t = static_cast<size_t>(iterations);

    // std::vector<std::tuple<float, float, float, float, float>> coeffs;
    std::array<std::tuple<float, float, float, float, float>, 80000000> coeffs;
    // coeffs.reserve((size_t)iterations); // reserving the right amount of memory has a huge impact on performance.
    
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        printf("\n%i",i);
        coeffs[i] = calculateBiquadCoeffs(cutoff + (float)i, sampleRate, Q);
        // coeffs.emplace_back(calculateBiquadCoeffs(cutoff + (float)i, fs, Q));
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    printf("\n%i paralell biquad lpf filters | duration %i ms of %i ms audio | %04.2f%% time budget consumed",
        numFilters,
        duration,
        (int)audioDurationMs,
        ((float)duration) / audioDurationMs * 100.f
    );

    return 0;
}
