// deteccion.ino
// deteccion del retardo del eco por correlacion y calculo de distancia

#define V_SONIDO 343.0 // velocidad del sonido en m/s
#define MUESTRAS_GUARDA 60 // debe ser mayor a N (48 con el chirp de 1ms); da un piso de ~21cm de distancia minima
                           // calibrar con hardware real si sigue detectando falsos ecos

float plantilla[N];

void prepararPlantilla() {
  for (int n = 0; n < N; n++) {
    plantilla[n] = (float)chirp[n] - 127.5;
  }
}

int detectar_retardo() {

  // promedio del buffer para quitar el offset de dc
  float promedio = 0;
  for (int i = 0; i < N_MIC; i++) {
    promedio += muestrasMic[i];
  }
  promedio /= N_MIC;

  int mejorRetardo = -1;
  float mejorCorrelacion = 0;

  for (int k = MUESTRAS_GUARDA; k < N_MIC - N; k++) {

    float suma = 0;

    for (int n = 0; n < N; n++) {
      suma += plantilla[n] * (muestrasMic[k + n] - promedio);
    }

    if (suma > mejorCorrelacion) {
      mejorCorrelacion = suma;
      mejorRetardo = k;
    }
  }

  return mejorRetardo;
}

float calcular_distancia(int retardo_muestras) {
  float tau = (float)retardo_muestras / FS;
  return V_SONIDO * tau / 2.0;
}
