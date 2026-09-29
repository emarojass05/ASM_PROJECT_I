#include <math.h>

// =========================================================
// PINES
// =========================================================

const int MIC = 34;
const int SPEAKER = 25;
const int LCD_SDA = 21;
const int LCD_SCL = 22;


// =========================================================
// CONSTANTES
// =========================================================

#define FS 24000

#define FFT_SIZE 256
#define CAPTURA_SIZE 256
#define CHIRP_SIZE 48

#define CORR_SIZE (CAPTURA_SIZE - CHIRP_SIZE + 1)
#define CORR_FFT_SIZE 512


// =========================================================
// TIMER
// =========================================================

hw_timer_t *timerChirp = NULL;

unsigned long tiempoAnterior = 0;

const unsigned long intervalo = 1000;


// =========================================================
// RESULTADO DEL ECO
// =========================================================

struct ResultadoEco {

  bool encontrado;

  int picoDirecto;
  int picoEco;

  double valorDirecto;
  double valorEco;

  int diferenciaMuestras;

  double tiempoVuelo;
  double distancia;
};


// =========================================================
// VARIABLES EXTERNAS
// =========================================================

extern double correlacionDirecta[CORR_SIZE];
extern double correlacionFFT[CORR_SIZE];

extern bool calibracionLista;


// =========================================================
// PROTOTIPOS MICROFONO
// =========================================================

void iniciarMicrofono();

void capturarMicrofono();

void centrarMuestras();


// =========================================================
// PROTOTIPOS CHIRP
// =========================================================

void generar_chirp();

void reproducir_chirp();

void ARDUINO_ISR_ATTR siguienteMuestra();


// =========================================================
// PROTOTIPOS FFT
// =========================================================

void calcularFFT();

void imprimirBandaChirp();


// =========================================================
// PROTOTIPOS CORRELACION
// =========================================================

void prepararChirpCorrelacion();

void calcularCorrelacionDirecta();

void calcularCorrelacionFFT();


ResultadoEco detectarEco(
  double correlacion[],
  int longitud,
  double umbralEcoRelativo
);


void imprimirResultadoEco(
  const char* nombre,
  ResultadoEco resultado
);


void imprimirCandidatosEco(
  double correlacion[],
  int longitud,
  int picoDirecto,
  double umbralRelativo
);


// =========================================================
// PROTOTIPOS CALIBRACION
// =========================================================

void guardarCalibracionBase(
  int picoDirecto
);


void calcularDiferenciaCorrelacion(
  int picoDirectoActual
);


int detectarEcoPorDiferencia(
  int picoDirecto
);


void imprimirEcoPorDiferencia(
  int picoDirecto,
  int picoEco
);


void imprimirCandidatosDiferencia(
  int picoDirecto
);


// =========================================================
// SETUP
// =========================================================

void setup() {

  Serial.begin(115200);


  // -------------------------------------------------------
  // Inicializar microfono
  // -------------------------------------------------------

  iniciarMicrofono();


  // -------------------------------------------------------
  // Generar chirp
  // -------------------------------------------------------

  generar_chirp();


  // Preparar referencia centrada del chirp
  prepararChirpCorrelacion();


  // -------------------------------------------------------
  // Timer del chirp
  // -------------------------------------------------------

  timerChirp = timerBegin(FS);

  timerAttachInterrupt(
    timerChirp,
    &siguienteMuestra
  );

  timerAlarm(
    timerChirp,
    1,
    true,
    0
  );


  // -------------------------------------------------------
  // Mensaje inicial
  // -------------------------------------------------------

  Serial.println();

  Serial.println(
    "======================================"
  );

  Serial.println(
    "RADAR INICIADO"
  );

  Serial.println(
    "Primera captura: mantener SIN OBJETO"
  );

  Serial.println(
    "para realizar la calibracion."
  );

  Serial.println(
    "======================================"
  );

  Serial.println();
}


// =========================================================
// LOOP
// =========================================================

void loop() {

  unsigned long tiempoActual =
      millis();


  if (
    tiempoActual - tiempoAnterior
    >= intervalo
  ) {

    tiempoAnterior = tiempoActual;


    // =====================================================
    // 1. EMITIR CHIRP
    // =====================================================

    reproducir_chirp();


    // =====================================================
    // 2. CAPTURAR MICROFONO
    // =====================================================

    capturarMicrofono();


    // =====================================================
    // 3. ELIMINAR COMPONENTE DC
    // =====================================================

    centrarMuestras();


    // =====================================================
    // 4. FFT ESPECTRAL
    // =====================================================

    calcularFFT();

    imprimirBandaChirp();


    // =====================================================
    // 5. CORRELACION DIRECTA
    // =====================================================

    calcularCorrelacionDirecta();

    ResultadoEco ecoDirecta =
        detectarEco(
          correlacionDirecta,
          CORR_SIZE,
          0.25
        );


    // =====================================================
    // 6. CORRELACION MEDIANTE FFT
    // =====================================================

    calcularCorrelacionFFT();

    ResultadoEco ecoFFT =
        detectarEco(
          correlacionFFT,
          CORR_SIZE,
          0.25
        );


    // =====================================================
    // 7. PRIMERA CAPTURA = CALIBRACION SIN OBJETO
    // =====================================================

    if (!calibracionLista) {

      guardarCalibracionBase(
        ecoFFT.picoDirecto
      );

      Serial.println();

      Serial.println(
        "======================================"
      );

      Serial.println(
        "CALIBRACION TERMINADA"
      );

      Serial.println(
        "Ahora puede colocar el objeto."
      );

      Serial.println(
        "======================================"
      );

      Serial.println();

      return;
    }


    // =====================================================
    // 8. RESULTADO CORRELACION DIRECTA
    // =====================================================

    imprimirResultadoEco(
      "CORRELACION DIRECTA",
      ecoDirecta
    );


    // =====================================================
    // 9. RESULTADO CORRELACION FFT
    // =====================================================

    imprimirResultadoEco(
      "CORRELACION FFT",
      ecoFFT
    );


    // =====================================================
    // 10. CANDIDATOS DE LA CORRELACION
    // =====================================================

    imprimirCandidatosEco(
      correlacionFFT,
      CORR_SIZE,
      ecoFFT.picoDirecto,
      0.20
    );


    // =====================================================
    // 11. COMPARAR CONTRA CALIBRACION SIN OBJETO
    // =====================================================

    calcularDiferenciaCorrelacion(
      ecoFFT.picoDirecto
    );
    


    // =====================================================
    // 12. MOSTRAR CANDIDATOS DESPUES DE CALIBRACION
    // =====================================================

    imprimirCandidatosDiferencia(
      ecoFFT.picoDirecto
    );


    // =====================================================
    // 13. DETECTAR ECO MEDIANTE DIFERENCIA
    // =====================================================

    int picoEcoCalibrado =
        detectarEcoPorDiferencia(
          ecoFFT.picoDirecto
        );


    // =====================================================
    // 14. CALCULAR Y MOSTRAR DISTANCIA CALIBRADA
    // =====================================================

    imprimirEcoPorDiferencia(
      ecoFFT.picoDirecto,
      picoEcoCalibrado
    );


    // =====================================================
    // 15. SEPARADOR
    // =====================================================

    Serial.println();

    Serial.println(
      "========================================"
    );

    Serial.println();
  }
}