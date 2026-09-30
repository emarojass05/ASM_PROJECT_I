"""
ecos.py

Funciones para generar una señal acústica conocida
y simular ecos con retardos controlados.
"""

import os
import numpy as np
import matplotlib.pyplot as plt
import os

FIG_DIR = os.path.join(os.path.dirname(__file__), "figuras")

FIG_DIR = os.path.join(os.path.dirname(__file__), "figuras")


def generar_chirp(fs, duracion, f_inicial, f_final):
    """Genera un chirp lineal entre dos frecuencias."""
    N = int(fs * duracion)
    t = np.arange(N) / fs

    pendiente = (f_final - f_inicial) / duracion

    fase = 2 * np.pi * ( f_inicial * t + 0.5 * pendiente * t**2)

    return np.cos(fase)


def generar_ecos(x, ecos):
    """Genera varios ecos retardados y atenuados. """

    max_retardo = max(retardo for retardo, _ in ecos)

    y = np.zeros(len(x) + max_retardo)

    for retardo, amplitud in ecos:
        inicio = retardo
        fin = retardo + len(x)

        y[inicio:fin] += amplitud * x

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

    os.makedirs(FIG_DIR, exist_ok=True)
    out_path = os.path.join(FIG_DIR, "senal_tx_eco.png")
    plt.savefig(out_path, dpi=150)
    print("Figura guardada en", out_path)

    # Si no hay pantalla disponible (por ejemplo, corriendo por SSH o en un
    # servidor de correccion automatica), plt.show() puede fallar o quedar
    # esperando; se ignora el error para que el script no se cuelgue.
    try:
        plt.show()
    except Exception:
        pass


if __name__ == "__main__":
    main()
