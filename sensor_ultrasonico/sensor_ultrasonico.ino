#include <Ultrasonic.h>

// Define os pinos TRIG e ECHO
Ultrasonic ultrasonic(9, 10); // TRIG = 9, ECHO = 10

void setup() {
  Serial.begin(9600);
  Serial.println("Iniciando medição de distância...");
}

void loop() {
  long distancia = ultrasonic.read(); // Retorna distância em cm
  Serial.print("Distância: ");
  Serial.print(distancia);
  Serial.println(" cm");

  delay(500); // Aguarda meio segundo antes da próxima medição
}
