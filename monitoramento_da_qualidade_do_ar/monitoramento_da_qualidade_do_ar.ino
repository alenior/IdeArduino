#include <HardwareSerial.h>

// UART2 do ESP32 — RX = GPIO16, TX = GPIO17
HardwareSerial pmsSerial(2);
uint8_t buf[32];

void setup() {
  Serial.begin(115200);
  pmsSerial.begin(9600, SERIAL_8N1, 16, 17);
  Serial.println("Inicializando sensor PMS7003...");
}

void loop() {
  /*while (pmsSerial.available()) {
    uint8_t b = pmsSerial.read();
    Serial.print("0x");
    if (b < 16) Serial.print("0");
    Serial.print(b, HEX);
    Serial.print(" ");
  }
  delay(100);*/

  if (pmsSerial.available() >= 32) {
    if (pmsSerial.read() == 0x42) {
      if (pmsSerial.read() == 0x4D) {
        buf[0] = 0x42;
        buf[1] = 0x4D;

        for (int i = 2; i < 32; i++) {
          buf[i] = pmsSerial.read();
        }

        uint16_t checksum = 0;
        for (int i = 0; i < 30; i++) {
          checksum += buf[i];
        }
        uint16_t received_checksum = (buf[30] << 8) | buf[31];

        if (checksum == received_checksum) {
          int pm1_0  = (buf[10] << 8) | buf[11];
          int pm2_5  = (buf[12] << 8) | buf[13];
          int pm10   = (buf[14] << 8) | buf[15];

          Serial.print("PM1.0: ");
          Serial.print(pm1_0);
          Serial.print(" µg/m3, PM2.5: ");
          Serial.print(pm2_5);
          Serial.print(" µg/m3, PM10: ");
          Serial.print(pm10);
          Serial.println(" µg/m3");
        } else {
          Serial.println("Pacote inválido (checksum não confere).");
        }
      }
    }
  }
  delay(1000);
}