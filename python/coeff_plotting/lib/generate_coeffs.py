from collections import namedtuple

import numpy as np


Coeffs = namedtuple('Coeffs', ['a', 'b'])

def biquad_resonant_lp_coeffs(cutoff_hz, Q, gain_db, fs=48000):
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