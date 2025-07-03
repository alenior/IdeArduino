#include <TinyGsmClient.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <HardwareSerial.h>
#include <TinyGPSPlus.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

// ======== REDE Wi-Fi (opcional) =========
#define WIFI_SSID "Alencar's Galaxy M14 5G"
#define WIFI_PASSWORD "11223344"

// ======== DADOS DO FIREBASE =========
#define API_KEY "AIzaSyAMcS7V5q3MDkWkZ4pVCXVNkodZQpfdjKM"
#define DATABASE_URL "https://monitoramento--qualidade-do-ar-default-rtdb.firebaseio.com/"
#define USER_EMAIL "esp32@teste.com"
#define USER_PASSWORD "123456"

// ======== DADOS APN (SIM card) =========
#define MODEM_BAUD 9600
#define APN "zap.vivo.com.br"  // Altere conforme operadora
#define GPRS_USER "vivo"
#define GPRS_PASS "vivo"

// Mapeamento de hardware
HardwareSerial pmsSerial(2); // PMS7003 (RX=16, TX=17)
HardwareSerial gpsSerial(1); // GPS (RX=4, TX=not used)
HardwareSerial sim800Serial(0); // SIM800L (RX=3, TX=1)

TinyGPSPlus gps;
TinyGsm modem(sim800Serial);
TinyGsmClient gsmClient;

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

uint8_t buf[32];

bool gprsConnected = false;
bool wifiConnected = false;

String twoDigits(int number) {
  return number < 10 ? "0" + String(number) : String(number);
}

void setup() {
  Serial.begin(115200);
  delay(3000);
  Serial.println("Inicializando...");

  // Sensores
  pmsSerial.begin(9600, SERIAL_8N1, 16, 17);
  gpsSerial.begin(9600, SERIAL_8N1, 4, -1);

  // SIM800L
  sim800Serial.begin(MODEM_BAUD, SERIAL_8N1, 3, 1);
  delay(3000);
  Serial.println("Inicializando SIM800L...");
  modem.restart();
  String modemInfo = modem.getModemInfo();
  Serial.println("Modem: " + modemInfo);

  // Tenta conexão GPRS
  if (modem.waitForNetwork()) {
    if (modem.gprsConnect(APN, GPRS_USER, GPRS_PASS)) {
      gprsConnected = true;
      Serial.println("GPRS conectado.");
    }
  }

  // Se GPRS falhar, tenta Wi-Fi
  if (!gprsConnected) {
    Serial.println("Tentando Wi-Fi...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    for (int i = 0; i < 20 && WiFi.status() != WL_CONNECTED; i++) {
      delay(500);
      Serial.print(".");
    }
    wifiConnected = (WiFi.status() == WL_CONNECTED);
    if (wifiConnected) Serial.println("\nWi-Fi conectado.");
    else Serial.println("\nFalha no Wi-Fi também.");
  }

  // Firebase
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  if (gprsConnected) {
    config.wifi.clearAP(); // Limpa Wi-Fi, usamos GSM
    Firebase.begin(&config, &auth, &gsmClient);
  } else {
    Firebase.begin(&config, &auth);
  }
  Firebase.reconnectWiFi(true);
}

void loop() {
  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
  }

  if (pmsSerial.available() >= 32 && pmsSerial.read() == 0x42 && pmsSerial.read() == 0x4D) {
    buf[0] = 0x42;
    buf[1] = 0x4D;
    for (int i = 2; i < 32; i++) buf[i] = pmsSerial.read();

    uint16_t checksum = 0;
    for (int i = 0; i < 30; i++) checksum += buf[i];
    uint16_t received_checksum = (buf[30] << 8) | buf[31];

    if (checksum == received_checksum) {
      int pm1_0 = (buf[10] << 8) | buf[11];
      int pm2_5 = (buf[12] << 8) | buf[13];
      int pm10  = (buf[14] << 8) | buf[15];

      double latitude = 0.0;
      double longitude = 0.0;
      double altitude = 0.0;
      String datetime = "0000-00-00T00:00:00Z";

      if (gps.location.isValid()) {
        latitude = gps.location.lat();
        longitude = gps.location.lng();
      }
      if (gps.altitude.isValid()) altitude = gps.altitude.meters();
      if (gps.date.isValid() && gps.time.isValid()) {
        datetime = String(gps.date.year()) + "-" +
                   twoDigits(gps.date.month()) + "-" +
                   twoDigits(gps.date.day()) + "T" +
                   twoDigits(gps.time.hour()) + ":" +
                   twoDigits(gps.time.minute()) + ":" +
                   twoDigits(gps.time.second()) + "Z";
      }

      if (Firebase.ready()) {
        FirebaseJson json;
        json.set("pm1_0", pm1_0);
        json.set("pm2_5", pm2_5);
        json.set("pm10", pm10);
        json.set("latitude", latitude);
        json.set("longitude", longitude);
        json.set("altitude", altitude);
        json.set("datetime_utc", datetime);

        if (Firebase.RTDB.setJSON(&fbdo, "/leituras", &json)) {
          Serial.println("Enviado ao Firebase com sucesso.");
        } else {
          Serial.print("Erro ao enviar: ");
          Serial.println(fbdo.errorReason());
        }
      } else {
        Serial.println("Firebase não está pronto.");
      }
    } else {
      Serial.println("Checksum inválido no PMS7003.");
    }
  }
  delay(5000);
}
