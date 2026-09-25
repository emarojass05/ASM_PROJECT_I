// fft.ino
// implementacion del fft radix-2 (cooley-tukey) para el esp32

#define N_FFT 1024 // debe ser potencia de 2

void fft_radix2(float* re, float* im, int n) {

  if (n <= 1) return;

  int mitad = n / 2;
  float re_par[mitad], im_par[mitad];
  float re_impar[mitad], im_impar[mitad];

  for (int i = 0; i < mitad; i++) {
    re_par[i] = re[2 * i];
    im_par[i] = im[2 * i];
    re_impar[i] = re[2 * i + 1];
    im_impar[i] = im[2 * i + 1];
  }

  fft_radix2(re_par, im_par, mitad);
  fft_radix2(re_impar, im_impar, mitad);

  for (int k = 0; k < mitad; k++) {

    float angulo = -2.0 * PI * k / n;
    float wr = cos(angulo);
    float wi = sin(angulo);

    float tr = wr * re_impar[k] - wi * im_impar[k];
    float ti = wr * im_impar[k] + wi * re_impar[k];

    re[k] = re_par[k] + tr;
    im[k] = im_par[k] + ti;
    re[k + mitad] = re_par[k] - tr;
    im[k + mitad] = im_par[k] - ti;
  }
}

void calcular_magnitudes(float* re, float* im, float* magnitud, int n) {
  for (int i = 0; i < n / 2; i++) {
    magnitud[i] = sqrt(re[i] * re[i] + im[i] * im[i]);
  }
}

float analizar_espectro() {

  // static: para que no compitan con la recursion del fft por espacio en el stack
  static float re[N_FFT];
  static float im[N_FFT];
  static float magnitud[N_FFT / 2];

  for (int i = 0; i < N_FFT; i++) {
    re[i] = (i < N_MIC) ? (float)muestrasMic[i] : 0.0;
    im[i] = 0.0;
  }

  fft_radix2(re, im, N_FFT);
  calcular_magnitudes(re, im, magnitud, N_FFT);

  int indicePico = 1;
  float valorPico = magnitud[1];

  for (int i = 2; i < N_FFT / 2; i++) {
    if (magnitud[i] > valorPico) {
      valorPico = magnitud[i];
      indicePico = i;
    }
  }

  float frecuenciaDominante = (float)indicePico * FS / N_FFT;

  Serial.print("Frecuencia dominante: ");
  Serial.print(frecuenciaDominante);
  Serial.println(" Hz");

  return frecuenciaDominante;
}