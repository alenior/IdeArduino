// Definindo os pinos para os LEDs do Semáforo 1 (Horizontal)
const int redLED1 = 15;
const int yellowLED1 = 2;
const int greenLED1 = 4;

// Definindo os pinos para os LEDs do Semáforo 2 (Vertical)
const int redLED2 = 1;
const int yellowLED2 = 22;
const int greenLED2 = 23;

void setup() {
  // Configura os pinos dos LEDs como saídas
  pinMode(redLED1, OUTPUT);
  pinMode(yellowLED1, OUTPUT);
  pinMode(greenLED1, OUTPUT);
  
  pinMode(redLED2, OUTPUT);
  pinMode(yellowLED2, OUTPUT);
  pinMode(greenLED2, OUTPUT);
}

void loop() {
  // Semáforo 1 (Horizontal) verde e Semáforo 2 (Vertical) vermelho
  digitalWrite(greenLED1, HIGH);
  digitalWrite(redLED2, HIGH);
  delay(5000); // 5 segundos para o verde horizontal
  
  // Troca para amarelo no Semáforo 1 (Horizontal)
  digitalWrite(greenLED1, LOW);
  digitalWrite(yellowLED1, HIGH);
  delay(2000); // 2 segundos para o amarelo horizontal
  
  // Semáforo 1 (Horizontal) vermelho e Semáforo 2 (Vertical) verde
  digitalWrite(yellowLED1, LOW);
  digitalWrite(redLED1, HIGH);
  digitalWrite(redLED2, LOW);
  digitalWrite(greenLED2, HIGH);
  delay(5000); // 5 segundos para o verde vertical
  
  // Troca para amarelo no Semáforo 2 (Vertical)
  digitalWrite(greenLED2, LOW);
  digitalWrite(yellowLED2, HIGH);
  delay(2000); // 2 segundos para o amarelo vertical
  
  // Reset para reiniciar o ciclo
  digitalWrite(yellowLED2, LOW);
  digitalWrite(redLED2, HIGH);
  digitalWrite(redLED1, LOW);
}
