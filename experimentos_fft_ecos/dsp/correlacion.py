"""
correlacion.py

Generación de señales simuladas y métodos de correlación
para detección de ecos.
"""

import numpy as np

from transformadas import (fft_radix2, ifft_radix2, siguiente_potencia_de_2)

def correlacion_directa(x, y):
    """Calcula la correlación cruzada directamente."""
    nx = len(x)
    ny = len(y)

    resultado = np.zeros(ny - nx + 1)

    for k in range(len(resultado)):
        suma = 0.0

        for n in range(nx):
            suma += x[n] * y[n + k]

        resultado[k] = suma

    return resultado

def correlacion_fft(x, y):
    """Calcula la correlación cruzada mediante FFT."""
    longitud = len(x) + len(y) - 1
    n_fft = siguiente_potencia_de_2(longitud)

    x_pad = np.zeros(n_fft, dtype=complex)
    y_pad = np.zeros(n_fft, dtype=complex)

    x_pad[:len(x)] = x
    y_pad[:len(y)] = y

    X = fft_radix2(x_pad)
    Y = fft_radix2(y_pad)

    R = np.conj(X) * Y

    correlacion = ifft_radix2(R)

    return np.real(correlacion[:len(y) - len(x) + 1])