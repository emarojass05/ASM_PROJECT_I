"""
magnitud_fase.py

Experimentos de magnitud y fase utilizando la FFT propia.
"""

import numpy as np
import matplotlib.pyplot as plt
import os
import sys
sys.path.append(os.path.join(os.path.dirname(__file__), "..", "dsp"))
from transformadas import fft_radix2

FIG_DIR = os.path.join(os.path.dirname(__file__), "figuras")
os.makedirs(FIG_DIR, exist_ok=True)

def obtener_espectro(x, fs):
    """
    Calcula las frecuencias, magnitudes y fases positivas de una señal.

    """
    N = len(x)

    X = fft_radix2(x)

    # Para una señal real, la segunda mitad de la FFT
    # contiene información simétrica, por lo que se usa
    # solamente el espectro positivo.
    mitad = N // 2 + 1

    X = X[:mitad]

    frecuencias = np.arange(mitad) * fs / N

    # Normalización para recuperar aproximadamente
    # las amplitudes originales.
    magnitud = np.abs(X) / N

    # Las frecuencias positivas representan también
    # su componente negativa correspondiente.
    magnitud[1:-1] *= 2

    fase = np.angle(X)

    # Elimina pequeños errores numericos alrededor de 0 rad
    fase[np.isclose(fase, 0.0, atol=1e-10)] = 0.0

    # La fase no tiene significado cuando prácticamente
    # no existe una componente en esa frecuencia.
    umbral = np.max(magnitud) * 1e-6
    fase[magnitud < umbral] = np.nan

    return frecuencias, magnitud, fase


def graficar_senal(t, x, frecuencias, magnitud, fase, titulo, numero_figura):
    """Grafica la señal en tiempo, su magnitud y su fase."""
    fig, ax = plt.subplots(3, 1, figsize=(9, 8))

    ax[0].plot(t, x)
    ax[0].set_title(f"{titulo} - Dominio del tiempo")
    ax[0].set_xlabel("Tiempo (s)")
    ax[0].set_ylabel("Amplitud")
    ax[0].grid(True)
    ax[0].set_xlim(0, 0.1)

    ax[1].stem(frecuencias, magnitud)
    ax[1].set_title("Espectro de magnitud")
    ax[1].set_xlabel("Frecuencia (Hz)")
    ax[1].set_ylabel("Magnitud")
    ax[1].grid(True)

    ax[2].stem(frecuencias, np.degrees(fase))
    ax[2].set_title("Espectro de fase")
    ax[2].set_xlabel("Frecuencia (Hz)")
    ax[2].set_ylabel("Fase (grados)")
    ax[2].grid(True)

    fig.tight_layout()

    out_path = os.path.join(FIG_DIR, f"magnitud_fase{numero_figura}.png")
    fig.savefig(out_path, dpi=150)
    print("Figura guardada en", out_path)

    # Si no hay pantalla disponible (por ejemplo, corriendo por SSH o en un
    # servidor de correccion automatica), plt.show() puede fallar o quedar
    # esperando; se ignora el error para que el script no se cuelgue.
    try:
        plt.show()
    except Exception:
        pass


def analizar_senal(x, t, fs, titulo, fig):
    """Calcula y muestra el espectro de una señal."""
    frecuencias, magnitud, fase = obtener_espectro(x, fs)

    graficar_senal(t, x, frecuencias, magnitud, fase, titulo, fig)


def main():
    fs = 4096
    N = 4096

    t = np.arange(N) / fs

    # Experimento 1:
    # Una componente de 500 Hz sin desplazamiento de fase.
    x1 = np.cos(2 * np.pi * 500 * t)

    analizar_senal( x1, t, fs, "Señal de 500 Hz y fase 0°", 1)

    # Experimento 2:
    # La misma frecuencia con un desfase de 45°.
    x2 = np.cos( 2 * np.pi * 500 * t  + np.pi / 4  )

    analizar_senal( x2, t, fs, "Señal de 500 Hz y fase 45°" , 2)

    # Experimento 3:
    # Dos componentes con diferentes amplitudes y fases.
    x3 = (np.cos(2 * np.pi * 500 * t) + 0.5 * np.cos( 2 * np.pi * 1200 * t + np.pi / 3))

    analizar_senal(x3, t, fs, "Señal compuesta de 500 Hz y 1200 Hz", 3)


if __name__ == "__main__":
    main()
