




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

void iniciarCapturaMic() {

    indiceMic = 0;
    capturaTerminada = false;
    capturandoMic = true;
}