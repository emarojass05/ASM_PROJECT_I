"""
transformadas.py
DFT y FFT propias (numpy.fft solo se usa para validar, no para calcular).
"""

import numpy as np


def dft_loop(x):
    """DFT directa con doble ciclo for. O(N^2)."""
    x = np.asarray(x, dtype=complex)
    N = len(x)
    X = np.zeros(N, dtype=complex)
    for k in range(N):
        suma = 0 + 0j
        for n in range(N):
            suma += x[n] * np.exp(-2j * np.pi * k * n / N)
        X[k] = suma
    return X


def dft_matrix(x):
    """DFT directa vectorizada (producto matriz-vector). O(N^2)."""
    x = np.asarray(x, dtype=complex)
    N = len(x)
    n = np.arange(N)
    k = n.reshape((N, 1))
    W = np.exp(-2j * np.pi * k * n / N)
    return np.sum(W * x, axis=1)  # evita bug de matmul complejo en macOS/Accelerate


def fft_radix2(x):
    """FFT Cooley-Tukey radix-2, recursiva. O(N log N). N debe ser potencia de 2."""
    x = np.asarray(x, dtype=complex)
    N = len(x)

    if N & (N - 1) != 0:
        raise ValueError("N debe ser potencia de 2 (usar rellenar_potencia_de_2)")

    if N <= 1:
        return x

    par = fft_radix2(x[0::2])
    impar = fft_radix2(x[1::2])

    factor = np.exp(-2j * np.pi * np.arange(N // 2) / N)
    mitad = factor * impar

    return np.concatenate([par + mitad, par - mitad])


def ifft_radix2(X):
    """FFT inversa: ifft(X) = conj(fft(conj(X))) / N."""
    X = np.asarray(X, dtype=complex)
    N = len(X)
    x = fft_radix2(np.conj(X))
    return np.conj(x) / N


def siguiente_potencia_de_2(n):
    """Menor potencia de 2 mayor o igual a n."""
    if n <= 1:
        return 1
    return 1 << (int(np.ceil(np.log2(n))))


def rellenar_potencia_de_2(x):
    """Zero-padding hasta la siguiente potencia de 2."""
    x = np.asarray(x, dtype=complex)
    N = len(x)
    N2 = siguiente_potencia_de_2(N)
    if N2 == N:
        return x
    return np.concatenate([x, np.zeros(N2 - N, dtype=complex)])


if __name__ == "__main__":
    # Validacion contra numpy.fft
    np.random.seed(0)
    x = np.random.randn(16) + 1j * np.random.randn(16)

    X_loop = dft_loop(x)
    X_mat = dft_matrix(x)
    X_fft = fft_radix2(x)
    X_ref = np.fft.fft(x)

    print("Validacion DFT/FFT propias vs numpy.fft (N=16, senal aleatoria)")
    print("  dft_loop   vs numpy.fft -> max error:", np.max(np.abs(X_loop - X_ref)))
    print("  dft_matrix vs numpy.fft -> max error:", np.max(np.abs(X_mat - X_ref)))
    print("  fft_radix2 vs numpy.fft -> max error:", np.max(np.abs(X_fft - X_ref)))

    x_rec = ifft_radix2(X_fft)
    print("  ifft_radix2(fft_radix2(x)) vs x -> max error:", np.max(np.abs(x_rec - x)))
