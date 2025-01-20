#include <Wire.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>
#include <Keypad.h>
#include <MFRC522.h>

// Configurações de hardware
#define RST_PIN 22
#define SS_PIN 21
MFRC522 rfid(SS_PIN, RST_PIN);

hd44780_I2Cexp lcd(0x27, 16, 2);

const byte ROWS = 4;
const byte COLS = 3;
char keys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};
byte rowPins[ROWS] = {32, 33, 25, 26};
byte colPins[COLS] = {27, 14, 12};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

const int ledRed = 2;
const int ledYellow = 5;
const int ledGreen = 16;
const int buzzer = 17;
const int relayPin = 13;

// Senha padrão
String senha = "123456";
String entradaSenha = "";

// Funções auxiliares
void acessoNegado() {
  lcd.clear();
  lcd.print("Senha errada");
  lcd.setCursor(0, 1);
  lcd.print("Não autorizado!");
  digitalWrite(buzzer, HIGH);
  delay(500);
  digitalWrite(buzzer, LOW);
  delay(250);
  digitalWrite(buzzer, HIGH);
  delay(500);
  digitalWrite(buzzer, LOW);

  Serial.println("Senha errada! Acesso não autorizado!");

  delay(2000);
  lcd.print("Acesso restrito");
  lcd.setCursor(0, 1);
  lcd.print("Credencial?");

  Serial.println("Sistema ativo: aguardando tentativa de acesso.");
}

void acessoLiberado() {
  lcd.clear();
  lcd.print("Acesso liberado");
  lcd.setCursor(0, 1);
  lcd.print("Bem vindo!");

  Serial.println("Acesso autorizado.");

  digitalWrite(ledRed, LOW);
  digitalWrite(ledYellow, LOW);
  digitalWrite(ledGreen, HIGH);
  digitalWrite(relayPin, HIGH);
  digitalWrite(buzzer, HIGH);
  delay(100);
  digitalWrite(buzzer, LOW);
  delay(3000);
  digitalWrite(relayPin, LOW);
  digitalWrite(ledGreen, LOW);
  digitalWrite(ledRed, HIGH);
  lcd.clear();
  lcd.print("Acesso restrito");
  lcd.setCursor(0, 1);
  lcd.print("Credencial?");

  Serial.println("Sistema ativo: aguardando tentativa de acesso.");
}

void setup() {
  // Configuração inicial
  Wire.begin(4, 15); // Inicializa I2C no LCD, com portas diferentes das padrões (21 e 22).
  lcd.begin(16, 2);
  lcd.backlight();
  lcd.print("Acesso restrito!");
  lcd.setCursor(0, 1);
  lcd.print("Credencial?");

  pinMode(ledRed, OUTPUT);
  pinMode(ledYellow, OUTPUT);
  pinMode(ledGreen, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(relayPin, OUTPUT);

  digitalWrite(ledRed, HIGH);

  SPI.begin();
  rfid.PCD_Init();

  Serial.begin(9600);
}

void loop() {
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return;
  }

  lcd.clear();
  lcd.print("Tag detectada.");
  lcd.setCursor(0, 1);
  lcd.print("Exige senha!");
  digitalWrite(ledYellow, HIGH);
  delay(1000);

  Serial.println("Tag detectada: exige senha.");

  entradaSenha = "";
  lcd.clear();
  lcd.print("Informe a senha:");
  lcd.setCursor(0, 1);

  Serial.print("Senha informada: ");

  while (entradaSenha.length() < 6) {
    lcd.cursor();
    char key = keypad.getKey();
    Serial.print(key);
    if (key) {
      entradaSenha += key;
      lcd.print("*");
      digitalWrite(buzzer, HIGH);
      delay(250);
      digitalWrite(buzzer, LOW);
      delay(250);
    }
  }

  Serial.println("");

  if (entradaSenha == senha) {
    acessoLiberado();
  } else {
    acessoNegado();
  }

  rfid.PICC_HaltA();
  digitalWrite(ledYellow, LOW);
}
