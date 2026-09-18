// archivo principal para compilacion
// Implementacion del radar en fisico con microcontrolador ESP32 
#include <math.h>

// -------------------------- Pines y constantes ------------------------
const int MIC = 34;
const int SPEAKER = 25;
const int LCD_SDA = 21;
const int LCD_SCL = 22;


void setup() {
  Serial.begin(115200);
  generar_chirp(); // crear el arreglo del chirp

}

void loop() {
  reproducir_chirp();

  
}
