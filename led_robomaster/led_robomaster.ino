const int ledPin = 3;  // pino que controla a base do BD137

void setup() {
  pinMode(ledPin, OUTPUT);
}

void loop() {
  digitalWrite(ledPin, HIGH);  // liga o LED
  delay(1000);                 // espera 1 segundo
  digitalWrite(ledPin, LOW);   // desliga o LED
  delay(1000);                 // espera 1 segundo
}
