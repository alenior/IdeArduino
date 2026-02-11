#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <Firebase_ESP_Client.h>
#include <ArduinoOTA.h>
#include <esp_task_wdt.h>

#include "secrets.h"

// ================= CONFIG =================
#define LED_PIN 2
#define HEARTBEAT_INTERVAL 10000
#define WDT_TIMEOUT_SECONDS 15

// ================ FIREBASE OBJECTS =================
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

String DEVICE_ID;
String DEVICE_APELIDO = "ESP32-DEMO";

unsigned long lastHeartbeat = 0;

// ================= WIFI =================
void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

// ================= DEVICE ID =================
void initDeviceId() {
  DEVICE_ID = WiFi.macAddress();
}

// ================= FIREBASE =================
void tokenStatusCallback(TokenInfo info) {

  Serial.println("==== Firebase Token Event ====");

  Serial.printf("Status: %d\n", info.status);
  Serial.printf("Type: %d\n", info.type);

  if (info.status == token_status_error) {
    Serial.printf("Error code: %d\n", info.error.code);
    Serial.printf("Error message: %s\n", info.error.message.c_str());
  }

  Serial.println("================================");
}

void initFirebase() {

  config.api_key = FIREBASE_API_KEY;
  config.token_status_callback = tokenStatusCallback; // opcional debug

  auth.user.email = FIREBASE_USER_EMAIL;
  auth.user.password = FIREBASE_USER_PASSWORD;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

// ================= WATCHDOG (CORE 3.x FIX) =================
void initWatchdog() {

  esp_task_wdt_config_t wdt_config = {
    .timeout_ms = WDT_TIMEOUT_SECONDS * 1000,
    .idle_core_mask = (1 << portNUM_PROCESSORS) - 1,
    .trigger_panic = true
  };

  esp_task_wdt_init(&wdt_config);
  esp_task_wdt_add(NULL);
}

// ================= OTA =================
void initOTA() {
  ArduinoOTA.setHostname(DEVICE_APELIDO.c_str());
  ArduinoOTA.begin();
}

// ================= PATH BASE =================
String basePath() {
  return "devices/" + DEVICE_ID;
}

// ================= PUBLICAR INFO =================
void publishDeviceInfo() {

  FirebaseJson json;
  json.set("fields/apelido/stringValue", DEVICE_APELIDO);
  json.set("fields/deviceId/stringValue", DEVICE_ID);
  json.set("fields/ativo/booleanValue", true);

  Firebase.Firestore.patchDocument(
    &fbdo,
    FIREBASE_PROJECT_ID,
    "",
    basePath().c_str(),
    json.raw(),
    "apelido,deviceId,ativo"
  );
}

// ================= PUBLICAR FUNÇÕES =================
void publishFunctions() {

  FirebaseJson json;

  json.set("fields/nome/stringValue", "Ligar LED");
  json.set("fields/tipo/stringValue", "atuador");
  json.set("fields/comando/stringValue", "LED_ON");

  Firebase.Firestore.patchDocument(
    &fbdo,
    FIREBASE_PROJECT_ID,
    "",
    (basePath() + "/functions/led_on").c_str(),
    json.raw(),
    "nome,tipo,comando"
  );

  json.clear();

  json.set("fields/nome/stringValue", "Desligar LED");
  json.set("fields/tipo/stringValue", "atuador");
  json.set("fields/comando/stringValue", "LED_OFF");

  Firebase.Firestore.patchDocument(
    &fbdo,
    FIREBASE_PROJECT_ID,
    "",
    (basePath() + "/functions/led_off").c_str(),
    json.raw(),
    "nome,tipo,comando"
  );
}

// ================= HEARTBEAT =================
void sendHeartbeat() {

  FirebaseJson json;

  json.set("fields/online/booleanValue", true);
  json.set("fields/uptime/integerValue", String(millis() / 1000));
  json.set("fields/rssi/integerValue", String(WiFi.RSSI()));
  json.set("fields/ip/stringValue", WiFi.localIP().toString());

  Firebase.Firestore.patchDocument(
    &fbdo,
    FIREBASE_PROJECT_ID,
    "",
    (basePath() + "/status").c_str(),
    json.raw(),
    "online,uptime,rssi,ip"
  );
}

// ================= EXECUTAR COMANDO =================
void executeCommand(String cmd) {
  if (cmd == "LED_ON") {
    digitalWrite(LED_PIN, HIGH);
  }
  else if (cmd == "LED_OFF") {
    digitalWrite(LED_PIN, LOW);
  }
}

// ================= VERIFICAR COMANDOS =================
void checkCommands() {

  if (Firebase.Firestore.getDocument(
        &fbdo,
        FIREBASE_PROJECT_ID,
        "",
        (basePath() + "/commands/pending").c_str())) {

    FirebaseJson payload;
    payload.setJsonData(fbdo.payload());

    FirebaseJsonData result;
    payload.get(result, "fields/cmd/stringValue");

    if (result.success) {

      String cmd = result.stringValue;
      executeCommand(cmd);

      FirebaseJson response;
      response.set("fields/status/stringValue", "executed");

      Firebase.Firestore.patchDocument(
        &fbdo,
        FIREBASE_PROJECT_ID,
        "",
        (basePath() + "/commands/pending").c_str(),
        response.raw(),
        "status"
      );
    }
  }
}

// ================= SETUP =================
void setup() {

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  initWiFi();
  initDeviceId();
  initFirebase();
  initWatchdog();
  initOTA();

  publishDeviceInfo();
  publishFunctions();
}

// ================= LOOP =================
void loop() {

  esp_task_wdt_reset();
  ArduinoOTA.handle();

  if (millis() - lastHeartbeat > HEARTBEAT_INTERVAL) {
    sendHeartbeat();
    lastHeartbeat = millis();
  }

  checkCommands();
}
