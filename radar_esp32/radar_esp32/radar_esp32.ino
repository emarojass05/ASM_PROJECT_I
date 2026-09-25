// archivo principal para compilacion
// Implementacion del radar en fisico con microcontrolador ESP32
#include <math.h>

SET_LOOP_TASK_STACK_SIZE(16384); // el fft recursivo necesita mas stack que el default (8kb)

// -------------------------- Pines y constantes ------------------------
const int MIC = 34;
const int SPEAKER = 25;
const int LCD_SDA = 21;
const int LCD_SCL = 22;
#define FS 48000 // frecuencia de muestreo
hw_timer_t *timerChirp = NULL;

// declaradas en captura_mic.ino, pero el archivo principal se compila primero
extern hw_timer_t *timerMic;
extern volatile bool capturaTerminada;


void setup() {
  Serial.begin(115200);

  configurar_adc();
  iniciarPantalla();
  generar_chirp();     // crear el arreglo del chirp
  prepararPlantilla(); // copia del chirp para correlacionar

  // El timer cuenta a 48 000 Hz
  timerChirp = timerBegin(FS);
  timerAttachInterrupt(timerChirp, &siguienteMuestra);
  timerAlarm(timerChirp, 1, true, 0);

  // Timer de captura del microfono, tambien a 48 000 Hz
  timerMic = timerBegin(FS);
  timerAttachInterrupt(timerMic, &tomarMuestraMic);
  timerAlarm(timerMic, 1, true, 0);
}

void loop() {

  iniciarCapturaMic();
  reproducir_chirp();

  while (!capturaTerminada) {
    // esperar a que termine la ventana de captura
  }

  int retardo = detectar_retardo();
  float frecuencia = analizar_espectro();

  if (retardo >= 0) {
    float distancia = calcular_distancia(retardo);

    Serial.print("Retardo: ");
    Serial.print(retardo);
    Serial.print(" muestras | Distancia: ");
    Serial.print(distancia);
    Serial.println(" m");

    mostrarResultado(distancia, frecuencia);

  } else {
    Serial.println("No se detecto eco");
    mostrarSinEco();
  }

  delay(1000);
}