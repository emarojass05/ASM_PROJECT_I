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

def detectar_picos(
    correlacion,
    umbral_relativo=0.25,
    distancia_minima=1):
    
    """Encuentra los principales picos de una correlación."""
    valores = np.abs(correlacion)

    if len(valores) == 0:
        return []

    umbral = np.max(valores) * umbral_relativo
    candidatos = []

    for i in range(len(valores)):

        if valores[i] < umbral:
            continue

        # Si estamos en el primer elemento,
        # no existe vecino izquierdo.
        max_izquierda = (
            i == 0
            or valores[i] > valores[i - 1]
        )

        # Si estamos en el último elemento,
        # no existe vecino derecho.
        max_derecha = (
            i == len(valores) - 1
            or valores[i] >= valores[i + 1]
        )

        if max_izquierda and max_derecha:
            candidatos.append(i)

    # Ordenar los candidatos desde el pico más fuerte
    # hasta el más débil.
    candidatos.sort(
        key=lambda i: valores[i],
        reverse=True
    )

    picos = []

    # Evitar detectar varios máximos pertenecientes
    # al mismo eco.
    for candidato in candidatos:
        if all(
            abs(candidato - pico) >= distancia_minima
            for pico in picos
        ):
            picos.append(candidato)

    return sorted(picos)