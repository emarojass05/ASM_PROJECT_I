// creacion del chirp

// va a ir de un rango de 2 a 8 khz
// formula = f(t) = f0 + (f1 - f0)t/T 
// donde f0 es la frec incicial y f1 la final
// T duracion total y t la variable en el tiempo

// --------- constantes del chirp -----------------
#define FS 48000 // frecuencia de muestreo
#define F_INICIAL 2000.0
#define F_FINAL 8000.0
#define DURACION 0.005

const int N = FS * DURACION; // numero de muestras

uint8_t chirp[N];

volatile int indiceChirp = 0;
volatile bool reproduciendo = false;

void generar_chirp(){
  float a = (F_FINAL - F_INICIAL) / DURACION;

  for (int n = 0; n < N; n++) {

    float t = (float)n / FS; // segundo del chirp

    float fase = (2.0 * PI) * (F_INICIAL * t + 0.5 * a * (t * t)); // fase para la señal

    float muestra = sin(fase);

    // para tenerlo en 8 bits:
    chirp[n] = (uint8_t)(127.5 + 127.5 * muestra);
  }
} 

void reproducir_chirp() {
    indiceChirp = 0;
    tiempoInicio = micros();
    reproduciendo = true;
}


void ARDUINO_ISR_ATTR siguienteMuestra() {

    if (!reproduciendo) {
        return;
    }

    if (indiceChirp < N) {

        dacWrite(SPEAKER, chirp[indiceChirp]);
        indiceChirp++;

    } else {

        tiempoFin = micros();

        reproduciendo = false;
        indiceChirp = 0;
        chirpTerminado = true;

        dacWrite(SPEAKER, 128);
    }
}
