"""
ecos.py

Funciones para generar una señal acústica conocida
y simular ecos con retardos controlados.
"""

import numpy as np
import matplotlib.pyplot as plt

def generar_chirp(fs, duracion, f_inicial, f_final):
    """Genera un chirp lineal entre dos frecuencias."""
    N = int(fs * duracion)
    t = np.arange(N) / fs

    pendiente = (f_final - f_inicial) / duracion

    fase = 2 * np.pi * ( f_inicial * t + 0.5 * pendiente * t**2)

    return np.cos(fase)


def generar_eco(x, retardo, amplitud=0.6):
    """Genera una copia retardada y atenuada de una señal."""

    y = np.zeros(len(x) + retardo)

    y[retardo:retardo + len(x)] = amplitud * x


    return y


def main():
    fs = 48000

    x = generar_chirp(
        fs=fs,
        duracion=0.005,
        f_inicial=2000,
        f_final=8000
    )

    retardo = 480

    y = generar_eco(
        x,
        retardo=retardo,
        amplitud=0.6
    )

    t_x = np.arange(len(x)) / fs
    t_y = np.arange(len(y)) / fs

    plt.plot(t_y, y, label="Eco")
    plt.plot(t_x, x, label="Señal transmitida")

    plt.xlabel("Tiempo (s)")
    plt.ylabel("Amplitud")
    plt.title("Señal transmitida y eco simulado")
    plt.legend()
    plt.grid(True)

    plt.show()


if __name__ == "__main__":
    main()