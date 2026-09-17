"""
comparacion_correlacion.py

Compara el tiempo de ejecución de la correlación directa
contra la correlación calculada mediante FFT.
"""

import os
import sys
import time

import matplotlib.pyplot as plt
import numpy as np


sys.path.append(
    os.path.join(
        os.path.dirname(__file__),
        "..",
        "dsp"
    )
)

from ecos import generar_chirp, generar_ecos
from correlacion import correlacion_directa, correlacion_fft


FIG_DIR = os.path.join(
    os.path.dirname(__file__),
    "figuras"
)

os.makedirs(FIG_DIR, exist_ok=True)


def medir_tiempo(func, x, y, repeticiones=3):
    """Mide el menor tiempo de varias ejecuciones."""
    tiempos = []

    for _ in range(repeticiones):
        t0 = time.perf_counter()
        func(x, y)
        t1 = time.perf_counter()

        tiempos.append(t1 - t0)

    return min(tiempos)


def main():
    fs = 48000

    tamanos = [
        64,
        128,
        256,
        512,
        1024
    ]

    tiempos_directa = []
    tiempos_fft = []

    print("Comparación de correlación directa vs FFT\n")

    for n in tamanos:
        duracion = n / fs

        x = generar_chirp(
            fs=fs,
            duracion=duracion,
            f_inicial=2000,
            f_final=8000
        )

        ecos = [
            (n, 0.6)
        ]

        y = generar_ecos(x, ecos)

        t_directa = medir_tiempo(
            correlacion_directa,
            x,
            y
        )

        t_fft = medir_tiempo(
            correlacion_fft,
            x,
            y
        )

        tiempos_directa.append(t_directa)
        tiempos_fft.append(t_fft)

        print(
            f"N={n:4d} | "
            f"Directa={t_directa * 1e3:9.3f} ms | "
            f"FFT={t_fft * 1e3:9.3f} ms"
        )

    # Guardar resultados
    csv_path = os.path.join(
        FIG_DIR,
        "tiempos_correlacion.csv"
    )

    with open(csv_path, "w") as archivo:
        archivo.write(
            "N,correlacion_directa_ms,correlacion_fft_ms\n"
        )

        for n, td, tf in zip(
            tamanos,
            tiempos_directa,
            tiempos_fft
        ):
            archivo.write(
                f"{n},{td * 1e3:.6f},{tf * 1e3:.6f}\n"
            )

    # Gráfica
    plt.figure(figsize=(8, 5))

    plt.plot(
        tamanos,
        np.array(tiempos_directa) * 1e3,
        "o-",
        label="Correlación directa"
    )

    plt.plot(
        tamanos,
        np.array(tiempos_fft) * 1e3,
        "s-",
        label="Correlación mediante FFT"
    )

    plt.xlabel("N (muestras)")
    plt.ylabel("Tiempo de ejecución (ms)")
    plt.title(
        "Comparación de correlación directa vs FFT"
    )
    plt.grid(True)
    plt.legend()
    plt.tight_layout()

    out_path = os.path.join(
        FIG_DIR,
        "tiempos_correlacion.png"
    )

    plt.savefig(out_path, dpi=150)
    plt.close()

    print("\nTabla guardada en:", csv_path)
    print("Figura guardada en:", out_path)


if __name__ == "__main__":
    main()