#include <driver/adc.h>

// ---------------------------------------------------------
// Configuración del micrófono
// ---------------------------------------------------------

// GPIO34 = ADC1_CHANNEL_6 en ESP32
#define MIC_CHANNEL ADC1_CHANNEL_6

// FS y CAPTURA_SIZE se definen en el archivo principal:
// #define FS 24000
// #define CAPTURA_SIZE 256

// Lecturas crudas del ADC
uint16_t muestrasMic[CAPTURA_SIZE];

// Señal centrada alrededor de cero.
// Esta se utilizará tanto para FFT como para correlación.
double muestrasCentradas[CAPTURA_SIZE];


// ---------------------------------------------------------
// Inicializar ADC
// ---------------------------------------------------------

void iniciarMicrofono() {

  // ADC de 12 bits: valores entre 0 y 4095
  adc1_config_width(ADC_WIDTH_BIT_12);

  // GPIO34 = ADC1_CHANNEL_6
  adc1_config_channel_atten(
    MIC_CHANNEL,
    ADC_ATTEN_DB_11
  );

  Serial.println("ADC del microfono configurado.");
}


// ---------------------------------------------------------
// Capturar 256 muestras a aproximadamente 24 kHz
// ---------------------------------------------------------

void capturarMicrofono() {

  /*
    FS = 24000 Hz

    Periodo ideal:
    1 / 24000 = 41.666... us

    No podemos representar directamente 41.666 us con
    micros(), por eso alternamos intervalos de 41 y 42 us
    utilizando un acumulador de residuo.
  */

  const uint32_t periodoBase = 1000000UL / FS;
  const uint32_t residuo = 1000000UL % FS;

  uint32_t siguiente = micros();
  uint32_t acumuladorResiduo = 0;

  for (int i = 0; i < CAPTURA_SIZE; i++) {

    // Esperar hasta el instante de la siguiente muestra
    while ((int32_t)(micros() - siguiente) < 0) {
      // espera activa
    }

    // Leer ADC
    muestrasMic[i] = adc1_get_raw(MIC_CHANNEL);

    // Programar la siguiente muestra
    siguiente += periodoBase;

    /*
      Para FS = 24000:

      1 000 000 / 24 000 =
      41 + 16000/24000 us

      El residuo permite agregar 1 us cuando corresponde.
    */
    acumuladorResiduo += residuo;

    if (acumuladorResiduo >= FS) {
      siguiente++;
      acumuladorResiduo -= FS;
    }
  }
}


// ---------------------------------------------------------
// Eliminar componente DC
// ---------------------------------------------------------

void centrarMuestras() {

  int32_t suma = 0;

  // Calcular promedio
  for (int i = 0; i < CAPTURA_SIZE; i++) {
    suma += muestrasMic[i];
  }

  double promedio =
      (double)suma / CAPTURA_SIZE;

  // Restar promedio
  for (int i = 0; i < CAPTURA_SIZE; i++) {

    muestrasCentradas[i] =
        (double)muestrasMic[i] - promedio;
  }
}


// ---------------------------------------------------------
// Mostrar señal cruda
// ---------------------------------------------------------

void imprimirMuestrasMicrofono() {

  Serial.println("------ Muestras ADC ------");

  for (int i = 0; i < CAPTURA_SIZE; i++) {
    Serial.println(muestrasMic[i]);
  }
}


// ---------------------------------------------------------
// Mostrar señal centrada
// ---------------------------------------------------------

void imprimirMuestrasCentradas() {

  Serial.println("------ Muestras centradas ------");

  for (int i = 0; i < CAPTURA_SIZE; i++) {
    Serial.println(muestrasCentradas[i]);
  }
}


// ---------------------------------------------------------
// Información básica de la captura
// ---------------------------------------------------------

void analizarCapturaMicrofono() {

  uint16_t minimo = 4095;
  uint16_t maximo = 0;

  uint32_t suma = 0;

  for (int i = 0; i < CAPTURA_SIZE; i++) {

    uint16_t valor = muestrasMic[i];

    if (valor < minimo) {
      minimo = valor;
    }

    if (valor > maximo) {
      maximo = valor;
    }

    suma += valor;
  }

  double promedio =
      (double)suma / CAPTURA_SIZE;

  uint16_t amplitud =
      maximo - minimo;

  Serial.print("Min: ");
  Serial.print(minimo);

  Serial.print(" | Max: ");
  Serial.print(maximo);

  Serial.print(" | Amplitud: ");
  Serial.print(amplitud);

  Serial.print(" | Promedio: ");
  Serial.println(promedio);
}