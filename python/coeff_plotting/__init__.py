from collections import namedtuple

import numpy as np
import matplotlib.pyplot as plt

Coeffs = namedtuple('Coeffs', ['a', 'b'])
def lp_coeffs(cutoff_hz, Q, gain_db, fs=48000):
    # Convert gain to linear scale
    A = 10 ** (gain_db / 40)

    omega = 2 * np.pi * cutoff_hz / fs
    alpha = np.sin(omega) / (2 * Q)

    b0 = (1 - np.cos(omega)) / 2
    b1 = 1 - np.cos(omega)
    b2 = (1 - np.cos(omega)) / 2
    a0 = 1 + alpha
    a1 = -2 * np.cos(omega)
    a2 = 1 - alpha

    # Normalize the coefficients
    b0 /= a0
    b1 /= a0
    b2 /= a0
    a1 /= a0
    a2 /= a0

    return (a1, a2), (b0, b1, b2)

sample_rate = 48000
Q = 0.707
gain_db = -60

starting_freq_hz = 50
num_octaves = 6

cutoff_freqs_hz_exponential = [0]
for octave in range(0, num_octaves):
    cutoff_freqs_hz_exponential.append(starting_freq_hz * np.power(2, octave))
    # print(starting_freq_hz * np.power(2, octave))
    
cutoff_freqs_hz_linear = [0]
for i in range(0, num_octaves):
    final_freq_hz = starting_freq_hz * np.power(2, num_octaves - 1)
    step_size_hz = (final_freq_hz - starting_freq_hz) / (num_octaves - 1)
    cutoff_freqs_hz_linear.append(starting_freq_hz + (i * step_size_hz))
    # print(starting_freq_hz + (i * step_size_hz))

filter_coeffs_exponential = {}
for freq in cutoff_freqs_hz_exponential:
    a, b = lp_coeffs(freq, Q, gain_db, sample_rate)
    filter_coeffs_exponential[str(freq)] = Coeffs(a, b)
    
filter_coeffs_linear = {}
for freq in cutoff_freqs_hz_linear:
    a, b = lp_coeffs(freq, Q, gain_db, sample_rate)
    filter_coeffs_linear[str(freq)] = Coeffs(a, b)

fig, axs = plt.subplots(2, 2)

a_coeff_plot_exp = axs[0, 0]
a_coeff_plot_exp.set_title('A Coeffs Exponential')

b_coeff_plot_exp = axs[1, 0]
b_coeff_plot_exp.set_title('B Coeffs Exponential')

a_coeff_plot_lin = axs[0, 1]
a_coeff_plot_lin.set_title('A Coeffs Linear')

b_coeff_plot_lin = axs[1, 1]
b_coeff_plot_lin.set_title('B Coeffs Linear')

for freq in filter_coeffs_exponential.keys():
    a_coeff_plot_exp.plot(range(1,3), filter_coeffs_exponential[freq].a, label=freq)
    b_coeff_plot_exp.plot(range(1,4), filter_coeffs_exponential[freq].b, label=freq)

for freq in filter_coeffs_linear.keys():
    a_coeff_plot_lin.plot(range(1,3), filter_coeffs_linear[freq].a, label=freq)
    b_coeff_plot_lin.plot(range(1,4), filter_coeffs_linear[freq].b, label=freq)

a_coeff_plot_exp.legend()
b_coeff_plot_exp.legend()
a_coeff_plot_lin.legend()
b_coeff_plot_lin.legend()

plt.tight_layout()
plt.show()

