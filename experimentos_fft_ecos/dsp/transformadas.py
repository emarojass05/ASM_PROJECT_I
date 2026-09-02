"""
transformadas.py
-----------------
Implementaciones propias (no se usa numpy.fft para el calculo, solo para
validar) de la Transformada Discreta de Fourier (DFT) y la Transformada
Rapida de Fourier (FFT, algoritmo Cooley-Tukey radix-2).

Estas funciones son utilizadas por el resto de los scripts de la semana 5
(experimentos de FFT y experimentos de deteccion de ecos) del proyecto de
radar acustico, curso CE1110 - Analisis de Senales Mixtas, TEC.

Autor: (completar con nombre del estudiante)
"""

import numpy as np


def dft_loop(x):
    """DFT directa (definicion), implementada con doble ciclo for.

    X[k] = sum_{n=0}^{N-1} x[n] * exp(-j*2*pi*k*n/N)

    Complejidad: O(N^2). Se deja explicita (sin vectorizar) para poder
    comparar el crecimiento del tiempo de ejecucion contra la FFT.
    Uso recomendado solo para N pequenos (<= 512) por su costo.
    """
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
    """DFT directa, misma definicion que dft_loop pero expresada como
    producto matriz-vector (W_N * x). Sigue siendo O(N^2) en numero de
    operaciones, pero al estar vectorizada con numpy es mucho mas rapida
    en la practica. Se usa para poder medir tiempos con N mas grandes
    sin esperar minutos por el ciclo puro de Python.
    """
    x = np.asarray(x, dtype=complex)
    N = len(x)
    n = np.arange(N)
    k = n.reshape((N, 1))
    W = np.exp(-2j * np.pi * k * n / N)  # matriz de N x N
    return W @ x


def fft_radix2(x):
    """FFT propia con el algoritmo Cooley-Tukey, decimacion en tiempo,
    radix-2 (divide y venceras). Requiere que len(x) sea potencia de 2.

    Complejidad: O(N log N).
    """
    x = np.asarray(x, dtype=complex)
    N = len(x)

    if N & (N - 1) != 0:
        raise ValueError("fft_radix2 requiere que N sea potencia de 2 "
                          "(use rellenar_potencia_de_2 antes de llamarla)")

    if N <= 1:
        return x

    par = fft_radix2(x[0::2])
    impar = fft_radix2(x[1::2])

    factor = np.exp(-2j * np.pi * np.arange(N // 2) / N)
    mitad = factor * impar

    return np.concatenate([par + mitad, par - mitad])


def ifft_radix2(X):
    """FFT inversa propia, reutilizando fft_radix2 mediante la propiedad:

        ifft(X) = conj( fft( conj(X) ) ) / N

    Esto evita reimplementar el algoritmo completo para la version
    inversa y mantiene una sola fuente de verdad para la mariposa FFT.
    """
    X = np.asarray(X, dtype=complex)
    N = len(X)
    x = fft_radix2(np.conj(X))
    return np.conj(x) / N


def siguiente_potencia_de_2(n):
    """Retorna la menor potencia de 2 mayor o igual a n."""
    if n <= 1:
        return 1
    return 1 << (int(np.ceil(np.log2(n))))


def rellenar_potencia_de_2(x):
    """Rellena con ceros (zero-padding) al final de x hasta que su
    longitud sea una potencia de 2, requisito del radix-2."""
    x = np.asarray(x, dtype=complex)
    N = len(x)
    N2 = siguiente_potencia_de_2(N)
    if N2 == N:
        return x
    return np.concatenate([x, np.zeros(N2 - N, dtype=complex)])


if __name__ == "__main__":
    # Validacion rapida: comparar contra numpy.fft (referencia de la
    # industria) solo para confirmar que la implementacion propia es
    # correcta antes de usarla en el resto de los experimentos.
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
