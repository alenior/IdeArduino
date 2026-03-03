#include <WiFi.h>
#include "esp_system.h"
#include "esp_chip_info.h"

// ==========================
// DEFINIÇÕES DE HARDWARE
// ==========================
#define LED_PIN 2
#define BUTTON_PIN 0

// ==========================
// METADATA DO DISPOSITIVO
// ==========================
#define DEVICE_PLATFORM "ESP32"
#define DEVICE_MODEL "GENERIC_DEVKIT"
#define HARDWARE_REV "1.0"
#define FIRMWARE_VERSION "1.0.0"

// ==========================
// VARIÁVEIS GLOBAIS
// ==========================
String DEVICE_ID;

float temperature = 25.0;
bool ledState = false;

unsigned long lastSensorRead = 0;
unsigned long lastHeartbeat = 0;

#define SENSOR_INTERVAL 3000
#define HEARTBEAT_INTERVAL 10000

// Debounce
bool buttonState = HIGH;
bool lastReading = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

// ==========================
// PROTÓTIPOS
// ==========================
void setLed(bool state);

// ==========================
// BOTÃO COM DEBOUNCE
// ==========================
void checkButton()
{
  bool reading = digitalRead(BUTTON_PIN);

  if (reading != lastReading)
  {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) >= debounceDelay)
  {
    if (reading != buttonState)
    {
      buttonState = reading;

      if (buttonState == LOW)
      {
        setLed(!ledState);
      }
    }
  }

  lastReading = reading;
}

// ==========================
// INFORMAÇÕES DO CHIP
// ==========================
void printChipInfo()
{
  esp_chip_info_t chip_info;
  esp_chip_info(&chip_info);

  Serial.println(F("=== CHIP INFO ==="));

  Serial.print(F("Modelo: ESP32"));
  Serial.print(F(" | Cores: "));
  Serial.print(chip_info.cores);

  Serial.print(F(" | Revisão: "));
  Serial.print(chip_info.revision);

  Serial.print(F(" | Flash: "));
  Serial.print(ESP.getFlashChipSize() / (1024 * 1024));
  Serial.println(F("MB"));

  if (chip_info.features & CHIP_FEATURE_BT)
    Serial.println(F("Bluetooth: SIM"));

  if (chip_info.features & CHIP_FEATURE_BLE)
    Serial.println(F("BLE: SIM"));
}

// ==========================
// DEVICE ID (MAC eFuse)
// ==========================
void initDeviceId()
{
  WiFi.mode(WIFI_MODE_STA);
  delay(100);

  uint64_t chipid = ESP.getEfuseMac();

  char macStr[18];
  sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X",
          (uint8_t)(chipid >> 40),
          (uint8_t)(chipid >> 32),
          (uint8_t)(chipid >> 24),
          (uint8_t)(chipid >> 16),
          (uint8_t)(chipid >> 8),
          (uint8_t)chipid);

  DEVICE_ID = String(macStr);

  Serial.print(F("MAC detectado: "));
  Serial.println(DEVICE_ID);
}

// ==========================
// CAPABILIDADES (CONTRATO)
// ==========================
void printCapabilities()
{
  Serial.println(F("=== DEVICE CAPABILITIES ==="));
  Serial.println(F("{"));

  Serial.print(F("\"deviceId\": \""));
  Serial.print(DEVICE_ID);
  Serial.println(F("\","));  

  Serial.print(F("\"platform\": \""));
  Serial.print(DEVICE_PLATFORM);
  Serial.println(F("\","));  

  Serial.print(F("\"model\": \""));
  Serial.print(DEVICE_MODEL);
  Serial.println(F("\","));  

  Serial.print(F("\"firmwareVersion\": \""));
  Serial.print(FIRMWARE_VERSION);
  Serial.println(F("\","));  

  Serial.print(F("\"hardwareRevision\": \""));
  Serial.print(HARDWARE_REV);
  Serial.println(F("\","));  

  Serial.println(F("\"capabilities\": {"));

  Serial.println(F("\"sensors\": ["));
  Serial.println(F("{\"id\": \"temperature\", \"type\": \"float\", \"unit\": \"celsius\"}"));
  Serial.println(F("],"));

  Serial.println(F("\"actuators\": ["));
  Serial.println(F("{\"id\": \"led\", \"type\": \"boolean\"}"));
  Serial.println(F("]"));

  Serial.println(F("}"));
  Serial.println(F("}"));
}

// ==========================
// SENSOR SIMULADO
// ==========================
void readSensors()
{
  if (millis() - lastSensorRead >= SENSOR_INTERVAL)
  {
    temperature += random(-5, 6) * 0.1;

    Serial.print(F("Temperature: "));
    Serial.println(temperature);

    lastSensorRead = millis();
  }
}

// ==========================
// HEARTBEAT SIMULADO
// ==========================
void sendHeartbeat()
{
  if (millis() - lastHeartbeat >= HEARTBEAT_INTERVAL)
  {
    Serial.println(F("HEARTBEAT"));
    lastHeartbeat = millis();
  }
}

// ==========================
// CONTROLE DO LED
// ==========================
void setLed(bool state)
{
  ledState = state;
  digitalWrite(LED_PIN, state ? HIGH : LOW);

  Serial.println(state ? F("LED ON") : F("LED OFF"));
}

// ==========================
// COMANDOS VIA SERIAL
// ==========================
void checkSerial()
{
  if (Serial.available())
  {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd == "LED_ON")
      setLed(true);

    else if (cmd == "LED_OFF")
      setLed(false);

    else if (cmd == "STATUS")
    {
      Serial.println(F("=== STATUS ==="));
      Serial.print(F("Temperature: "));
      Serial.println(temperature);

      Serial.print(F("LED: "));
      Serial.println(ledState);
    }
  }
}

// ==========================
// SETUP
// ==========================
void setup()
{
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  randomSeed(esp_random());

  initDeviceId();
  printChipInfo();
  printCapabilities();
}

// ==========================
// LOOP
// ==========================
void loop()
{
  readSensors();
  checkSerial();
  checkButton();
  sendHeartbeat();
}
