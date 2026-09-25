// archivo principal para compilacion
// Implementacion del radar en fisico con microcontrolador ESP32
#include <math.h>

SET_LOOP_TASK_STACK_SIZE(32768); // el fft recursivo necesita bastante mas stack que el default (8kb); 16kb no alcanzo

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

  // escribir "cal" en el Monitor Serial y enter para ver los 5 picos mas fuertes
  // de esta captura (para calibrar MUESTRAS_GUARDA con datos reales)
  if (Serial.available()) {
    String comando = Serial.readStringUntil('\n');
    comando.trim();
    if (comando == "cal") {
      calibrar_correlacion();
    }
  }

  int retardo = filtrar_retardo(detectar_retardo());
  float frecuencia = analizar_espectro();

  if (retardo >= 0) {
    float distancia_m = calcular_distancia(retardo);
    float distancia_cm = distancia_m * 100.0;

    Serial.print("Retardo: ");
    Serial.print(retardo);
    Serial.print(" muestras | Distancia: ");
    Serial.print(distancia_cm);
    Serial.println(" cm");

    mostrarResultado(distancia_cm, frecuencia);

  } else {
    Serial.println("No se detecto eco");
    mostrarSinEco();
  }

  delay(1000);
}