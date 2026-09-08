import os
import time
import numpy as np
import matplotlib.pyplot as plt
import sys

sys.path.append(os.path.join(os.path.dirname(__file__), "..", "dsp"))

from ecos import generar_chirp, generar_eco
from correlacion import correlacion_directa, correlacion_fft


FIG_DIR = os.path.join(os.path.dirname(__file__), "figuras")
os.makedirs(FIG_DIR, exist_ok=True)


def agregar_ruido(senal, snr_db):
    """Agrega ruido gaussiano para lograr un SNR objetivo en dB."""
    potencia_senal = np.mean(senal ** 2)
    potencia_ruido = potencia_senal / (10 ** (snr_db / 10))
    ruido = np.sqrt(potencia_ruido) * np.random.randn(len(senal))
    return senal + ruido


def medir_tiempo(func, *args, repeticiones=5):
    """Mide el tiempo mínimo de ejecución de una función."""
    tiempos = []

    for _ in range(repeticiones):
        t0 = time.perf_counter()
        func(*args)
        t1 = time.perf_counter()
        tiempos.append(t1 - t0)

    return min(tiempos)


def experimento(fs, x, retardo_real, snr_db, etiqueta):
    """Ejecuta un experimento de detección de eco con ruido."""
    y = generar_eco(x, retardo=retardo_real, amplitud=0.6)

    if snr_db is not None:
        y = agregar_ruido(y, snr_db)

    r_directa = correlacion_directa(x, y)
    r_fft = correlacion_fft(x, y)

    t_directa = medir_tiempo(correlacion_directa, x, y)
    t_fft = medir_tiempo(correlacion_fft, x, y)

    retardo_directa = np.argmax(r_directa)
    retardo_fft = np.argmax(r_fft)

    velocidad_sonido = 343
    tiempo_vuelo = retardo_fft / fs
    distancia = velocidad_sonido * tiempo_vuelo / 2

    print(f"\n--- {etiqueta} ---")
    print("Retardo real:", retardo_real)
    print("Retardo directa:", retardo_directa, f"({t_directa:.6f} s)")
    print("Retardo FFT:", retardo_fft, f"({t_fft:.6f} s)")
    print("Distancia estimada:", round(distancia, 3), "m")

    t_x = np.arange(len(x)) / fs
    t_y = np.arange(len(y)) / fs
    t_r = np.arange(len(r_fft)) / fs

    fig, axs = plt.subplots(4, 1, figsize=(9, 8))

    axs[0].plot(t_x, x)
    axs[0].set_title("Señal transmitida (chirp)")
    axs[0].set_xlabel("Tiempo (s)")
    axs[0].set_ylabel("Amplitud")
    axs[0].grid(True)

    axs[1].plot(t_y, y)
    titulo_ruido = "Señal recibida"
    if snr_db is not None:
        titulo_ruido += f" (SNR = {snr_db} dB)"
    axs[1].set_title(titulo_ruido)
    axs[1].set_xlabel("Tiempo (s)")
    axs[1].set_ylabel("Amplitud")
    axs[1].grid(True)

    axs[2].plot(t_r, r_directa)
    axs[2].axvline(retardo_real / fs, color="r", linestyle="--", label="retardo real")
    axs[2].axvline(retardo_directa / fs, color="g", linestyle=":", label="pico detectado")
    axs[2].set_title("Correlación directa")
    axs[2].set_xlabel("Tiempo (s)")
    axs[2].set_ylabel("Correlación")
    axs[2].legend()
    axs[2].grid(True)

    axs[3].plot(t_r, r_fft)
    axs[3].axvline(retardo_real / fs, color="r", linestyle="--", label="retardo real")
    axs[3].axvline(retardo_fft / fs, color="g", linestyle=":", label="pico detectado")
    axs[3].set_title("Correlación mediante FFT")
    axs[3].set_xlabel("Tiempo (s)")
    axs[3].set_ylabel("Correlación")
    axs[3].legend()
    axs[3].grid(True)

    plt.tight_layout()
    plt.savefig(os.path.join(FIG_DIR, f"experimento_{etiqueta}.png"), dpi=150)
    plt.close(fig)

    return t_directa, t_fft, retardo_directa, retardo_fft


def main():
    np.random.seed(0)

    fs = 48000
    x = generar_chirp(fs=fs, duracion=0.005, f_inicial=2000, f_final=8000)
    retardo_real = 480

    experimento(fs, x, retardo_real, snr_db=None, etiqueta="ideal")

    for snr in [20, 10, 5, 0, -5]:
        experimento(fs, x, retardo_real, snr_db=snr, etiqueta=f"snr_{snr}")


if __name__ == "__main__":
    main()