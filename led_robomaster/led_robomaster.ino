#include <Wire.h>

const int ledPin = 3;      // pino que controla a base do BD137
unsigned long tempoLigado = 0;  // contador de tempo em segundos

void setup() {
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);  // liga o LED no início
}

void loop() {
  // LED já está ligado no setup; conta o tempo e exibe
  tempoLigado++;
  Serial.print("Led acesso! Tempo: ");
  Serial.print(tempoLigado);
  Serial.println("s");

  delay(1000);  // aguarda 1 segundo
}