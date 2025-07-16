#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <HardwareSerial.h>
#include <TinyGPSPlus.h>

#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define WIFI_SSID "GCNET-Alencar" //"Alencar's Galaxy M14 5G"
#define WIFI_PASSWORD "11223344"

#define API_KEY "AIzaSyAMcS7V5q3MDkWkZ4pVCXVNkodZQpfdjKM"
#define DATABASE_URL "https://monitoramento--qualidade-do-ar-default-rtdb.firebaseio.com/"
#define USER_EMAIL "esp32@teste.com"
#define USER_PASSWORD "123456"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

HardwareSerial pmsSerial(2);    // PMS7003: RX=16, TX=17
HardwareSerial gpsSerial(1);    // GPS: RX=4 (conectado ao TX do GPS), TX=não usado
TinyGPSPlus gps;

uint8_t buf[32];

String twoDigits(int number) {
  return number < 10 ? "0" + String(number) : String(number);
}

String classificarQualidadeComposta(int pm1_0, int pm2_5, int pm10) {
  int score = 0;

  // PM1.0
  if (pm1_0 > 0) score++;
  if (pm1_0 > 10) score++;

  // PM2.5
  if (pm2_5 > 15) score++;
  if (pm2_5 > 50) score++;

  // PM10
  if (pm10 > 50) score++;
  if (pm10 > 100) score++;

  if (score <= 1) return "Bom";
  if (score <= 3) return "Moderado";
  return "Ruim";
}

void setup() {
  Serial.begin(115200);
  pmsSerial.begin(9600, SERIAL_8N1, 16, 17);
  gpsSerial.begin(9600, SERIAL_8N1, 4, -1);
  Serial.println("Inicializando sensores...");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Conectando ao Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConectado ao Wi-Fi");

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  Firebase.begin(&config, &auth);
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
      }

      String qualidade = classificarQualidadeComposta(pm1_0, pm2_5, pm10);

      Serial.printf("PM1.0: %d, PM2.5: %d, PM10: %d\n", pm1_0, pm2_5, pm10);
      Serial.printf("Latitude: %.6f, Longitude: %.6f, Altitude: %.2f m\n", latitude, longitude, altitude);
      Serial.println("UTC: " + datetime);
      Serial.println("Qualidade do Ar: " + qualidade);

      String timestampKey = String(millis());  // ou use datetime se for único
      String basePath = "/historico/" + timestampKey;
      // String basePath = "/leituras";
      Firebase.RTDB.setInt(&fbdo, basePath + "/pm1_0", pm1_0);
      Firebase.RTDB.setInt(&fbdo, basePath + "/pm2_5", pm2_5);
      Firebase.RTDB.setInt(&fbdo, basePath + "/pm10", pm10);
      Firebase.RTDB.setDouble(&fbdo, basePath + "/latitude", latitude);
      Firebase.RTDB.setDouble(&fbdo, basePath + "/longitude", longitude);
      Firebase.RTDB.setDouble(&fbdo, basePath + "/altitude", altitude);
      Firebase.RTDB.setString(&fbdo, basePath + "/datetime_utc", datetime);
      Firebase.RTDB.setString(&fbdo, basePath + "/qualidade", qualidade);

    } else {
      Serial.println("Checksum inválido no PMS7003.");
    }
  }
  delay(5000);
}
