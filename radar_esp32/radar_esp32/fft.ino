#include <arduinoFFT.h>

// ---------------------------------------------------------
// Arreglos FFT
// ---------------------------------------------------------

double vReal[FFT_SIZE];
double vImag[FFT_SIZE];

ArduinoFFT<double> FFT =
    ArduinoFFT<double>(
      vReal,
      vImag,
      FFT_SIZE,
      FS
    );


// ---------------------------------------------------------
// Preparar muestras para FFT
// Las muestras ya vienen centradas
// ---------------------------------------------------------

void prepararFFT() {

  for (int i = 0; i < FFT_SIZE; i++) {

    vReal[i] = muestrasCentradas[i];

    // Entrada puramente real
    vImag[i] = 0.0;
  }
}


// ---------------------------------------------------------
// Calcular FFT
// ---------------------------------------------------------

void calcularFFT() {

  prepararFFT();

  // Reducir fuga espectral
  FFT.windowing(
    FFTWindow::Hamming,
    FFTDirection::Forward
  );

  // FFT
  FFT.compute(
    FFTDirection::Forward
  );

  // Parte compleja -> magnitud
  FFT.complexToMagnitude();
}


// ---------------------------------------------------------
// Mostrar espectro completo
// ---------------------------------------------------------

void imprimirFFT() {

  Serial.println("------ FFT ------");

  // Solo N/2 por Nyquist
  for (int k = 0; k < FFT_SIZE / 2; k++) {

    double frecuencia =
        ((double)k * FS) / FFT_SIZE;

    Serial.print("Bin ");
    Serial.print(k);

    Serial.print(" | ");
    Serial.print(frecuencia);
    Serial.print(" Hz");

    Serial.print(" | Magnitud: ");
    Serial.println(vReal[k]);
  }
}


// ---------------------------------------------------------
// Mostrar banda del chirp: 2 kHz - 8 kHz
// ---------------------------------------------------------

void imprimirBandaChirp() {

  Serial.println("------ Banda 2-8 kHz ------");

  for (int k = 0; k < FFT_SIZE / 2; k++) {

    double frecuencia =
        ((double)k * FS) / FFT_SIZE;

    if (
      frecuencia >= F_INICIAL &&
      frecuencia <= F_FINAL
    ) {

      Serial.print("Bin ");
      Serial.print(k);

      Serial.print(" | ");
      Serial.print(frecuencia);
      Serial.print(" Hz");

      Serial.print(" | Magnitud: ");
      Serial.println(vReal[k]);
    }
  }
}