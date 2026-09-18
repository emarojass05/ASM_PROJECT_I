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


void setup() {
  Serial.begin(115200);
  generar_chirp(); // crear el arreglo del chirp

  // El timer cuenta a 48 000 Hz
  timerChirp = timerBegin(FS);

  // Ejecutar siguienteMuestra cuando ocurra la interrupcion
  timerAttachInterrupt(timerChirp, &siguienteMuestra);

  // Interrupcion cada 1 tick del timer
  timerAlarm(timerChirp, 1, true, 0);

}

void loop() {
  reproducir_chirp();

  delay(1000);
}


