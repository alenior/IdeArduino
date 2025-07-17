#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <HardwareSerial.h>
#include <TinyGPSPlus.h>
#include <time.h>

#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

// --- Wi-Fi ---
#define WIFI_SSID "GCNET-Alencar"
#define WIFI_PASSWORD "11223344"

// --- Firebase ---
#define API_KEY "AIzaSyAMcS7V5q3MDkWkZ4pVCXVNkodZQpfdjKM"
#define DATABASE_URL "https://monitoramento--qualidade-do-ar-default-rtdb.firebaseio.com/"
#define USER_EMAIL "esp32@teste.com"
#define USER_PASSWORD "123456"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// --- Sensores ---
HardwareSerial pmsSerial(2);    // PMS7003: RX=16, TX=17
HardwareSerial gpsSerial(1);    // GPS: RX=4 (conectado ao TX do GPS)
TinyGPSPlus gps;
uint8_t buf[32];

// --- Utilitários ---
String twoDigits(int number) {
  return number < 10 ? "0" + String(number) : String(number);
}

String classificarQualidadeComposta(int pm1_0, int pm2_5, int pm10) {
  int score = 0;
  if (pm1_0 > 0) score++;
  if (pm1_0 > 10) score++;
  if (pm2_5 > 15) score++;
  if (pm2_5 > 50) score++;
  if (pm10 > 50) score++;
  if (pm10 > 100) score++;

  if (score <= 1) return "Bom";
  if (score <= 3) return "Moderado";
  return "Ruim";
}

void sincronizarRelogioComGPS() {
  if (gps.date.isValid() && gps.time.isValid()) {
    struct tm tm;
    tm.tm_year = gps.date.year() - 1900;
    tm.tm_mon = gps.date.month() - 1;
    tm.tm_mday = gps.date.day();
    tm.tm_hour = gps.time.hour();
    tm.tm_min = gps.time.minute();
    tm.tm_sec = gps.time.second();
    time_t t = mktime(&tm);
    struct timeval now = { .tv_sec = t };
    settimeofday(&now, nullptr);
    Serial.println("Horário sincronizado com o GPS.");
  }
}

void setup() {
  Serial.begin(115200);
  pmsSerial.begin(9600, SERIAL_8N1, 16, 17);
  gpsSerial.begin(9600, SERIAL_8N1, 4, -1);
  Serial.println("Inicializando sensores...");

  // --- Wi-Fi ---
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Conectando ao Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConectado ao Wi-Fi");

  // --- Firebase ---
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  config.token_status_callback = tokenStatusCallback;
  config.timeout.serverResponse = 10 * 1000;

  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {
  // --- Atualiza GPS ---
  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
  }

  // --- Lê PMS7003 ---
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

      double latitude = gps.location.isValid() ? gps.location.lat() : 0.0;
      double longitude = gps.location.isValid() ? gps.location.lng() : 0.0;
      double altitude = gps.altitude.isValid() ? gps.altitude.meters() : 0.0;

      String datetime = "0000-00-00T00:00:00Z";
      if (gps.date.isValid() && gps.time.isValid()) {
        datetime = String(gps.date.year()) + "-" +
                   twoDigits(gps.date.month()) + "-" +
                   twoDigits(gps.date.day()) + "T" +
                   twoDigits(gps.time.hour()) + ":" +
                   twoDigits(gps.time.minute()) + ":" +
                   twoDigits(gps.time.second()) + "Z";
        sincronizarRelogioComGPS();
      }

      if (datetime == "0000-00-00T00:00:00Z") {
        Serial.println("GPS sem fix — dados não enviados.");
        return;
      }

      String qualidade = classificarQualidadeComposta(pm1_0, pm2_5, pm10);

      Serial.printf("PM1.0: %d, PM2.5: %d, PM10: %d\n", pm1_0, pm2_5, pm10);
      Serial.printf("Latitude: %.6f, Longitude: %.6f, Altitude: %.2f m\n", latitude, longitude, altitude);
      Serial.println("UTC: " + datetime);
      Serial.println("Qualidade do Ar: " + qualidade);

      // --- Enviar ao Firebase ---
      if (Firebase.ready()) {
        FirebaseJson json;
        json.set("pm1_0", pm1_0);
        json.set("pm2_5", pm2_5);
        json.set("pm10", pm10);
        json.set("latitude", latitude);
        json.set("longitude", longitude);
        json.set("altitude", altitude);
        json.set("datetime_utc", datetime);
        json.set("qualidade", qualidade);

        String basePath = "/historico/" + datetime;
        if (Firebase.RTDB.setJSON(&fbdo, basePath, &json)) {
          Serial.println("Dados enviados com sucesso.");
        } else {
          Serial.println("Erro ao enviar dados: " + fbdo.errorReason());
        }
      } else {
        Serial.println("Firebase não está pronto.");
      }
    } else {
      Serial.println("Checksum inválido no PMS7003.");
    }
  }

  delay(60000);
}
