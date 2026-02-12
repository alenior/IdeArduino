#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <ArduinoOTA.h>
#include <esp_task_wdt.h>

#include "secrets.h"

// ===== CONFIG =====
#define LED_PIN 2
#define HEARTBEAT_INTERVAL 10000
#define WDT_TIMEOUT_SECONDS 15

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

String DEVICE_ID;
unsigned long lastHeartbeat = 0;

// ===== WIFI =====
void initWiFi()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
  }
}

// ===== DEVICE ID =====
void initDeviceId()
{
  DEVICE_ID = WiFi.macAddress();
}

// ===== FIREBASE =====
void initFirebase()
{

  config.api_key = FIREBASE_API_KEY;

  // Token recebido via backend
  auth.token.uid = "";
  auth.token.custom_token = DEVICE_CUSTOM_TOKEN;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

// ===== BASE PATH =====
String basePath()
{
  return "devices/" + DEVICE_ID;
}

// ===== HEARTBEAT =====
void sendHeartbeat()
{

  FirebaseJson json;

  json.set("fields/online/booleanValue", true);
  json.set("fields/uptime/integerValue", String(millis() / 1000));
  json.set("fields/rssi/integerValue", String(WiFi.RSSI()));

  Firebase.Firestore.patchDocument(
      &fbdo,
      FIREBASE_PROJECT_ID,
      "",
      (basePath() + "/status/runtime").c_str(),
      json.raw(),
      "online,uptime,rssi");
}

// ===== EXECUTE COMMAND =====
void executeCommand(String cmd)
{
  if (cmd == "LED_ON")
    digitalWrite(LED_PIN, HIGH);
  if (cmd == "LED_OFF")
    digitalWrite(LED_PIN, LOW);
}

// ===== CHECK COMMAND =====
void checkCommands()
{

  if (Firebase.Firestore.getDocument(
          &fbdo,
          FIREBASE_PROJECT_ID,
          "",
          (basePath() + "/commands/pending").c_str()))
  {

    FirebaseJson payload;
    payload.setJsonData(fbdo.payload());

    FirebaseJsonData result;
    payload.get(result, "fields/cmd/stringValue");

    if (result.success)
    {

      executeCommand(result.stringValue);

      Firebase.Firestore.deleteDocument(
          &fbdo,
          FIREBASE_PROJECT_ID,
          "",
          (basePath() + "/commands/pending").c_str());
    }
  }
}

// ===== SETUP =====
void setup()
{

  pinMode(LED_PIN, OUTPUT);

  initWiFi();
  initDeviceId();
  initFirebase();
}

// ===== LOOP =====
void loop()
{

  if (millis() - lastHeartbeat > HEARTBEAT_INTERVAL)
  {
    sendHeartbeat();
    lastHeartbeat = millis();
  }

  checkCommands();
}
