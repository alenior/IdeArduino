/*
Display 7 segmentos (Entre parênteses, as portas utilizadas no site 
wokwi, devido à ausência das utilizadas no hardware real.)
*/
const int segA = 18;
const int segB = 25; // (34)
const int segC = 4;
const int segD = 22; // (35)
const int segE = 23; // (36)
const int segF = 19;
const int segG = 21;

// Semáforo 1 (veículos)
const int sem1Verde = 17;
const int sem1Amarelo = 16;
const int sem1Vermelho = 13;

// Semáforo 2 (veículos)
const int sem2Verde = 33;
const int sem2Amarelo = 32; // (12)
const int sem2Vermelho = 26;

// Semáforo pedestre
const int pedestreVerde = 14;
const int pedestreVermelho = 15;

// Buzzer
const int buzzer = 27; // (21)

const int botaoPedestre = 0; // Botão de interrupção
volatile bool pedRequest = false;

// Declaração da função handleButtonPress antes do setup
void handleButtonPress();

void setup() {
  // Configuração dos segmentos do display
  pinMode(segA, OUTPUT);
  pinMode(segB, OUTPUT);
  pinMode(segC, OUTPUT);
  pinMode(segD, OUTPUT);
  pinMode(segE, OUTPUT);
  pinMode(segF, OUTPUT);
  pinMode(segG, OUTPUT);

  // Configuração dos semáforos, buzzer e botão de interrupção.
  pinMode(sem1Vermelho, OUTPUT);
  pinMode(sem1Amarelo, OUTPUT);
  pinMode(sem1Verde, OUTPUT);
  pinMode(sem2Vermelho, OUTPUT);
  pinMode(sem2Amarelo, OUTPUT);
  pinMode(sem2Verde, OUTPUT);
  pinMode(pedestreVermelho, OUTPUT);
  pinMode(pedestreVerde, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(botaoPedestre, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(botaoPedestre), handleButtonPress, FALLING);

  // Inicia semáforos com o Semáforo 1 em verde, Semáforo 2 em vermelho e Semáforo de pedestre em vermelho.
  digitalWrite(sem1Verde, HIGH);
  digitalWrite(sem2Vermelho, HIGH);
  digitalWrite(pedestreVermelho, HIGH);

  // Inicia display de 7 segmentos (apagado).
  digitalWrite(segA, HIGH);
  digitalWrite(segB, HIGH);
  digitalWrite(segC, HIGH);
  digitalWrite(segD, HIGH);
  digitalWrite(segE, HIGH);
  digitalWrite(segF, HIGH);
  digitalWrite(segG, HIGH);
}

void loop() {
  if (pedRequest) {
    pedestrePassagem(); // Executa a função para a passagem de pedestre
    pedRequest = false; // Reseta o pedido do pedestre
  } else {
    alternanciaVeiculos(); // Alternância normal dos semáforos de veículos
  }
}

// Função para conceder passagem ao pedestre
void pedestrePassagem() {
  digitalWrite(sem1Vermelho, HIGH);
  digitalWrite(sem2Vermelho, HIGH);
  digitalWrite(pedestreVermelho, LOW);
  digitalWrite(pedestreVerde, HIGH);

  // Contagem regressiva no display e buzina
  for (int countdown = 9; countdown >= 0; countdown--) {
    exibirNumero(countdown);
    if (countdown == 0) {
      tone(buzzer, 1000, 1000); // Emite 1 bip longo
      delay(1000);
    } else if (countdown <= 4 && countdown >= 1) {
      tone(buzzer, 1000, 100); // Emite o primeiro bip breve
      delay(200);               // Espera 200 ms (intervalo entre os bipes)
      tone(buzzer, 1000, 100);  // Emite o segundo bip breve
      delay(600);               // Espera o restante do tempo até completar 1 segundo
    } else {
      tone(buzzer, 1000, 100);  // Emite um bip normal
      delay(1000);              // Espera 1 segundo
    }

  }

  // Termina o tempo de pedestre
  digitalWrite(pedestreVerde, LOW);
  digitalWrite(pedestreVermelho, HIGH);
  digitalWrite(sem1Vermelho, LOW);
  digitalWrite(segA, HIGH);
  digitalWrite(segB, HIGH);
  digitalWrite(segC, HIGH);
  digitalWrite(segD, HIGH);
  digitalWrite(segE, HIGH);
  digitalWrite(segF, HIGH);
  digitalWrite(segG, HIGH);
}

// Função para exibir número no display de 7 segmentos
void exibirNumero(int num) {
  // Zera todos os segmentos antes de exibir o novo número
  digitalWrite(segA, HIGH);
  digitalWrite(segB, HIGH);
  digitalWrite(segC, HIGH);
  digitalWrite(segD, HIGH);
  digitalWrite(segE, HIGH);
  digitalWrite(segF, HIGH);
  digitalWrite(segG, HIGH);

  switch (num) {
    case 0:
      digitalWrite(segA, LOW); digitalWrite(segB, LOW); digitalWrite(segC, LOW);
      digitalWrite(segD, LOW); digitalWrite(segE, LOW); digitalWrite(segF, LOW);
      break;
    case 1:
      digitalWrite(segB, LOW); digitalWrite(segC, LOW);
      break;
    case 2:
      digitalWrite(segA, LOW); digitalWrite(segB, LOW); digitalWrite(segD, LOW);
      digitalWrite(segE, LOW); digitalWrite(segG, LOW);
      break;
    case 3:
      digitalWrite(segA, LOW); digitalWrite(segB, LOW); digitalWrite(segC, LOW);
      digitalWrite(segD, LOW); digitalWrite(segG, LOW);
      break;
    case 4:
      digitalWrite(segB, LOW); digitalWrite(segC, LOW); digitalWrite(segF, LOW);
      digitalWrite(segG, LOW);
      break;
    case 5:
      digitalWrite(segA, LOW); digitalWrite(segC, LOW); digitalWrite(segD, LOW);
      digitalWrite(segF, LOW); digitalWrite(segG, LOW);
      break;
    case 6:
      digitalWrite(segA, LOW); digitalWrite(segC, LOW); digitalWrite(segD, LOW);
      digitalWrite(segE, LOW); digitalWrite(segF, LOW); digitalWrite(segG, LOW);
      break;
    case 7:
      digitalWrite(segA, LOW); digitalWrite(segB, LOW); digitalWrite(segC, LOW);
      break;
    case 8:
      digitalWrite(segA, LOW); digitalWrite(segB, LOW); digitalWrite(segC, LOW);
      digitalWrite(segD, LOW); digitalWrite(segE, LOW); digitalWrite(segF, LOW);
      digitalWrite(segG, LOW);
      break;
    case 9:
      digitalWrite(segA, LOW); digitalWrite(segB, LOW); digitalWrite(segC, LOW);
      digitalWrite(segF, LOW); digitalWrite(segG, LOW);
      break;
  }
  delay(1000); // Tempo entre a exibição dos números
}

// Função para alternância dos semáforos de veículos
void alternanciaVeiculos() {
  digitalWrite(sem1Verde, HIGH);
  delay(5000);
  digitalWrite(sem1Verde, LOW);
  
  digitalWrite(sem1Amarelo, HIGH);
  delay(2000);
  digitalWrite(sem1Amarelo, LOW);
  
  digitalWrite(sem1Vermelho, HIGH);

  delay(1000); 
  digitalWrite(sem2Vermelho, LOW);
  digitalWrite(sem2Verde, HIGH);
  delay(5000);
  digitalWrite(sem2Verde, LOW);
  
  digitalWrite(sem2Amarelo, HIGH);
  delay(2000);
  digitalWrite(sem2Amarelo, LOW);

  digitalWrite(sem2Vermelho, HIGH);
  delay(1000); 
  digitalWrite(sem1Vermelho, LOW); 
}

void IRAM_ATTR handleButtonPress() {
  pedRequest = true;
}
