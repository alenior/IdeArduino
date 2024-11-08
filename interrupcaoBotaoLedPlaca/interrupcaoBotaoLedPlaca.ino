// Definindo as constantes dos pinos que serão utilizados
const int INTERRUPT_PIN = 0; // pino do botão
const int LED_PIN = 2; // pino do LED
const int SERIAL_BAUD_RATE = 115200; // taxa de transmissão serial

// Definindo a variável contador e inicializando em 0
volatile int contador = 0;
volatile bool interrupcao_ocorreu = false;

// Função de interrupção para o botão
void handleInterrupt() {
  if(!interrupcao_ocorreu) { // se a interrupção ainda não ocorreu
    interrupcao_ocorreu = true; // marca que a interrupção ocorreu
    contador = contador + 1; // incrementa o contador
    digitalWrite(LED_PIN, !digitalRead(LED_PIN)); // inverte o nível lógico do pino 2
    Serial.println(contador); // envia o valor do contador pelo monitor serial
  }
}

void setup() {
  // Configurando o pino do botão como entrada e ativa a pull-up interna
  pinMode(INTERRUPT_PIN, INPUT_PULLUP);

  // Configurando o pino do LED como saída e desliga o LED
  pinMode(LED_PIN, OUTPUT);
  pinMode(2, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Iniciando a comunicação serial
  Serial.begin(SERIAL_BAUD_RATE);
  Serial.println("Hello, ESP32!");

  // Configurando a interrupção externa no pino do botão
  attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), handleInterrupt, FALLING);
}

void loop() {
  if(interrupcao_ocorreu) { // se a interrupção ocorreu
    delay(50); // aguarda 50ms para evitar rebote do botão
    interrupcao_ocorreu = false; // marca que a interrupção foi tratada
  }
  
  digitalWrite(LED_PIN, !digitalRead(LED_PIN));
  //digitalWrite(2, HIGH);
  //delay(2000);
  //digitalWrite(2, LOW);
  delay(1000);
}