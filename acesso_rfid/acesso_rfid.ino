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
bool senhaDigitada = false;

// Senha padrão
String senha = "123456";
String entradaSenha = "";

// Lista de IDs de cartões autorizados
const int numCartoesAutorizados = 2;
String cartoesAutorizados[numCartoesAutorizados] = {
  "52 7D 83 54",  // ID do cartão
  // "9F 18 32 28"   // ID da tag
};

// Funções auxiliares
void acessoNegado() {
  lcd.clear();
  lcd.noCursor();
  lcd.print("Senha errada!");
  lcd.setCursor(0, 1);
  lcd.print("Acesso negado!");
  digitalWrite(buzzer, HIGH);
  delay(500);
  digitalWrite(buzzer, LOW);
  delay(250);
  digitalWrite(buzzer, HIGH);
  delay(500);
  digitalWrite(buzzer, LOW);

  Serial.println("Senha errada! Acesso não autorizado!");

  delay(2000);
  lcd.clear();
  lcd.print("Acesso restrito!");
  lcd.setCursor(0, 1);
  lcd.print("Credencial?");

  Serial.println("Sistema ativo: aguardando tentativa de acesso.");
}

void acessoLiberado() {
  lcd.clear();
  lcd.noCursor();
  lcd.print("Acesso liberado.");
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
  lcd.print("Acesso restrito!");
  lcd.setCursor(0, 1);
  lcd.print("Credencial?");

  Serial.println("Sistema ativo: aguardando tentativa de acesso.");
}

bool verificarCartaoAutorizado(String idCartao) {
  for (int i = 0; i < numCartoesAutorizados; i++) {
    if (cartoesAutorizados[i] == idCartao) {
      return true;
    }
  }
  return false;
}

void setup() {
  // Configuração inicial
  Wire.begin(4, 15); // Inicializa I2C no LCD, com portas diferentes das padrões (21 e 22).
  lcd.begin(16, 2);
  lcd.backlight();
  lcd.clear();
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
  Serial.println("Sistema ativo: aguardando tentativa de acesso.");
}

void loop() {
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return;
  }

  String idCartao = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    idCartao += String(rfid.uid.uidByte[i], HEX);
    if (i < rfid.uid.size - 1) idCartao += " ";
  }
  idCartao.toUpperCase();

  Serial.print("Cartão detectado: ");
  Serial.println(idCartao);

  if (verificarCartaoAutorizado(idCartao)) {
    acessoLiberado(); // Libera acesso sem senha para cartões autorizados
  } else {
    lcd.clear();
    lcd.print("Tag detectada.");
    lcd.setCursor(0, 1);
    lcd.print("Exige senha!");

    digitalWrite(ledYellow, HIGH);
    delay(2000);

    entradaSenha = "";
    lcd.clear();
    lcd.print("Informe senha:");
    lcd.setCursor(0, 1);

    while (true) {
      lcd.cursor();
      char key = keypad.getKey();

      if (key) {
        if (key == '*') {
          Serial.println("");
          if (entradaSenha == senha) {
            acessoLiberado();
          } else {
            acessoNegado();
          }
          break;
        } else if (key == '#') {
          if (entradaSenha.length() > 0) {
            entradaSenha.remove(entradaSenha.length() - 1);

            digitalWrite(buzzer, HIGH);
            delay(250);
            digitalWrite(buzzer, LOW);

            lcd.setCursor(entradaSenha.length(), 1);
            lcd.print(" ");
            lcd.setCursor(entradaSenha.length(), 1);

            if (senhaDigitada == false) {
              Serial.print("Senha informada: ");
              senhaDigitada = true;
            }

            if (key != '#') {
              Serial.print(key);
            }
          }
        } else {
          if (entradaSenha.length() < 6) {
            entradaSenha += key;
            lcd.print("*");
            digitalWrite(buzzer, HIGH);
            delay(250);
            digitalWrite(buzzer, LOW);

            if (senhaDigitada == false) {
              Serial.print("Senha informada: ");
              senhaDigitada = true;
            }
            if (key != '#') {
              Serial.print(key);
            }
          }
        }
      }
      lcd.noCursor();
    }

    senhaDigitada = false;

    digitalWrite(ledYellow, LOW);
  }
  rfid.PICC_HaltA();
}