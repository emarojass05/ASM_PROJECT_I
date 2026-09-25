// captura_mic.ino
// captura de audio del microfono via adc para la deteccion de ecos

#include <driver/adc.h>

#define DURACION_CAPTURA 0.010 // segundos; alcanza para cubrir bien hasta 60cm (con margen), ya no se necesitan varios metros
const int N_MIC = FS * DURACION_CAPTURA; // numero de muestras a capturar

volatile uint16_t muestrasMic[N_MIC];
volatile int indiceMic = 0;
volatile bool capturandoMic = false;
volatile bool capturaTerminada = false;

hw_timer_t *timerMic = NULL;

void configurar_adc() {
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11); // GPIO34
}

void iniciarCapturaMic() {
  indiceMic = 0;
  capturaTerminada = false;
  capturandoMic = true;
}

void ARDUINO_ISR_ATTR tomarMuestraMic() {

  if (!capturandoMic) {
    return;
  }

  if (indiceMic < N_MIC) {

    muestrasMic[indiceMic] = adc1_get_raw(ADC1_CHANNEL_6);
    indiceMic++;

  } else {

    capturandoMic = false;
    capturaTerminada = true;
  }
}
