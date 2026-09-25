// deteccion.ino
// deteccion del retardo del eco por correlacion y calculo de distancia

#define V_SONIDO 343.0 // velocidad del sonido en m/s
#define MUESTRAS_GUARDA 40 // debe ser mayor a N (24 con el chirp de 0.5ms); da un piso de ~14cm de distancia minima
                           // el limite fisico real esta cerca de este valor, no se puede bajar mucho mas

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

int filtrar_retardo(int retardo) {

  // guarda las ultimas 5 lecturas validas y devuelve la mediana,
  // para no saltar entre el eco real y reflejos multiples (multipath) de un ciclo a otro

  static int historial[5] = {-1, -1, -1, -1, -1};
  static int indice = 0;

  if (retardo < 0) {
    return retardo; // no se detecto nada, no se toca el historial
  }

  historial[indice] = retardo;
  indice = (indice + 1) % 5;

  int ordenado[5];
  memcpy(ordenado, historial, sizeof(historial));

  for (int i = 1; i < 5; i++) {
    int clave = ordenado[i];
    int j = i - 1;
    while (j >= 0 && ordenado[j] > clave) {
      ordenado[j + 1] = ordenado[j];
      j--;
    }
    ordenado[j + 1] = clave;
  }

  return ordenado[2]; // mediana
}

float calcular_distancia(int retardo_muestras) {
  float tau = (float)retardo_muestras / FS;
  return V_SONIDO * tau / 2.0;
}
