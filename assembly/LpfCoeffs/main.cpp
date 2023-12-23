#include <cmath>
#include <tuple>
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
    float cutoff = 3000;
    float fs = 48000;
    float Q = 1.414;

    auto [b0, b1, b2, a1, a2] = calculateBiquadCoeffs(cutoff, fs, Q);

    std::cout << "Biquad Filter Coefficients:" << std::endl;
    std::cout << "b0: " << b0 << ", b1: " << b1 << ", b2: " << b2 << std::endl;
    std::cout << "a1: " << a1 << ", a2: " << a2 << std::endl;

    return 0;
}
