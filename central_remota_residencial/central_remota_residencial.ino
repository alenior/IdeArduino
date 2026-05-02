#include <WiFi.h>
#include <Firebase_ESP_Client.h>

#define WIFI_SSID "GCNET-Alencar"
#define WIFI_PASSWORD "11223344"

#define API_KEY "SUA_API_KEY"
#define DATABASE_URL "SEU_URL"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// GPIOs
int relePins[4] = {2, 3, 4, 5};

void setup() {
  Serial.begin(115200);

  for (int i = 0; i < 4; i++) {
    pinMode(relePins[i], OUTPUT);
    digitalWrite(relePins[i], HIGH); // desligado (ativo LOW)
  }

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) delay(500);

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {

  for (int i = 0; i < 4; i++) {
    String path = "/dispositivos/casa01/rele" + String(i + 1);

    if (Firebase.RTDB.getBool(&fbdo, path)) {
      bool estado = fbdo.boolData();

      digitalWrite(relePins[i], estado ? LOW : HIGH);
    }
  }

  delay(500);
}