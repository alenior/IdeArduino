#include <SPI.h>
#include <MFRC522.h>

// Configuração dos pinos do RFID
#define RST_PIN 22
#define SS_PIN 21
MFRC522 rfid(SS_PIN, RST_PIN);

void setup() {
  Serial.begin(9600); // Inicializa comunicação serial
  SPI.begin();        // Inicializa comunicação SPI
  rfid.PCD_Init();    // Inicializa o módulo RFID

  Serial.println("Passe o cartão/tag no leitor para obter o UID...");
}

void loop() {
  // Verifica se há um novo cartão/tag presente
  if (!rfid.PICC_IsNewCardPresent()) {
    return;
  }

  // Verifica se o cartão/tag foi lido corretamente
  if (!rfid.PICC_ReadCardSerial()) {
    return;
  }

  // Exibe o UID do cartão/tag no monitor serial
  Serial.print("UID do cartão/tag: ");
  for (byte i = 0; i < rfid.uid.size; i++) {
    Serial.print(rfid.uid.uidByte[i] < 0x10 ? "0" : ""); // Adiciona zero à esquerda, se necessário
    Serial.print(rfid.uid.uidByte[i], HEX);             // Imprime cada byte do UID em hexadecimal
    if (i != rfid.uid.size - 1) {
      Serial.print(":"); // Separa os bytes com dois pontos
    }
  }
  Serial.println();

  // Finaliza a comunicação com o cartão/tag
  rfid.PICC_HaltA();
}
