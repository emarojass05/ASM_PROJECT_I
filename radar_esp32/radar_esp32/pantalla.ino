// pantalla.ino
// muestra los resultados del radar en una pantalla lcd i2c (16x2)

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define DIRECCION_LCD 0x27 // si no muestra nada, probar con 0x3F

LiquidCrystal_I2C lcd(DIRECCION_LCD, 16, 2);

void iniciarPantalla() {
  lcd.init();
  lcd.backlight();
  lcd.clear();
}

void mostrarResultado(float distancia_cm, float frecuencia) {

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Dist: ");
  lcd.print(distancia_cm, 1);
  lcd.print(" cm");

  lcd.setCursor(0, 1);
  lcd.print("F: ");
  lcd.print((int)frecuencia);
  lcd.print(" Hz");
}

void mostrarSinEco() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("No se detecto");
  lcd.setCursor(0, 1);
  lcd.print("eco");
}