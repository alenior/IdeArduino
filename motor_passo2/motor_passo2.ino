// Define as portas do ESP32 conectadas ao ULN2003A
#define IN1 25
#define IN2 26
#define IN3 27
#define IN4 14

// Define a porta do botão Enable
#define BUTTON_PIN 0

// Variáveis para o controle do motor
int stepsPerRevolution = 2048; // Número de passos para uma rotação completa
int stepDelay = 2;             // Tempo entre passos (ms)
int currentStep = 0;           // Passo atual
bool clockwise = true;         // Direção inicial: Horário

// Sequência de ativação das bobinas do motor
int stepSequence[8][4] = {
  {1, 0, 0, 0},
  {1, 1, 0, 0},
  {0, 1, 0, 0},
  {0, 1, 1, 0},
  {0, 0, 1, 0},
  {0, 0, 1, 1},
  {0, 0, 0, 1},
  {1, 0, 0, 1}
};

void setup() {
  // Configura as portas do motor como saída
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // Configura o botão como entrada com pull-up interno
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Inicializa as bobinas como desligadas
  stopMotor();
}

void loop() {
  // Verifica o estado do botão
  if (digitalRead(BUTTON_PIN) == LOW) {
    // Botão pressionado: mudar para anti-horário
    clockwise = false;
  } else {
    // Botão não pressionado: girar no sentido horário
    clockwise = true;
  }

  // Realiza um passo do motor
  stepMotor(clockwise);
  delay(stepDelay);
}

// Função para realizar um passo no motor
void stepMotor(bool clockwise) {
  if (clockwise) {
    currentStep = (currentStep + 1) % 8; // Avança na sequência
  } else {
    currentStep = (currentStep - 1 + 8) % 8; // Retrocede na sequência
  }

  // Ativa as bobinas conforme a sequência
  digitalWrite(IN1, stepSequence[currentStep][0]);
  digitalWrite(IN2, stepSequence[currentStep][1]);
  digitalWrite(IN3, stepSequence[currentStep][2]);
  digitalWrite(IN4, stepSequence[currentStep][3]);
}

// Função para desligar todas as bobinas
void stopMotor() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}
