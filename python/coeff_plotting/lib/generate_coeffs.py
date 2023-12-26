from collections import namedtuple

import numpy as np


Coeffs = namedtuple('Coeffs', ['a', 'b'])


def biquad_resonant_lp_coeffs(cutoff_hz, Q, fs=48000):
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


def biquad_resonant_hp_coeffs(cutoff_hz, Q, fs=48000):
    omega = 2 * np.pi * cutoff_hz / fs
    alpha = np.sin(omega) / (2 * Q)

    b0 = (1 + np.cos(omega)) / 2
    b1 = -(1 + np.cos(omega))
    b2 = (1 + np.cos(omega)) / 2
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


def biquad_resonant_bp_coeffs(cutoff_hz, Q, fs=48000):
    omega = 2 * np.pi * cutoff_hz / fs
    alpha = np.sin(omega) / (2 * Q)

    b0 = np.sin(omega) / 2
    b1 = 0
    b2 = -(np.sin(omega) / 2)
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


def biquad_flat_bp_coeffs(cutoff_hz, Q, fs=48000):
    omega = 2 * np.pi * cutoff_hz / fs
    alpha = np.sin(omega) / (2 * Q)

    b0 = alpha
    b1 = 0
    b2 = -alpha
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


def biquad_notch_coeffs(cutoff_hz, Q, fs=48000):
    omega = 2 * np.pi * cutoff_hz / fs
    alpha = np.sin(omega) / (2 * Q)

    b0 = 1
    b1 = -2 * np.cos(omega)
    b2 = 1
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


def biquad_ap_coeffs(cutoff_hz, Q, fs=48000):
    omega = 2 * np.pi * cutoff_hz / fs
    alpha = np.sin(omega) / (2 * Q)

    b0 = 1 - alpha
    b1 = -2 * np.cos(omega)
    b2 = 1 + alpha
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


def biquad_peak_coeffs(cutoff_hz, Q, gain_db, fs=48000):
    A = 10 ** (gain_db / 40)
    omega = 2 * np.pi * cutoff_hz / fs
    alpha = np.sin(omega) / (2 * Q)

    b0 = 1 + alpha * omega
    b1 = -2 * np.cos(omega)
    b2 = 1 - alpha * omega
    a0 = 1 + alpha / A
    a1 = -2 * np.cos(omega)
    a2 = 1 - alpha / A

    # Normalize the coefficients
    b0 /= a0
    b1 /= a0
    b2 /= a0
    a1 /= a0
    a2 /= a0

    return (a1, a2), (b0, b1, b2)


def biquad_low_shelf_coeffs(cutoff_hz, Q, gain_db, fs=48000):
    A = 10 ** (gain_db / 40)
    omega = 2 * np.pi * cutoff_hz / fs
    alpha = np.sin(omega) / (2 * Q)

    b0 = A * ((A+1) - (A-1) * np.cos(omega) + 2 * np.sqrt(A) * alpha)
    b1 = 2 * A * ((A-1) - (A+1) * np.cos(omega))
    b2 = A * ((A+1) - (A-1) * np.cos(omega) - 2 * np.sqrt(A) * alpha)
    a0 = (A+1) + (A-1) * np.cos(omega) + 2 * np.sqrt(A) * alpha
    a1 = -2 * ((A-1) + (A+1) * np.cos(omega))
    a2 = (A+1) + (A-1) * np.cos(omega) - 2 * np.sqrt(A) * alpha

    # Normalize the coefficients
    b0 /= a0
    b1 /= a0
    b2 /= a0
    a1 /= a0
    a2 /= a0

    return (a1, a2), (b0, b1, b2)


def biquad_high_shelf_coeffs(cutoff_hz, Q, gain_db, fs=48000):
    A = 10 ** (gain_db / 40)
    omega = 2 * np.pi * cutoff_hz / fs
    alpha = np.sin(omega) / (2 * Q)

    b0 = A * ((A+1) + (A-1) * np.cos(omega) + 2 * np.sqrt(A) * alpha)
    b1 = -2 * A * ((A-1) + (A+1) * np.cos(omega))
    b2 = A * ((A+1) + (A-1) * np.cos(omega) - 2 * np.sqrt(A) * alpha)
    a0 = (A+1) - (A-1) * np.cos(omega) + 2 * np.sqrt(A) * alpha
    a1 = 2 * ((A-1) - (A+1) * np.cos(omega))
    a2 = (A+1) - (A-1) * np.cos(omega) - 2 * np.sqrt(A) * alpha

    # Normalize the coefficients
    b0 /= a0
    b1 /= a0
    b2 /= a0
    a1 /= a0
    a2 /= a0

    return (a1, a2), (b0, b1, b2)