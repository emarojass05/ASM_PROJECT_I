// archivo principal para compilacion
// Implementacion del radar en fisico con microcontrolador ESP32 
#include <math.h>

// -------------------------- Pines y constantes ------------------------
const int MIC = 34;
const int SPEAKER = 25;
const int LCD_SDA = 21;
const int LCD_SCL = 22;
#define FS 48000 // frecuencia de muestreo
hw_timer_t *timerChirp = NULL;

volatile unsigned long tiempoInicio = 0;
volatile unsigned long tiempoFin = 0;
volatile bool chirpTerminado = false;

#define DURACION_CAPTURA 0.020
const int N_MIC = FS * DURACION_CAPTURA;

volatile uint16_t muestrasMic[N_MIC];

volatile int indiceMic = 0;
volatile bool capturandoMic = false;
volatile bool capturaTerminada = false;

hw_timer_t *timerMic = NULL;

void setup() {
  Serial.begin(115200);
  generar_chirp(); // crear el arreglo del chirp

   // Timer del parlante
    timerChirp = timerBegin(FS);
    timerAttachInterrupt(timerChirp, &siguienteMuestra);
    timerAlarm(timerChirp, 1, true, 0);

    // Timer del microfono
    timerMic = timerBegin(FS);
    timerAttachInterrupt(timerMic, &tomarMuestraMic);
    timerAlarm(timerMic, 1, true, 0);

}

void loop() {

    iniciarCapturaMic();

    reproducir_chirp();

    while (!capturaTerminada) {
    }

    // Analizar lo que recibio el microfono
    uint16_t minimo = 4095;
    uint16_t maximo = 0;

    for (int i = 0; i < N_MIC; i++) {

        if (muestrasMic[i] < minimo) {
            minimo = muestrasMic[i];
        }

        if (muestrasMic[i] > maximo) {
            maximo = muestrasMic[i];
        }
    }

    Serial.print("Min: ");
    Serial.print(minimo);

    Serial.print(" | Max: ");
    Serial.print(maximo);

    Serial.print(" | Amplitud: ");
    Serial.println(maximo - minimo);

    delay(1000);
}


