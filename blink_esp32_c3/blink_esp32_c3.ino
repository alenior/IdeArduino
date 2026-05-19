void setup() {
  // Configura o GPIO2 como saída
  pinMode(2, OUTPUT);
}

void loop() {
  digitalWrite(2, HIGH); // Liga o GPIO2
  delay(1000);           // Espera 1 segundo

  digitalWrite(2, LOW);  // Desliga o GPIO2
  delay(1000);           // Espera 1 segundo
}