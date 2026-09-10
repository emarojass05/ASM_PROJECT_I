import os
import time
import numpy as np
import matplotlib.pyplot as plt
import sys

sys.path.append(os.path.join(os.path.dirname(__file__), "..", "dsp"))

from ecos import generar_chirp, generar_ecos
from correlacion import correlacion_directa, correlacion_fft, detectar_picos


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


def experimento(fs, x, ecos, snr_db, etiqueta):
    """Ejecuta un experimento de detección de ecos."""

    y = generar_ecos(x, ecos)

    if snr_db is not None:
        y = agregar_ruido(y, snr_db)

    # Correlaciones
    r_directa = correlacion_directa(x, y)
    r_fft = correlacion_fft(x, y)

    # Tiempos de ejecución
    t_directa = medir_tiempo(correlacion_directa, x, y)
    t_fft = medir_tiempo(correlacion_fft, x, y)

    # Pico principal detectado
    # Detección de múltiples ecos
    retardos_directa = detectar_picos(
        r_directa,
        umbral_relativo=0.25,
        distancia_minima=len(x)
    )

    retardos_fft = detectar_picos(
        r_fft,
        umbral_relativo=0.25,
        distancia_minima=len(x)
    )

    # Retardos utilizados en la simulación
    retardos_reales = [
        retardo for retardo, _ in ecos
    ]

    # Distancias detectadas mediante FFT
    velocidad_sonido = 343
    distancias = []

    for retardo in retardos_fft:
        tiempo_vuelo = retardo / fs
        distancia = velocidad_sonido * tiempo_vuelo / 2
        distancias.append(distancia)

    print(f"\n--- {etiqueta} ---")

    print("Retardos reales:", retardos_reales)

    print(
        "Retardos directa:",
        retardos_directa,
        f"({t_directa:.6f} s)"
    )

    print(
        "Retardos FFT:",
        retardos_fft,
        f"({t_fft:.6f} s)"
    )

    print("Distancias estimadas:")

    for retardo, distancia in zip(retardos_fft, distancias):
        print(
            f"  {retardo} muestras -> "
            f"{distancia:.3f} m"
        )

    # Ejes temporales
    t_x = np.arange(len(x)) / fs
    t_y = np.arange(len(y)) / fs
    t_r = np.arange(len(r_fft)) / fs

    fig, axs = plt.subplots(4, 1, figsize=(9, 8))

    # Señal transmitida
    axs[0].plot(t_x, x)
    axs[0].set_title("Señal transmitida (chirp)")
    axs[0].set_xlabel("Tiempo (s)")
    axs[0].set_ylabel("Amplitud")
    axs[0].grid(True)

    # Señal recibida
    axs[1].plot(t_y, y)

    titulo = "Señal recibida"
    if snr_db is not None:
        titulo += f" (SNR = {snr_db} dB)"

    axs[1].set_title(titulo)
    axs[1].set_xlabel("Tiempo (s)")
    axs[1].set_ylabel("Amplitud")
    axs[1].grid(True)

    # Correlación directa
    axs[2].plot(t_r, r_directa)

    for retardo in retardos_reales:
        axs[2].axvline(
            retardo / fs,
            linestyle="--",
            label=f"Real: {retardo}"
        )

    for retardo in retardos_directa:
        axs[2].axvline(
            retardo / fs,
            linestyle=":",
            label=f"Detectado: {retardo}"
        )

    axs[2].set_title("Correlación directa")
    axs[2].set_xlabel("Tiempo (s)")
    axs[2].set_ylabel("Correlación")
    axs[2].legend()
    axs[2].grid(True)

    # Correlación FFT
    axs[3].plot(t_r, r_fft)

    for retardo in retardos_reales:
        axs[3].axvline(
            retardo / fs,
            linestyle="--",
            label=f"Real: {retardo}"
        )

    for retardo in retardos_fft:
        axs[3].axvline(
            retardo / fs,
            linestyle=":",
            label=f"Detectado: {retardo}"
        )

    axs[3].set_title("Correlación mediante FFT")
    axs[3].set_xlabel("Tiempo (s)")
    axs[3].set_ylabel("Correlación")
    axs[3].legend()
    axs[3].grid(True)

    plt.tight_layout()

    plt.savefig(
        os.path.join(
            FIG_DIR,
            f"experimento_{etiqueta}.png"
        ),
        dpi=150
    )

    plt.close(fig)

    return (
        t_directa,
        t_fft,
        retardos_directa,
        retardos_fft
    )


def main():
    np.random.seed(0)

    fs = 48000

    x = generar_chirp(
        fs=fs,
        duracion=0.005,
        f_inicial=2000,
        f_final=8000
    )

    # ========================================
    # Experimento 1: un eco limpio
    # ========================================

    eco_simple = [
        (480, 0.6)
    ]

    experimento(
        fs,
        x,
        eco_simple,
        snr_db=None,
        etiqueta="eco_simple"
    )

    # ========================================
    # Experimento 2: múltiples ecos
    # ========================================

    ecos_multiples = [
        (480, 0.6),
        (900, 0.35),
        (1400, 0.2)
    ]

    experimento(
        fs,
        x,
        ecos_multiples,
        snr_db=None,
        etiqueta="ecos_multiples"
    )

    # ========================================
    # Experimento 3: múltiples ecos + ruido
    # ========================================

    experimento(
        fs,
        x,
        ecos_multiples,
        snr_db=10,
        etiqueta="ecos_multiples_ruido"
    )

if __name__ == "__main__":
    main()