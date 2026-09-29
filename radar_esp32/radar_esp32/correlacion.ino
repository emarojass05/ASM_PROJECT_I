#include <arduinoFFT.h>
#define VELOCIDAD_SONIDO 343.0
#define DISTANCIA_MINIMA 0.15   // 15 cm


extern uint8_t chirp[CHIRP_SIZE];

double chirpCentrado[CHIRP_SIZE];

double correlacionDirecta[CORR_SIZE];
double correlacionFFT[CORR_SIZE];

double corrXReal[CORR_FFT_SIZE];
double corrXImag[CORR_FFT_SIZE];

double corrYReal[CORR_FFT_SIZE];
double corrYImag[CORR_FFT_SIZE];

double correlacionBase[CORR_SIZE];
double diferenciaCorrelacion[CORR_SIZE];

bool calibracionLista = false;
int picoDirectoBase = -1;

// Objetos FFT independientes
ArduinoFFT<double> FFTCorrX =
    ArduinoFFT<double>(
      corrXReal,
      corrXImag,
      CORR_FFT_SIZE,
      FS
    );

ArduinoFFT<double> FFTCorrY =
    ArduinoFFT<double>(
      corrYReal,
      corrYImag,
      CORR_FFT_SIZE,
      FS
    );


// ---------------------------------------------------------
// Preparar chirp de referencia
// ---------------------------------------------------------

void prepararChirpCorrelacion() {

  /*
    chirp[] tiene valores aproximadamente:

    0 ... 255

    y está centrado en 127.5.

    Para correlación queremos:

    -127.5 ... +127.5
  */

  for (int i = 0; i < CHIRP_SIZE; i++) {

    chirpCentrado[i] =
        (double)chirp[i] - 127.5;
  }
}


// =========================================================
// CORRELACIÓN DIRECTA
// =========================================================

void calcularCorrelacionDirecta() {

  for (int k = 0; k < CORR_SIZE; k++) {

    double suma = 0.0;

    for (int n = 0; n < CHIRP_SIZE; n++) {

      suma +=
        chirpCentrado[n] *
        muestrasCentradas[n + k];
    }

    correlacionDirecta[k] = suma;
  }
}


// =========================================================
// CORRELACIÓN MEDIANTE FFT
// =========================================================

void calcularCorrelacionFFT() {

  // -------------------------------------------------------
  // 1. Zero padding
  // -------------------------------------------------------

  for (int i = 0; i < CORR_FFT_SIZE; i++) {

    corrXReal[i] = 0.0;
    corrXImag[i] = 0.0;

    corrYReal[i] = 0.0;
    corrYImag[i] = 0.0;
  }


  // -------------------------------------------------------
  // 2. Copiar chirp
  // -------------------------------------------------------

  for (int i = 0; i < CHIRP_SIZE; i++) {

    corrXReal[i] = chirpCentrado[i];
  }


  // -------------------------------------------------------
  // 3. Copiar señal capturada
  // -------------------------------------------------------

  for (int i = 0; i < CAPTURA_SIZE; i++) {

    corrYReal[i] = muestrasCentradas[i];
  }


  // -------------------------------------------------------
  // 4. FFT del chirp X
  // -------------------------------------------------------

  FFTCorrX.compute(
    FFTDirection::Forward
  );


  // -------------------------------------------------------
  // 5. FFT de señal Y
  // -------------------------------------------------------

  FFTCorrY.compute(
    FFTDirection::Forward
  );


  // -------------------------------------------------------
  // 6. R = conj(X) * Y
  // -------------------------------------------------------

  for (int i = 0; i < CORR_FFT_SIZE; i++) {

    double xr = corrXReal[i];
    double xi = corrXImag[i];

    double yr = corrYReal[i];
    double yi = corrYImag[i];


    /*
      conj(X) = xr - j*xi

      (xr - j*xi)(yr + j*yi)

      Real:
        xr*yr + xi*yi

      Imag:
        xr*yi - xi*yr
    */

    double real =
        xr * yr +
        xi * yi;

    double imag =
        xr * yi -
        xi * yr;


    /*
      Guardamos el producto en Y,
      porque X ya no lo necesitamos.
    */

    corrYReal[i] = real;
    corrYImag[i] = imag;
  }


  // -------------------------------------------------------
  // 7. IFFT
  // -------------------------------------------------------

  FFTCorrY.compute(
    FFTDirection::Reverse
  );


  // -------------------------------------------------------
  // 8. Obtener región válida
  // -------------------------------------------------------

  for (int k = 0; k < CORR_SIZE; k++) {

    /*
      La escala absoluta no nos preocupa demasiado
      para encontrar los picos.

      Dividimos por CORR_FFT_SIZE para normalizar
      explícitamente el resultado.
    */

    correlacionFFT[k] =
        corrYReal[k] / CORR_FFT_SIZE;
  }
}


// =========================================================
// ENCONTRAR PICO MÁS GRANDE
// =========================================================

int obtenerPicoCorrelacionDirecta() {

  int indiceMax = 0;

  double maximo =
      fabs(correlacionDirecta[0]);

  for (int i = 1; i < CORR_SIZE; i++) {

    double valor =
        fabs(correlacionDirecta[i]);

    if (valor > maximo) {

      maximo = valor;
      indiceMax = i;
    }
  }

  return indiceMax;
}


int obtenerPicoCorrelacionFFT() {

  int indiceMax = 0;

  double maximo =
      fabs(correlacionFFT[0]);

  for (int i = 1; i < CORR_SIZE; i++) {

    double valor =
        fabs(correlacionFFT[i]);

    if (valor > maximo) {

      maximo = valor;
      indiceMax = i;
    }
  }

  return indiceMax;
}


// =========================================================
// IMPRIMIR CORRELACIÓN DIRECTA
// =========================================================

void imprimirCorrelacionDirecta() {

  Serial.println(
    "------ Correlacion directa ------"
  );

  for (int i = 0; i < CORR_SIZE; i++) {

    Serial.print(i);

    Serial.print(" | ");

    Serial.println(
      correlacionDirecta[i]
    );
  }
}


// =========================================================
// IMPRIMIR CORRELACIÓN FFT
// =========================================================

void imprimirCorrelacionFFT() {

  Serial.println(
    "------ Correlacion FFT ------"
  );

  for (int i = 0; i < CORR_SIZE; i++) {

    Serial.print(i);

    Serial.print(" | ");

    Serial.println(
      correlacionFFT[i]
    );
  }
}


// ---------------------------------------------------------
// Detectar picos locales en una correlacion
// ---------------------------------------------------------

int detectarPicos(
  double correlacion[],
  int longitud,
  int picosEncontrados[],
  int maxPicos,
  double umbralRelativo = 0.25,
  int distanciaMinima = 1
) {

  // 1. Encontrar magnitud maxima absoluta
  double maximoGlobal = 0.0;

  for (int i = 0; i < longitud; i++) {

    double valor = fabs(correlacion[i]);

    if (valor > maximoGlobal) {
      maximoGlobal = valor;
    }
  }

  if (maximoGlobal == 0.0) {
    return 0;
  }

  // Umbral relativo
  double umbral =
      maximoGlobal * umbralRelativo;


  // -------------------------------------------------------
  // Guardar candidatos
  // -------------------------------------------------------

  int candidatos[CORR_SIZE];
  int cantidadCandidatos = 0;

  for (int i = 0; i < longitud; i++) {

    double valor = fabs(correlacion[i]);

    // Debe superar el umbral
    if (valor < umbral) {
      continue;
    }


    // Maximo respecto al vecino izquierdo
    bool maxIzquierda =
        (i == 0) ||
        (valor > fabs(correlacion[i - 1]));


    // Maximo respecto al vecino derecho
    bool maxDerecha =
        (i == longitud - 1) ||
        (valor >= fabs(correlacion[i + 1]));


    if (maxIzquierda && maxDerecha) {

      candidatos[cantidadCandidatos] = i;
      cantidadCandidatos++;
    }
  }


  // -------------------------------------------------------
  // Ordenar candidatos por fuerza
  // Mayor magnitud primero
  // -------------------------------------------------------

  for (int i = 0; i < cantidadCandidatos - 1; i++) {

    for (int j = i + 1; j < cantidadCandidatos; j++) {

      double valorI =
          fabs(correlacion[candidatos[i]]);

      double valorJ =
          fabs(correlacion[candidatos[j]]);

      if (valorJ > valorI) {

        int temp = candidatos[i];
        candidatos[i] = candidatos[j];
        candidatos[j] = temp;
      }
    }
  }


  // -------------------------------------------------------
  // Seleccionar picos respetando distancia minima
  // -------------------------------------------------------

  int cantidadPicos = 0;

  for (int i = 0; i < cantidadCandidatos; i++) {

    int candidato = candidatos[i];

    bool suficientementeLejos = true;

    for (int j = 0; j < cantidadPicos; j++) {

      if (
        abs(candidato - picosEncontrados[j])
        < distanciaMinima
      ) {

        suficientementeLejos = false;
        break;
      }
    }

    if (suficientementeLejos) {

      picosEncontrados[cantidadPicos] =
          candidato;

      cantidadPicos++;

      // Evitar llenar el arreglo
      if (cantidadPicos >= maxPicos) {
        break;
      }
    }
  }


  // -------------------------------------------------------
  // Ordenar picos por posicion temporal
  // -------------------------------------------------------

  for (int i = 0; i < cantidadPicos - 1; i++) {

    for (int j = i + 1; j < cantidadPicos; j++) {

      if (
        picosEncontrados[j]
        < picosEncontrados[i]
      ) {

        int temp = picosEncontrados[i];

        picosEncontrados[i] =
            picosEncontrados[j];

        picosEncontrados[j] = temp;
      }
    }
  }

  return cantidadPicos;
}

// ---------------------------------------------------------
// Buscar pico directo y posible eco
// ---------------------------------------------------------

ResultadoEco detectarEco(
  double correlacion[],
  int longitud,
  double umbralEcoRelativo = 0.25
) {

  ResultadoEco resultado;

  resultado.encontrado = false;
  resultado.picoDirecto = -1;
  resultado.picoEco = -1;
  resultado.valorDirecto = 0.0;
  resultado.valorEco = 0.0;
  resultado.diferenciaMuestras = 0;
  resultado.tiempoVuelo = 0.0;
  resultado.distancia = 0.0;


  // =====================================================
  // 1. BUSCAR PICO DIRECTO
  // =====================================================

  double maxDirecto = 0.0;
  int indiceDirecto = 0;

  /*
    Buscamos el sonido directo solamente al inicio.

    En tus pruebas aparece aproximadamente
    alrededor de la muestra 13.

    60 muestras son 2.5 ms a 24 kHz.
  */
  int limiteDirecto = 30;

  if (limiteDirecto > longitud) {
    limiteDirecto = longitud;
  }

  for (int i = 0; i < limiteDirecto; i++) {

    double valor = fabs(correlacion[i]);

    if (valor > maxDirecto) {

      maxDirecto = valor;
      indiceDirecto = i;
    }
  }

  resultado.picoDirecto = indiceDirecto;
  resultado.valorDirecto = correlacion[indiceDirecto];


  // =====================================================
  // 2. IGNORAR LOBULOS DEL PICO DIRECTO
  // =====================================================

  /*
    El chirp tiene 48 muestras.

    Ignoramos una región completa después del pico
    directo para no confundir la estructura de
    autocorrelación del chirp con un eco.
  */

  int muestrasMinimas =
    (int)ceil(
      (2.0 * DISTANCIA_MINIMA / VELOCIDAD_SONIDO)
      * FS
    );

  int inicioBusquedaEco =
     indiceDirecto + muestrasMinimas;

  if (inicioBusquedaEco >= longitud) {

    // No existe espacio suficiente para buscar eco
    return resultado;
  }


  // =====================================================
  // 3. UMBRAL DEL ECO
  // =====================================================

  double umbralEco =
      maxDirecto * umbralEcoRelativo;


  // =====================================================
  // 4. BUSCAR MEJOR PICO POSTERIOR
  // =====================================================

  double maxEco = 0.0;
  int indiceEco = -1;

  for (
    int i = inicioBusquedaEco;
    i < longitud;
    i++
  ) {

    double valor =
        fabs(correlacion[i]);


    // Debe superar el umbral
    if (valor < umbralEco) {
      continue;
    }


    // Debe ser máximo local
    bool maxIzquierda =
        (i == 0) ||
        (
          valor >
          fabs(correlacion[i - 1])
        );

    bool maxDerecha =
        (i == longitud - 1) ||
        (
          valor >=
          fabs(correlacion[i + 1])
        );


    if (
      maxIzquierda &&
      maxDerecha
    ) {

      // Tomamos el PRIMER pico que supera el umbral
      // (el reflejo mas cercano en el tiempo), no el
      // de mayor amplitud en toda la ventana. Un lobulo
      // lejano mas fuerte (autocorrelacion del chirp u
      // otra reflexion) ya no reemplaza al primero.
      maxEco = valor;
      indiceEco = i;
      break;
    }
  }


  // =====================================================
  // 5. SI NO HAY ECO
  // =====================================================

  if (indiceEco == -1) {

    return resultado;
  }


  // =====================================================
  // 6. CALCULAR TIEMPO Y DISTANCIA
  // =====================================================

  resultado.encontrado = true;

  resultado.picoEco = indiceEco;
  resultado.valorEco =
      correlacion[indiceEco];


  resultado.diferenciaMuestras =
      indiceEco - indiceDirecto;


  // Tiempo de vuelo
  resultado.tiempoVuelo =
      (double)resultado.diferenciaMuestras
      / FS;


  // Velocidad del sonido aprox. 343 m/s
  const double velocidadSonido = 343.0;


  resultado.distancia =
      (
        velocidadSonido *
        resultado.tiempoVuelo
      )
      / 2.0;


  return resultado;
}

void imprimirResultadoEco(
  const char* nombre,
  ResultadoEco resultado
) {

  Serial.println();
  Serial.print("------ ");
  Serial.print(nombre);
  Serial.println(" ------");


  Serial.print("Pico directo: ");
  Serial.print(resultado.picoDirecto);

  Serial.print(" | valor: ");
  Serial.println(resultado.valorDirecto);


  if (!resultado.encontrado) {

    Serial.println(
      "No se detecto un eco valido."
    );

    return;
  }


  Serial.print("Pico eco: ");
  Serial.print(resultado.picoEco);

  Serial.print(" | valor: ");
  Serial.println(resultado.valorEco);


  Serial.print("Diferencia: ");
  Serial.print(
    resultado.diferenciaMuestras
  );

  Serial.println(" muestras");


  Serial.print("Tiempo de vuelo: ");
  Serial.print(
    resultado.tiempoVuelo * 1000.0,
    3
  );

  Serial.println(" ms");


  Serial.print("Distancia estimada: ");
  Serial.print(
    resultado.distancia * 100.0,
    2
  );

  Serial.println(" cm");
}
void imprimirCandidatosEco(
  double correlacion[],
  int longitud,
  int picoDirecto,
  double umbralRelativo = 0.20
) {

  double maxDirecto = fabs(correlacion[picoDirecto]);
  double umbral = maxDirecto * umbralRelativo;

  int muestrasMinimas =
      (int)ceil(
        (2.0 * DISTANCIA_MINIMA / VELOCIDAD_SONIDO)
        * FS
      );

  int inicio = picoDirecto + muestrasMinimas;

  Serial.println("------ CANDIDATOS ECO ------");

  for (int i = inicio; i < longitud - 1; i++) {

    double valor = fabs(correlacion[i]);

    if (valor < umbral) {
      continue;
    }

    bool maxLocal =
      valor > fabs(correlacion[i - 1]) &&
      valor >= fabs(correlacion[i + 1]);

    if (!maxLocal) {
      continue;
    }

    int diferencia = i - picoDirecto;

    double tiempo =
      (double)diferencia / FS;

    double distancia =
      (VELOCIDAD_SONIDO * tiempo) / 2.0;

    Serial.print("muestra: ");
    Serial.print(i);

    Serial.print(" | delta: ");
    Serial.print(diferencia);

    Serial.print(" | valor: ");
    Serial.print(correlacion[i]);

    Serial.print(" | distancia: ");
    Serial.print(distancia * 100.0, 2);

    Serial.println(" cm");
  }
}
void guardarCalibracionBase(int picoDirecto) {

  for (int i = 0; i < CORR_SIZE; i++) {
    correlacionBase[i] = correlacionFFT[i];
  }

  picoDirectoBase = picoDirecto;
  calibracionLista = true;

  Serial.print("Calibracion base guardada. Pico directo base: ");
  Serial.println(picoDirectoBase);
}
void calcularDiferenciaCorrelacion(int picoDirectoActual) {

  if (!calibracionLista) {
    return;
  }

  // Cuanto se desplazó la captura actual respecto
  // a la captura usada para calibración.
  int desplazamiento =
      picoDirectoActual - picoDirectoBase;

  for (int i = 0; i < CORR_SIZE; i++) {

    // Buscar el punto equivalente en la calibración
    int indiceBase = i - desplazamiento;

    if (
      indiceBase < 0 ||
      indiceBase >= CORR_SIZE
    ) {

      diferenciaCorrelacion[i] = 0.0;
      continue;
    }

    diferenciaCorrelacion[i] =
      fabs(
        fabs(correlacionFFT[i]) -
        fabs(correlacionBase[indiceBase])
      );
  }
}
int detectarEcoPorDiferencia(
  int picoDirecto
) {

  if (!calibracionLista) {
    return -1;
  }

  // -------------------------------------------------------
  // Distancia mínima permitida
  // -------------------------------------------------------

  int muestrasMinimas =
      (int)ceil(
        (2.0 * DISTANCIA_MINIMA /
         VELOCIDAD_SONIDO)
        * FS
      );

  int inicio =
      picoDirecto + muestrasMinimas;

  if (inicio >= CORR_SIZE - 1) {
    return -1;
  }


  // -------------------------------------------------------
  // 1. Buscar la mayor diferencia
  // -------------------------------------------------------

  double maximoGlobal = 0.0;

  for (int i = inicio; i < CORR_SIZE - 1; i++) {

    if (
      diferenciaCorrelacion[i] >
      maximoGlobal
    ) {

      maximoGlobal =
          diferenciaCorrelacion[i];
    }
  }

  if (maximoGlobal <= 0.0) {
    return -1;
  }


  // -------------------------------------------------------
  // 2. Umbral
  // -------------------------------------------------------

  const double umbralRelativo = 0.25;

  double umbral =
      maximoGlobal * umbralRelativo;


  // -------------------------------------------------------
  // 3. Buscar TODOS los máximos locales
  //    y conservar el más fuerte
  // -------------------------------------------------------

  double mejorValor = 0.0;
  int mejorIndice = -1;

  for (
    int i = inicio;
    i < CORR_SIZE - 1;
    i++
  ) {

    double valor =
        diferenciaCorrelacion[i];

    if (valor < umbral) {
      continue;
    }


    bool maxLocal =
        valor >
          diferenciaCorrelacion[i - 1]
        &&
        valor >=
          diferenciaCorrelacion[i + 1];


    if (!maxLocal) {
      continue;
    }


    // Ya NO retornamos el primero.
    // Buscamos el cambio más grande respecto
    // a la calibración sin objeto.
    if (valor > mejorValor) {

      mejorValor = valor;
      mejorIndice = i;
    }
  }


  return mejorIndice;
}


void imprimirCandidatosDiferencia(
  int picoDirecto
) {

  int muestrasMinimas =
      (int)ceil(
        (2.0 * DISTANCIA_MINIMA /
         VELOCIDAD_SONIDO)
        * FS
      );

  int inicio =
      picoDirecto + muestrasMinimas;


  Serial.println(
    "------ CANDIDATOS POR CALIBRACION ------"
  );


  for (
    int i = inicio;
    i < CORR_SIZE - 1;
    i++
  ) {

    double valor =
        diferenciaCorrelacion[i];

    bool maxLocal =
        valor >
          diferenciaCorrelacion[i - 1]
        &&
        valor >=
          diferenciaCorrelacion[i + 1];

    if (!maxLocal) {
      continue;
    }


    int delta =
        i - picoDirecto;

    double tiempo =
        (double)delta / FS;

    double distancia =
        VELOCIDAD_SONIDO *
        tiempo / 2.0;


    Serial.print("muestra: ");
    Serial.print(i);

    Serial.print(" | delta: ");
    Serial.print(delta);

    Serial.print(" | diferencia: ");
    Serial.print(valor);

    Serial.print(" | distancia: ");
    Serial.print(
      distancia * 100.0,
      2
    );

    Serial.println(" cm");
  }
}

void imprimirEcoPorDiferencia(
  int picoDirecto,
  int picoEco
) {

  Serial.println();

  Serial.println(
    "------ ECO POR CALIBRACION ------"
  );


  Serial.print("Pico directo: ");
  Serial.println(picoDirecto);


  if (picoEco < 0) {

    Serial.println(
      "No se detecto un eco valido."
    );

    return;
  }


  int delta =
      picoEco - picoDirecto;


  double tiempo =
      (double)delta / FS;


  double distancia =
      (
        VELOCIDAD_SONIDO *
        tiempo
      )
      / 2.0;


  Serial.print("Pico eco: ");
  Serial.println(picoEco);


  Serial.print("Delta: ");
  Serial.print(delta);

  Serial.println(
    " muestras"
  );


  Serial.print(
    "Tiempo de vuelo: "
  );

  Serial.print(
    tiempo * 1000.0,
    3
  );

  Serial.println(
    " ms"
  );


  Serial.print(
    "Distancia estimada: "
  );

  Serial.print(
    distancia * 100.0,
    2
  );

  Serial.println(
    " cm"
  );
}