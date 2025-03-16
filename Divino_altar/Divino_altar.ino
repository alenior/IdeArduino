#include <Stepper.h>
#include <FastLED.h>

// Configuração do motor 28BYJ-48
#define STEPS_PER_REV 2048
#define IN1 13
#define IN2 12
#define IN3 14
#define IN4 27
Stepper motor(STEPS_PER_REV, IN1, IN3, IN2, IN4);

// Configuração dos LEDs WS2812B com FastLED
#define LED_PIN     5    // Pino de controle
#define LED_COUNT   8    // Número de LEDs
CRGB leds[LED_COUNT];    // Array para armazenar os LEDs

void setup() {
  motor.setSpeed(5);
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, LED_COUNT);  // Configura o controle dos LEDs
  FastLED.clear();    // Limpa a memória da LED
  FastLED.show();     // Atualiza os LEDs
}

void loop() {
  motor.step(500);
  efeitoLuz();
  motor.step(-500);
  efeitoLuz();
}

// Efeito de transição entre branco e azul para TODOS os LEDs
void efeitoLuz() {
  // Transição do branco para o azul
  for (int i = 0; i <= 255; i++) {  
    for (int j = 0; j < LED_COUNT; j++) {
      leds[j] = CRGB(i, i, 255);  // Transição de branco para azul
    }
    FastLED.show();
    delay(20);
  }

  // Transição do azul para o branco
  for (int i = 255; i >= 0; i--) {  
    for (int j = 0; j < LED_COUNT; j++) {
      leds[j] = CRGB(i, i, 255);  // Transição de azul para branco
    }
    FastLED.show();
    delay(20);
  }
}
