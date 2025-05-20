#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Inicializa o LCD no endereço 0x27 com 16 colunas e 2 linhas
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Definições de pinos
const int sensorAPin = 2;
const int sensorBPin = 15;
const int sensorPedestre = 4;

const int vermelhoA = 5;
const int verdeB = 18;
const int amareloB = 19;
const int vermelhoB = 32;         // <-- alterado de 21 para 32
const int verdePedestre = 33;     // <-- alterado de 22 para 33
const int vermelhoPedestre = 23;
const int buzzer = 25;
const int verdeA = 26;
const int amareloA = 27;

// Variáveis de controle
volatile int fluxo_a = 0;
volatile int fluxo_b = 0;

bool estado_verdeA = false;
bool estado_verdeB = false;
int contador_verde_a = 0;
int contador_verde_b = 0;

bool tempo_fluxo_a_ativo = false;
bool tempo_fluxo_b_ativo = false;
volatile bool pedestre_chama = false;

volatile unsigned long ultima_interrupt_a = 0;
volatile unsigned long ultima_interrupt_b = 0;
const unsigned long intervalo_debounce = 200; // 200ms

// Atualiza LCD
void atualiza_lcd_temporizado() {
  lcd.setCursor(0, 0);
  if (contador_verde_a > 0 && contador_verde_a < 10) lcd.print(" ");
  lcd.print(estado_verdeA ? String(contador_verde_a) : "--");
  lcd.print("|A:");
  lcd.print(fluxo_a);
  lcd.print(" ");

  lcd.setCursor(9, 0);
  if (contador_verde_b > 0 && contador_verde_b < 10) lcd.print(" ");
  lcd.print(estado_verdeB ? String(contador_verde_b) : "--");
  lcd.print("|B:");
  lcd.print(fluxo_b);
  lcd.print(" ");

  lcd.setCursor(0, 1);
  if (pedestre_chama) {
    lcd.print("PEDESTRE?: SIM");
  } else {
    lcd.print("PEDESTRE?:    ");
  }
}

// Contadores via interrupção
void IRAM_ATTR sensorA_ISR() {
  unsigned long agora = millis();
  if (estado_verdeA && tempo_fluxo_a_ativo && (agora - ultima_interrupt_a > intervalo_debounce)) {
    fluxo_a++;
    ultima_interrupt_a = agora;
  }
}

void IRAM_ATTR sensorB_ISR() {
  unsigned long agora = millis();
  if (estado_verdeB && tempo_fluxo_b_ativo && (agora - ultima_interrupt_b > intervalo_debounce)) {
    fluxo_b++;
    ultima_interrupt_b = agora;
  }
}

void IRAM_ATTR pedestre_ISR() {
  if (pedestre_chama == false) {
    pedestre_chama = true;
  }
  else pedestre_chama = false;
}

// Compara fluxo de A e B para definir tempo de verde
int compara(int fluxoA, int fluxoB) {
  if (fluxoB == 0) fluxoB = 1;
  int relacao = (fluxoA * 10) / fluxoB;
  if (relacao <= 13) return 30;
  else if (relacao <= 16) return 35;
  else return 40;
}

// Beep do buzzer
void buzzerBeep(int tempo = 300) {
  tone(buzzer, 1000);
  delay(tempo);
  noTone(buzzer);
}

// Animação e tempo de travessia do pedestre
void pedestre_passando() {
  const char* animacoes[] = { " >---", " ->--", " -->-", " --->", " !!!!", " XXXX" };

  digitalWrite(vermelhoA, HIGH);
  digitalWrite(vermelhoB, HIGH);
  digitalWrite(vermelhoPedestre, HIGH);
  digitalWrite(verdePedestre, HIGH);

  lcd.clear();

  for (int tempo = 10; tempo >= 0; tempo--) {
    lcd.setCursor(0, 0);
    lcd.print("FECHA EM ");
    if (tempo < 10) lcd.print(" ");
    lcd.print(tempo);
    if (tempo == 10) lcd.print(" ----");

    lcd.setCursor(11, 0);
    if (tempo >= 3 && tempo <= 10) {
      lcd.print(animacoes[(10 - tempo) % 4]);
      buzzerBeep(500);
      delay(500);
    } else if (tempo == 2 || tempo == 1) {
      lcd.print(animacoes[4]);
      buzzerBeep(200);
      buzzerBeep(200);
      delay(600);
    } else {
      lcd.print(animacoes[5]);
      buzzerBeep(1000);
      break;
    }
  }
  lcd.clear();

  digitalWrite(verdePedestre, LOW);
  digitalWrite(vermelhoPedestre, HIGH);
}

// Setup inicial
void setup() {
  pinMode(sensorAPin, INPUT_PULLUP);
  pinMode(sensorBPin, INPUT_PULLUP);
  pinMode(sensorPedestre, INPUT_PULLUP);

  pinMode(vermelhoA, OUTPUT);
  pinMode(amareloA, OUTPUT);
  pinMode(verdeA, OUTPUT);
  pinMode(vermelhoB, OUTPUT);
  pinMode(amareloB, OUTPUT);
  pinMode(verdeB, OUTPUT);
  pinMode(vermelhoPedestre, OUTPUT);
  pinMode(verdePedestre, OUTPUT);
  pinMode(buzzer, OUTPUT);

  noTone(buzzer);

  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.print("Inicializando");
  lcd.setCursor(0, 1);
  lcd.print("Aguarde...");
  delay(1000);
  lcd.clear();

  attachInterrupt(digitalPinToInterrupt(sensorAPin), sensorA_ISR, FALLING);
  attachInterrupt(digitalPinToInterrupt(sensorBPin), sensorB_ISR, FALLING);
  attachInterrupt(digitalPinToInterrupt(sensorPedestre), pedestre_ISR, FALLING);

  atualiza_lcd_temporizado();
}

// Loop principal
void loop() {
  // Verde A
  digitalWrite(verdeA, HIGH);
  digitalWrite(vermelhoB, HIGH);
  digitalWrite(vermelhoA, LOW);
  digitalWrite(amareloA, LOW);
  digitalWrite(verdeB, LOW);
  digitalWrite(amareloB, LOW);
  digitalWrite(vermelhoPedestre, HIGH);
  digitalWrite(verdePedestre, LOW);

  estado_verdeA = true;
  estado_verdeB = false;
  contador_verde_a = 0;
  tempo_fluxo_a_ativo = true;
  buzzerBeep();

  for (int i = 0; i < 15; i++) {
    delay(1000);
    contador_verde_a++;
    atualiza_lcd_temporizado();
  }

  tempo_fluxo_a_ativo = false;
  int tempo_total = compara(fluxo_a, fluxo_b);
  for (int i = 15; i < tempo_total; i++) {
    delay(1000);
    contador_verde_a++;
    atualiza_lcd_temporizado();
  }

  estado_verdeA = false;
  digitalWrite(verdeA, LOW);
  digitalWrite(amareloA, HIGH);
  delay(3000);
  digitalWrite(amareloA, LOW);
  fluxo_b = 0;

  if (pedestre_chama) {
    pedestre_passando();
    pedestre_chama = false;
  }

  // Verde B
  digitalWrite(verdeB, HIGH);
  digitalWrite(vermelhoA, HIGH);
  digitalWrite(vermelhoB, LOW);
  digitalWrite(amareloB, LOW);
  digitalWrite(verdeA, LOW);
  digitalWrite(amareloA, LOW);
  digitalWrite(vermelhoPedestre, HIGH);
  digitalWrite(verdePedestre, LOW);

  estado_verdeB = true;
  contador_verde_b = 0;
  tempo_fluxo_b_ativo = true;
  buzzerBeep();

  for (int i = 0; i < 15; i++) {
    delay(1000);
    contador_verde_b++;
    atualiza_lcd_temporizado();
  }

  tempo_fluxo_b_ativo = false;
  tempo_total = compara(fluxo_b, fluxo_a);
  for (int i = 15; i < tempo_total; i++) {
    delay(1000);
    contador_verde_b++;
    atualiza_lcd_temporizado();
  }

  estado_verdeB = false;
  digitalWrite(verdeB, LOW);
  digitalWrite(amareloB, HIGH);
  delay(3000);
  digitalWrite(amareloB, LOW);
  fluxo_a = 0;

  if (pedestre_chama) {
    pedestre_passando();
    pedestre_chama = false;
  }
}