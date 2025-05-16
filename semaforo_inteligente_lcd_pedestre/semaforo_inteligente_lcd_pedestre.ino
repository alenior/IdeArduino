#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);  // Endereço padrão do I2C LCD

// Definições de pinos com nomes únicos
const int sensorAPin = 2;
const int sensorBPin = 3;
const int sensorPedestre = A2;

const int vermelhoA = 4;
const int verdeB = 5;
const int amareloB = 6;
const int vermelhoB = 7;
const int verdePedestre = 8;
const int vermelhoPedestre = 9;
const int buzzer = 10;
const int verdeA = 11;
const int amareloA = 12;

// Variáveis de controle
volatile int fluxo_a = 0;
volatile int fluxo_b = 0;
bool estado_verdeA = false;
bool estado_verdeB = false;
int contador_verde_a = 0;
int contador_verde_b = 0;
bool tempo_fluxo_a_ativo = false;
bool tempo_fluxo_b_ativo = false;
bool pedestre_chama = false;

void atualiza_lcd_temporizado() {
  lcd.setCursor(0, 0);
  if (!pedestre_chama) {
    lcd.print(estado_verdeA ? String(contador_verde_a) : "--");
    lcd.print("|A:");
    lcd.print(fluxo_a);
    lcd.print("  ");

    lcd.setCursor(9, 0);
    lcd.print(estado_verdeB ? String(contador_verde_b) : "--");
    lcd.print("|B:");
    lcd.print(fluxo_b);
  } else {
    lcd.print(estado_verdeA ? String(contador_verde_a) : "--");
    lcd.print("|A:");
    lcd.print(fluxo_a);
    lcd.print("**");

    lcd.setCursor(9, 0);
    lcd.print(estado_verdeB ? String(contador_verde_b) : "--");
    lcd.print("|B:");
    lcd.print(fluxo_b);
  }
}

// Interrupções
void sensorA() {
  if (estado_verdeA && tempo_fluxo_a_ativo)
    fluxo_a++;
}

void sensorB() {
  if (estado_verdeB && tempo_fluxo_b_ativo)
    fluxo_b++;
}

int compara(int fluxoA, int fluxoB) {
  if (fluxoB == 0) fluxoB = 1;
  int relacao = (fluxoA * 10) / fluxoB;
  if (relacao <= 13) return 30;
  else if (relacao <= 16) return 35;
  else return 40;
}

void buzzerBeep(int tempo = 100) {
  digitalWrite(buzzer, LOW);
  delay(tempo);
  digitalWrite(buzzer, HIGH);
}

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
    lcd.print(tempo);
    lcd.print(" ----");

    lcd.setCursor(12, 0);
    if (tempo >= 7 && tempo <= 10) {
      lcd.print(animacoes[(10 - tempo) % 4]);
      buzzerBeep(500); delay(500);
    } else if (tempo >= 3 && tempo <= 6) {
      lcd.print(animacoes[(10 - tempo) % 4]);
      buzzerBeep(500); delay(500);
    } else if (tempo == 2 || tempo == 1) {
      lcd.print(animacoes[4]);
      buzzerBeep(100); delay(200);
      buzzerBeep(300); delay(400);
    } else {
      lcd.print(animacoes[5]);
      buzzerBeep(1000);
      break;
    }
  }

  digitalWrite(verdePedestre, LOW);
  digitalWrite(vermelhoPedestre, LOW);
}

void setup() {
  // Configurações de pino
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
  digitalWrite(buzzer, HIGH); // desliga buzzer inicialmente

  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.print("Inicializando...");
  delay(1000);
  lcd.clear();

  attachInterrupt(digitalPinToInterrupt(sensorAPin), sensorA, FALLING);
  attachInterrupt(digitalPinToInterrupt(sensorBPin), sensorB, FALLING);

  atualiza_lcd_temporizado();
}

void loop() {
  int tempo_total;

  pedestre_chama = !digitalRead(sensorPedestre);

  // ------- Verde A, Vermelho B -------
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
  tempo_total = compara(fluxo_a, fluxo_b);
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
  if (pedestre_chama) pedestre_passando();

  pedestre_chama = false;

  // ------- Verde B, Vermelho A -------
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
  if (pedestre_chama) pedestre_passando();
}