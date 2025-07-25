#include <Ultrasonic.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Pinos TRIG e ECHO
Ultrasonic ultrasonic(9, 10);

// Pino do buzzer
const int buzzer = 11;

// Endereço I2C do display LCD (geralmente 0x27 ou 0x3F)
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  Serial.begin(9600);
  pinMode(buzzer, OUTPUT);

  // Inicializa o LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Medindo...");

  Serial.println("Iniciando medição de distância...");
}

void loop() {
  long distancia = ultrasonic.read(); // em cm

  // Exibe na serial
  Serial.print("Distância: ");
  Serial.print(distancia);
  Serial.println(" cm");

  // Exibe no LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Distancia:");
  lcd.setCursor(0, 1);
  lcd.print(distancia);
  lcd.print(" cm");

  // Sinal sonoro por faixa de distância
  if (distancia > 350) {
    tone(buzzer, 400, 200);
    delay(800);
  }
  else if (distancia > 250) {
    tone(buzzer, 600, 150);
    delay(300);
    noTone(buzzer);
    delay(500);
  }
  else if (distancia > 150) {
    tone(buzzer, 800, 100);
    delay(200);
    noTone(buzzer);
    delay(300);
  }
  else if (distancia > 50) {
    tone(buzzer, 1000, 100);
    delay(150);
    noTone(buzzer);
    delay(200);
  }
  else {
    tone(buzzer, 1200, 100);
    delay(100);
    noTone(buzzer);
    delay(100);
  }

  delay(1000); // Tempo total entre leituras
}
