#include <Stepper.h>

// Define o número de passos para uma rotação completa
#define STEPS_PER_REV 2024

// Define as portas do ESP32 conectadas ao ULN2003A
#define IN1 25
#define IN2 26
#define IN3 27
#define IN4 14

// Define a porta do botão Enable
#define BUTTON_PIN 0

// Define a porta do buzzer
#define BUZZER_PIN 13

// Cria uma instância do motor de passo
Stepper stepper(STEPS_PER_REV, IN1, IN3, IN2, IN4);

// Variáveis para controle do botão e direção do motor
bool clockwise = true;       // Sentido inicial do motor
bool lastButtonState = HIGH; // Estado anterior do botão
bool motorDirectionChanged = false; // Controle de mudança de direção

unsigned long previousMillis = 0; // Armazena o tempo do último bip
const long interval = 500;        // Intervalo de tempo entre bips (500ms para bips claros)
bool buzzerState = false;         // Estado atual do buzzer (ligado ou desligado)

void setup() {
  // Configura a velocidade do motor (em rotações por minuto)
  stepper.setSpeed(10);

  // Configura o botão como entrada com pull-up interno
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Inicializa as portas do motor como saída
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // Configura o pino do buzzer como saída
  pinMode(BUZZER_PIN, OUTPUT);

  // Inicializa o monitor serial para depuração
  Serial.begin(9600);
  Serial.println("Sistema iniciado");
}

void loop() {
  // Lê o estado atual do botão
  bool currentButtonState = digitalRead(BUTTON_PIN);

  // Verifica se houve uma transição de HIGH para LOW (pressionado)
  if (currentButtonState == LOW && lastButtonState == HIGH) {
    // Alterna o sentido do motor
    clockwise = !clockwise;

    // Atualiza a flag de mudança
    motorDirectionChanged = true;

    // Exibe o novo sentido no monitor serial
    Serial.println(clockwise ? "Sentido horário" : "Sentido anti-horário");

    // Pequeno atraso para evitar múltiplas leituras de um único clique
    delay(50);
  }

  // Atualiza o estado anterior do botão
  lastButtonState = currentButtonState;

  // Move o motor no sentido atualizado
  if (motorDirectionChanged || currentButtonState == HIGH) {
    if (clockwise) {
      stepper.step(1); // Gira no sentido horário
      noTone(BUZZER_PIN); // Desliga o buzzer
    } else {
      stepper.step(-1); // Gira no sentido anti-horário

      // Gera bips claros enquanto o motor gira no sentido anti-horário
      unsigned long currentMillis = millis();
      if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;

        // Alterna o estado do buzzer
        buzzerState = !buzzerState;

        if (buzzerState) {
          tone(BUZZER_PIN, 1000); // Ativa o buzzer com frequência de 1000 Hz (para o bip)
        } else {
          noTone(BUZZER_PIN); // Desativa o buzzer (fim do bip)
        }
      }
    }
    motorDirectionChanged = false; // Reseta a flag após movimento
  }

  delay(2); // Ajuste para suavizar o movimento do motor
}
