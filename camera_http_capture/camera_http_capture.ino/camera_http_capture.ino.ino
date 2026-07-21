#include <Arduino.h>
#include <WiFi.h>
#include <inttypes.h>

#include "board_config.h"
#include "camera_service.h"
#include "secrets.h"
#include "web_server_service.h"

namespace AppConfig {

constexpr uint32_t SERIAL_BAUD_RATE = 115200;
constexpr uint32_t SERIAL_WAIT_TIMEOUT_MS = 5000;

constexpr uint32_t WIFI_CONNECT_TIMEOUT_MS = 20000;
constexpr uint32_t WIFI_STATUS_INTERVAL_MS = 500;

constexpr uint32_t CONNECTION_CHECK_INTERVAL_MS = 5000;

}  // namespace AppConfig

namespace {

uint32_t lastConnectionCheckMs = 0;

void initializeSerial()
{
    Serial.begin(AppConfig::SERIAL_BAUD_RATE);

    const uint32_t startTime = millis();

    while (!Serial &&
           millis() - startTime <
               AppConfig::SERIAL_WAIT_TIMEOUT_MS) {

        delay(10);
    }

    delay(500);
}

bool connectToWiFi()
{
    Serial.printf(
        "[WIFI] RSSI: %d dBm\n",
        WiFi.RSSI()
    );

    WiFi.mode(WIFI_STA);

    // Desabilitar o modo de economia de energia melhora
    // previsibilidade e latência em aplicações de câmera.
    WiFi.setSleep(false);

    WiFi.begin(
        Secrets::WIFI_SSID,
        Secrets::WIFI_PASSWORD
    );

    const uint32_t connectionStart = millis();

    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - connectionStart >=
            AppConfig::WIFI_CONNECT_TIMEOUT_MS) {

            Serial.println();
            Serial.println(
                "[WIFI] Tempo limite de conexao excedido."
            );

            return false;
        }

        Serial.print(".");
        delay(AppConfig::WIFI_STATUS_INTERVAL_MS);
    }

    Serial.println();
    Serial.println("[WIFI] Conectado.");

    Serial.print("[WIFI] Endereco IP: ");
    Serial.println(WiFi.localIP());

    Serial.print("[WIFI] Gateway: ");
    Serial.println(WiFi.gatewayIP());

    Serial.print("[WIFI] Mascara: ");
    Serial.println(WiFi.subnetMask());

    Serial.print("[WIFI] MAC: ");
    Serial.println(WiFi.macAddress());

    Serial.printf(
        "[WIFI] RSSI: %d dBm\n",
        WiFi.RSSI()
    );

    return true;
}

void printAccessAddresses()
{
    const IPAddress ip = WiFi.localIP();

    Serial.println();
    Serial.println("=======================================");
    Serial.println("       SERVIDOR DISPONIVEL");
    Serial.println("=======================================");

    Serial.printf(
        "Pagina principal : http://%u.%u.%u.%u/\n",
        ip[0], ip[1], ip[2], ip[3]
    );

    Serial.printf(
        "Captura JPEG     : http://%u.%u.%u.%u/capture\n",
        ip[0], ip[1], ip[2], ip[3]
    );

    Serial.printf(
        "Status JSON      : http://%u.%u.%u.%u/status\n",
        ip[0], ip[1], ip[2], ip[3]
    );

    Serial.printf(
        "Health check     : http://%u.%u.%u.%u/health\n",
        ip[0], ip[1], ip[2], ip[3]
    );

    Serial.println("=======================================");
}

void indicateFailure()
{
    digitalWrite(BoardPins::STATUS_LED, LOW);
}

void indicateSuccess()
{
    digitalWrite(BoardPins::STATUS_LED, HIGH);
}

void checkWiFiConnection()
{
    if (millis() - lastConnectionCheckMs <
        AppConfig::CONNECTION_CHECK_INTERVAL_MS) {

        return;
    }

    lastConnectionCheckMs = millis();

    if (WiFi.status() == WL_CONNECTED) {
        return;
    }

    Serial.println("[WIFI] Conexao perdida.");

    indicateFailure();

    WiFi.disconnect();
    WiFi.begin(
        Secrets::WIFI_SSID,
        Secrets::WIFI_PASSWORD
    );
}

}  // namespace

void setup()
{
    pinMode(BoardPins::STATUS_LED, OUTPUT);
    indicateFailure();

    initializeSerial();

    Serial.println();
    Serial.println("=======================================");
    Serial.println(" ESP32-S3 OV5640 - CAPTURA HTTP");
    Serial.println("=======================================");

    Serial.printf("Chip            : %s\n", ESP.getChipModel());

    Serial.printf(
        "CPU             : %" PRIu32 " MHz\n",
        ESP.getCpuFreqMHz()
    );

    Serial.printf(
        "PSRAM detectada : %s\n",
        psramFound() ? "SIM" : "NAO"
    );

    Serial.printf(
        "PSRAM total     : %lu bytes\n",
        static_cast<unsigned long>(ESP.getPsramSize())
    );

    if (!CameraService::begin()) {
        Serial.println();
        Serial.println(
            "[RESULTADO] Falha na inicializacao da camera."
        );

        indicateFailure();
        return;
    }

    if (!connectToWiFi()) {
        Serial.println();
        Serial.println(
            "[RESULTADO] Camera iniciou, mas o Wi-Fi falhou."
        );

        indicateFailure();
        return;
    }

    if (!WebServerService::begin()) {
        Serial.println();
        Serial.println(
            "[RESULTADO] Falha ao iniciar servidor HTTP."
        );

        indicateFailure();
        return;
    }

    indicateSuccess();
    printAccessAddresses();

    Serial.println();
    Serial.println("[RESULTADO] SISTEMA INICIALIZADO.");
}

void loop()
{
    checkWiFiConnection();
    delay(20);
}