import numpy as np
import matplotlib.pyplot as plt

from lib.generate_coeffs import *

sample_rate = 48000
Q = 0.707
gain_db = -60


filter_coeffs_linear = {}
for freq in range(1, 24001):
    a, b = biquad_resonant_lp_coeffs(freq, Q, gain_db, sample_rate)
    filter_coeffs_linear[str(freq)] = Coeffs(a, b)

coeffs = {
    "a1": [],
    "a2": [],
    "b0": [],
    "b1": [],
    "b2": [],
}
for freq_key, coefficient_set in filter_coeffs_linear.items():
    coeffs["a1"].append(coefficient_set.a[0])
    coeffs["a2"].append(coefficient_set.a[1])
    coeffs["b0"].append(coefficient_set.b[0])
    coeffs["b1"].append(coefficient_set.b[1])
    coeffs["b2"].append(coefficient_set.b[2])


fig, axs = plt.subplots(1,5)


axs[0].plot(list(map(lambda x: -3 + ((x/24000) * 3), range(1, 24001))), coeffs["a1"])
axs[0].set_title("a1")

axs[1].plot(list(map(lambda x: ((x/24000) * 2), range(1, 24001))), coeffs["a2"])
axs[1].set_title("a2")

axs[2].plot(list(map(lambda x: (x/24000), range(1, 24001))), coeffs["b0"])
axs[2].set_title("b0")

axs[3].plot(list(map(lambda x: (x/24000), range(1, 24001))), coeffs["b1"])
axs[3].set_title("b1")

axs[4].plot(list(map(lambda x: (x/24000), range(1, 24001))), coeffs["b2"])
axs[4].set_title("b2")


# a_coeff_plot_lin.legend()
# b_coeff_plot_lin.legend()

plt.tight_layout()
plt.show()

