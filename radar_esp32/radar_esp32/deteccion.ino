// deteccion.ino
// deteccion del retardo del eco por correlacion y calculo de distancia

#define V_SONIDO 343.0 // velocidad del sonido en m/s
#define MUESTRAS_GUARDA 120 // debe ser mayor a N (96 con el chirp de 2ms); da un piso de ~43cm de distancia minima
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

  Serial.print("Correlacion pico: ");
  Serial.println(mejorCorrelacion);

  return mejorRetardo;
}

void calibrar_correlacion() {

  // recorre toda la ventana (sin guard) y muestra los 5 picos mas fuertes,
  // para ver donde esta el artefacto de acople directo / resonancia y calibrar el guard con datos reales

  float promedio = 0;
  for (int i = 0; i < N_MIC; i++) {
    promedio += muestrasMic[i];
  }
  promedio /= N_MIC;

  const int TOP = 5;
  int topK[TOP] = {-1, -1, -1, -1, -1};
  float topVal[TOP] = {0, 0, 0, 0, 0};

  for (int k = 1; k < N_MIC - N; k++) {

    float suma = 0;
    for (int n = 0; n < N; n++) {
      suma += plantilla[n] * (muestrasMic[k + n] - promedio);
    }

    for (int i = 0; i < TOP; i++) {
      if (suma > topVal[i]) {
        for (int j = TOP - 1; j > i; j--) {
          topVal[j] = topVal[j - 1];
          topK[j] = topK[j - 1];
        }
        topVal[i] = suma;
        topK[i] = k;
        break;
      }
    }
  }

  Serial.println("--- calibracion: top 5 picos de correlacion ---");
  for (int i = 0; i < TOP; i++) {
    float distancia_cm = V_SONIDO * ((float)topK[i] / FS) / 2.0 * 100.0;
    Serial.print("Muestra: ");
    Serial.print(topK[i]);
    Serial.print(" | Correlacion: ");
    Serial.print(topVal[i]);
    Serial.print(" | Distancia equivalente: ");
    Serial.print(distancia_cm);
    Serial.println(" cm");
  }
  Serial.println("-----------------------------------------------");
}

float calcular_distancia(int retardo_muestras) {
  float tau = (float)retardo_muestras / FS;
  return V_SONIDO * tau / 2.0;
}
