const int ledPin = 3;  // pino que controla a base do BD137

void setup() {
  pinMode(ledPin, OUTPUT);
}

void loop() {
  digitalWrite(ledPin, HIGH);  // liga o LED
}
