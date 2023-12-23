#include <chrono>
#include <cmath>
#include <tuple>
#include <array>
#include <vector>
#include <iostream>

#include <immintrin.h> // avx2 simd

/*

non-optimized: AVX2  6.54% faster
    optimized: AVX2 36.78% faster

    ????

if lerping coefficients sounds fine and is accurate, then this is an excellent optimization

*/

std::array<float, 5> calculateBiquadCoeffs(float cutoff, float fs, float Q) {
// std::tuple<float, float, float, float, float> calculateBiquadCoeffs(float cutoff, float fs, float Q) {
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

std::array<float, 5> lerpBiquadCoeffs(
    std::array<float, 5>& start,
    std::array<float, 5>& end,
    float amount
) {

    __m256 vec_start = _mm256_set_ps(
        start[0],
        start[1],
        start[2],
        start[3],
        start[4],
        0,0,0 // only need 5 slots
    );
    __m256 vec_end = _mm256_set_ps(
        end[0],
        end[1],
        end[2],
        end[3],
        end[4],
        0,0,0 // only need 5 slots
    );
    __m256 t = _mm256_set1_ps(amount);

    // Perform SIMD lerp for 8 data points
    __m256 result = _mm256_add_ps(vec_start, _mm256_mul_ps(t, _mm256_sub_ps(vec_end, vec_start)));

    float output[8];
    _mm256_storeu_ps(output, result);

    return {output[0], output[1], output[2], output[3], output[4]};
}

int main() {
    printf("\n----------------------------------------------------------------");
    const int durationSamples = 100000;
    const int numFilters = 800;
    const int iterations = durationSamples * numFilters;
    float sampleRate = 48000;
    float audioDurationMs = ((float)durationSamples) / sampleRate * 1000;
    float cutoff = 3000;
    float Q = 1.414;

    std::vector<std::array<float, 5>> coeffs;
    // std::vector<std::tuple<float, float, float, float, float>> coeffs;
    coeffs.reserve((size_t)iterations); // reserving the right amount of memory has a huge impact on performance.
    
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        coeffs.emplace_back(calculateBiquadCoeffs(cutoff + (float)i, sampleRate, Q));
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    float freqStart = 20.f;
    float freqEnd = 20000.f;
    float freqRange = freqEnd - freqStart;
    float freqRangeRecip = freqRange / 1;
    float cutoffAdjusted = cutoff - freqStart;
    float cutoffCompensation = freqStart * freqRangeRecip; // compensate for bottom freq not being 0
    auto startCoeffs = calculateBiquadCoeffs(freqStart, sampleRate, Q);
    auto endCoeffs = calculateBiquadCoeffs(freqEnd, sampleRate, Q);

    auto simd_start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        float lerpAmount = (cutoffAdjusted + i) * freqRangeRecip + cutoffCompensation;
        coeffs.emplace_back(lerpBiquadCoeffs(startCoeffs, endCoeffs, lerpAmount));
    }
    auto simd_end = std::chrono::high_resolution_clock::now();
    auto simd_duration = std::chrono::duration_cast<std::chrono::milliseconds>(simd_end - simd_start).count();

    printf("\n%i parallel biquad lpf filters | duration %i ms of %i ms audio | %04.2f%% time budget consumed [COEFF CALCULATION]",
        numFilters,
        duration,
        (int)audioDurationMs,
        ((float)duration) / audioDurationMs * 100.f
    );
    printf("\n%i parallel biquad lpf filters | duration %i ms of %i ms audio | %04.2f%% time budget consumed [AVX2 SIMD LERP]",
        numFilters,
        simd_duration,
        (int)audioDurationMs,
        ((float)simd_duration) / audioDurationMs * 100.f
    );
    printf("\n");

    return 0;
}
