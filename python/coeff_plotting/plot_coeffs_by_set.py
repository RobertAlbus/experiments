import numpy as np
import matplotlib.pyplot as plt

from lib.generate_coeffs import *

sample_rate = 48000
Q = 0.707
gain_db = -60

starting_freq_hz = 1
num_octaves = 14

cutoff_freqs_hz_linear = [0]
for i in range(0, num_octaves):
    final_freq_hz = starting_freq_hz * np.power(2, num_octaves - 1)
    step_size_hz = (final_freq_hz - starting_freq_hz) / (num_octaves - 1)
    cutoff_freqs_hz_linear.append(starting_freq_hz + (i * step_size_hz))

filter_coeffs_linear = {}
for freq in cutoff_freqs_hz_linear:
    a, b = biquad_resonant_lp_coeffs(freq, Q, gain_db, sample_rate)
    filter_coeffs_linear[str(freq)] = Coeffs(a, b)

fig, axs = plt.subplots(2)

a_coeff_plot_lin = axs[0]
a_coeff_plot_lin.set_title('A Coeffs Linear')

b_coeff_plot_lin = axs[1]
b_coeff_plot_lin.set_title('B Coeffs Linear')

for freq in filter_coeffs_linear.keys():
    a_coeff_plot_lin.plot(range(1,3), filter_coeffs_linear[freq].a, label=freq)
    b_coeff_plot_lin.plot(range(1,4), filter_coeffs_linear[freq].b, label=freq)

# a_coeff_plot_lin.legend()
# b_coeff_plot_lin.legend()

plt.tight_layout()
plt.show()

