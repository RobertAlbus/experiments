import numpy as np
import matplotlib.pyplot as plt

from lib.generate_coeffs import *

sample_rate = 48000
gain_db = -60

Qs = [0.15, 0.707, 1.414, 2.828, 5.656]

filter_coeffs_linear = {}
for freq in range(1, 24001):
    filter_coeffs_linear[str(freq)] = {}
    for Q in Qs:
        a, b = biquad_resonant_lp_coeffs(freq, Q, gain_db, sample_rate)
        filter_coeffs_linear[str(freq)][str(Q)] = Coeffs(a, b)
coeffs = {}
for Q in Qs:
    coeffs[str(Q)] = {
        "a1": [],
        "a2": [],
        "b0": [],
        "b1": [],
        "b2": [],
    }

for freq_key, coefficient_set in filter_coeffs_linear.items():
    for Q, coefficients in coefficient_set.items():
        coeffs[Q]["a1"].append(coefficients.a[0])
        coeffs[Q]["a2"].append(coefficients.a[1])
        coeffs[Q]["b0"].append(coefficients.b[0])
        coeffs[Q]["b1"].append(coefficients.b[1])
        coeffs[Q]["b2"].append(coefficients.b[2])


fig, axs = plt.subplots(len(Qs), 5)

for i, Q in enumerate(Qs):
    axs[i, 0].plot(list(map(lambda x: -3 + ((x/24000) * 3), range(1, 24001))), coeffs[str(Q)]["a1"])
    axs[i, 0].set_title("a1")
    axs[i, 1].plot(list(map(lambda x: ((x/24000) * 2), range(1, 24001))), coeffs[str(Q)]["a2"])
    axs[i, 1].set_title("a2")
    axs[i, 2].plot(list(map(lambda x: (x/24000), range(1, 24001))), coeffs[str(Q)]["b0"])
    axs[i, 2].set_title("b0")
    axs[i, 3].plot(list(map(lambda x: (x/24000), range(1, 24001))), coeffs[str(Q)]["b1"])
    axs[i, 3].set_title("b1")
    axs[i, 4].plot(list(map(lambda x: (x/24000), range(1, 24001))), coeffs[str(Q)]["b2"])
    axs[i, 4].set_title("b2")


# a_coeff_plot_lin.legend()
# b_coeff_plot_lin.legend()

plt.tight_layout()
plt.show()

