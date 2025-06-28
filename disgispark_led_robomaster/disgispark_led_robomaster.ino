// Digispark (ATtiny85) - controla a saída PWM pra LED

void setup() {
  pinMode(0, OUTPUT); // PWM output
}

void loop() {
  // Aumenta gradualmente o brilho
  for(int i = 0; i <= 255; i++) {
    analogWrite(0, i);
    delay(10);
  }
  
  // Diminui gradualmente
  for(int i = 255; i >= 0; i--) {
    analogWrite(0, i);
    delay(10);
  }
}
