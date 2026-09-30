// creacion del chirp

// va a ir de un rango de 2 a 8 khz
// formula = f(t) = f0 + (f1 - f0)t/T 
// donde f0 es la frec incicial y f1 la final
// T duracion total y t la variable en el tiempo

#define FS 24000
#define F_INICIAL 2000.0
#define F_FINAL 8000.0
#define DURACION 0.002


uint8_t chirp[CHIRP_SIZE];

volatile int indiceChirp = 0;
volatile bool reproduciendo = false;

void generar_chirp() {
  float a = (F_FINAL - F_INICIAL) / DURACION;

  for (int n = 0; n < CHIRP_SIZE; n++) {

    float t = (float)n / FS;

    float fase = 2.0 * PI *
                 (F_INICIAL * t +
                 0.5 * a * t * t);

    float muestra = sin(fase);

    chirp[n] = (uint8_t)(127.5 + 127.5 * muestra);
  }
}

void reproducir_chirp() {
  indiceChirp = 0;
  reproduciendo = true;
}

void ARDUINO_ISR_ATTR siguienteMuestra() {

  if (!reproduciendo) {
    return;
  }

  if (indiceChirp < CHIRP_SIZE) {

    dacWrite(SPEAKER, chirp[indiceChirp]);
    indiceChirp++;

  } else {

    reproduciendo = false;
    indiceChirp = 0;

    dacWrite(SPEAKER, 128);
  }
}