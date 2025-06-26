#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <HardwareSerial.h>
#include <TinyGPSPlus.h>  // GPS

#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

// ======== SUA REDE Wi-Fi =========
#define WIFI_SSID "Alencar's Galaxy M14 5G"
#define WIFI_PASSWORD "11223344"

// ======== DADOS DO FIREBASE =========
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

void setup() {
  Serial.begin(115200);

  // Inicializa os sensores
  pmsSerial.begin(9600, SERIAL_8N1, 16, 17);  // PMS7003
  gpsSerial.begin(9600, SERIAL_8N1, 4, -1);   // GPS NEO-6M

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
  // Leitura do GPS (loop de parsing)
  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
  }

  // Leitura do PMS7003
  if (pmsSerial.available() >= 32 && pmsSerial.read() == 0x42 && pmsSerial.read() == 0x4D) {
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
      int pm1_0 = (buf[10] << 8) | buf[11];
      int pm2_5 = (buf[12] << 8) | buf[13];
      int pm10  = (buf[14] << 8) | buf[15];

      Serial.printf("PM1.0: %d, PM2.5: %d, PM10: %d\n", pm1_0, pm2_5, pm10);

      // Somente envia se os dados de localização forem válidos
      if (gps.location.isValid() && gps.altitude.isValid() && gps.date.isValid() && gps.time.isValid()) {
        double latitude = gps.location.lat();
        double longitude = gps.location.lng();
        double altitude = gps.altitude.meters();
        String datetime = String(gps.date.year()) + "-" + gps.date.month() + "-" + gps.date.day() + "T" +
                          gps.time.hour() + ":" + gps.time.minute() + ":" + gps.time.second() + "Z";

        Serial.printf("Latitude: %.6f, Longitude: %.6f, Altitude: %.2f m\n", latitude, longitude, altitude);
        Serial.println("UTC: " + datetime);

        // Enviar dados ao Firebase
        String basePath = "/leituras";
        Firebase.RTDB.setInt(&fbdo, basePath + "/pm1_0", pm1_0);
        Firebase.RTDB.setInt(&fbdo, basePath + "/pm2_5", pm2_5);
        Firebase.RTDB.setInt(&fbdo, basePath + "/pm10", pm10);
        Firebase.RTDB.setDouble(&fbdo, basePath + "/latitude", latitude);
        Firebase.RTDB.setDouble(&fbdo, basePath + "/longitude", longitude);
        Firebase.RTDB.setDouble(&fbdo, basePath + "/altitude", altitude);
        Firebase.RTDB.setString(&fbdo, basePath + "/datetime_utc", datetime);
      } else {
        Serial.println("Aguardando fix do GPS...");
      }
    } else {
      Serial.println("Checksum inválido no PMS7003.");
    }
  }

  delay(3000);  // Aguarda entre medições
}
