"""
comparacion_tiempos.py
Compara el tiempo de ejecucion de DFT (dft_matrix) vs FFT (fft_radix2)
para distintos N. Genera figuras/tiempos_dft_fft.png y .csv
"""

import os
import time
import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

import sys
sys.path.append(os.path.join(os.path.dirname(__file__), "..", "dsp"))
from transformadas import dft_loop, dft_matrix, fft_radix2  # noqa: E402

FIG_DIR = os.path.join(os.path.dirname(__file__), "figuras")
os.makedirs(FIG_DIR, exist_ok=True)


def medir_tiempo(func, x, repeticiones=5):
    """Tiempo minimo de varias repeticiones (reduce ruido de medicion)."""
    tiempos = []
    for _ in range(repeticiones):
        t0 = time.perf_counter()
        func(x)
        t1 = time.perf_counter()
        tiempos.append(t1 - t0)
    return min(tiempos)


def main():
    exponentes = list(range(4, 13))       # N = 16 .. 4096
    exponentes_loop = list(range(4, 10))  # N = 16 .. 512 (dft_loop es lento)

    Ns = [2 ** e for e in exponentes]
    Ns_loop = [2 ** e for e in exponentes_loop]

    t_dft_matrix = []
    t_fft = []
    t_dft_loop = []

    np.random.seed(1)

    print("Midiendo dft_matrix y fft_radix2 ...")
    for N in Ns:
        x = np.random.randn(N)
        t_dft_matrix.append(medir_tiempo(dft_matrix, x))
        t_fft.append(medir_tiempo(fft_radix2, x))
        print(f"  N={N:5d}  dft_matrix={t_dft_matrix[-1]*1e3:9.3f} ms  "
              f"fft_radix2={t_fft[-1]*1e3:9.3f} ms")

    print("Midiendo dft_loop (N pequenos) ...")
    for N in Ns_loop:
        x = np.random.randn(N)
        t_dft_loop.append(medir_tiempo(dft_loop, x, repeticiones=2))
        print(f"  N={N:5d}  dft_loop={t_dft_loop[-1]*1e3:9.3f} ms")

    csv_path = os.path.join(FIG_DIR, "tiempos_dft_fft.csv")
    with open(csv_path, "w") as f:
        f.write("N,dft_matrix_ms,fft_radix2_ms\n")
        for N, td, tf in zip(Ns, t_dft_matrix, t_fft):
            f.write(f"{N},{td*1e3:.6f},{tf*1e3:.6f}\n")
    print("Tabla guardada en", csv_path)

    fig, ax = plt.subplots(1, 2, figsize=(11, 4.5))

    ax[0].plot(Ns, np.array(t_dft_matrix) * 1e3, "o-", label="DFT (dft_matrix), O(N^2)")
    ax[0].plot(Ns, np.array(t_fft) * 1e3, "s-", label="FFT propia (radix-2), O(N log N)")
    ax[0].set_xlabel("N (muestras)")
    ax[0].set_ylabel("Tiempo de ejecucion (ms)")
    ax[0].set_title("Tiempo de ejecucion vs N")
    ax[0].legend()
    ax[0].grid(True, alpha=0.3)

    ax[1].loglog(Ns, np.array(t_dft_matrix) * 1e3, "o-", label="DFT, O(N^2)")
    ax[1].loglog(Ns, np.array(t_fft) * 1e3, "s-", label="FFT, O(N log N)")
    ax[1].set_xlabel("N (muestras, escala log)")
    ax[1].set_ylabel("Tiempo de ejecucion (ms, escala log)")
    ax[1].set_title("Mismo resultado en escala log-log")
    ax[1].legend()
    ax[1].grid(True, which="both", alpha=0.3)

    fig.suptitle("Comparacion de tiempos: DFT directa vs FFT (Cooley-Tukey radix-2)")
    fig.tight_layout()

    out_path = os.path.join(FIG_DIR, "tiempos_dft_fft.png")
    fig.savefig(out_path, dpi=150)
    print("Figura guardada en", out_path)


if __name__ == "__main__":
    main()
