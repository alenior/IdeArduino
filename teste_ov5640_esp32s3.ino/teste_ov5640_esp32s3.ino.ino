#include <Arduino.h>
#include <inttypes.h>
#include "esp_camera.h"
#include "esp_err.h"
#include "esp_heap_caps.h"

namespace BoardPins {

// Controle do sensor não informado pelo fabricante.
constexpr int CAM_PWDN  = -1;
constexpr int CAM_RESET = -1;

// Barramento SCCB, compatível com I2C.
constexpr int CAM_SIOD = 4;
constexpr int CAM_SIOC = 5;

// Sincronismo e clock.
constexpr int CAM_VSYNC = 6;
constexpr int CAM_HREF  = 7;
constexpr int CAM_XCLK  = 15;
constexpr int CAM_PCLK  = 13;

// Barramento paralelo DVP.
//
// Nomenclatura do módulo:
// Y2 -> D0
// Y3 -> D1
// ...
// Y9 -> D7
constexpr int CAM_D0 = 11;  // Y2
constexpr int CAM_D1 = 9;   // Y3
constexpr int CAM_D2 = 8;   // Y4
constexpr int CAM_D3 = 10;  // Y5
constexpr int CAM_D4 = 12;  // Y6
constexpr int CAM_D5 = 18;  // Y7
constexpr int CAM_D6 = 17;  // Y8
constexpr int CAM_D7 = 16;  // Y9

constexpr int STATUS_LED = 2;

}  // namespace BoardPins

namespace CameraTest {

constexpr uint32_t SERIAL_BAUD_RATE = 115200;

// Começaremos com clock conservador.
// O padrão frequente é 20 MHz, mas 16 MHz reduz a exigência
// durante o primeiro diagnóstico.
constexpr uint32_t XCLK_FREQUENCY_HZ = 20000000;

// VGA oferece um teste mais representativo que QVGA,
// sem exigir imediatamente resolução elevada.
constexpr framesize_t INITIAL_FRAME_SIZE = FRAMESIZE_XGA;

// No driver da câmera, número menor representa JPEG de melhor qualidade.
// 12 é um ponto inicial equilibrado.
constexpr int JPEG_QUALITY = 10;

// Um único buffer simplifica o diagnóstico inicial.
constexpr size_t FRAME_BUFFER_COUNT = 1;

constexpr uint8_t CAPTURE_COUNT = 10;
constexpr uint32_t DELAY_BETWEEN_CAPTURES_MS = 1000;

bool cameraInitialized = false;

void printBytes(const char* label, size_t bytes)
{
    Serial.printf(
        "%-30s: %10lu bytes (%6.2f MB)\n",
        label,
        static_cast<unsigned long>(bytes),
        static_cast<double>(bytes) / (1024.0 * 1024.0)
    );
}

void printMemoryStatus()
{
    Serial.println();
    Serial.println("========== ESTADO DA MEMORIA ==========");

    printBytes("Heap interno livre", ESP.getFreeHeap());
    printBytes(
        "Maior bloco interno",
        heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL)
    );

    printBytes("PSRAM total", ESP.getPsramSize());
    printBytes("PSRAM livre", ESP.getFreePsram());

    printBytes(
        "Maior bloco livre PSRAM",
        heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM)
    );

    Serial.println("=======================================");
}

camera_config_t createCameraConfig()
{
    // Inicialização por atribuições explícitas:
    // evita dependência da ordem dos campos em inicializadores designados.
    camera_config_t config = {};

    config.pin_pwdn = BoardPins::CAM_PWDN;
    config.pin_reset = BoardPins::CAM_RESET;
    config.pin_xclk = BoardPins::CAM_XCLK;

    config.pin_sccb_sda = BoardPins::CAM_SIOD;
    config.pin_sccb_scl = BoardPins::CAM_SIOC;

    config.pin_d0 = BoardPins::CAM_D0;
    config.pin_d1 = BoardPins::CAM_D1;
    config.pin_d2 = BoardPins::CAM_D2;
    config.pin_d3 = BoardPins::CAM_D3;
    config.pin_d4 = BoardPins::CAM_D4;
    config.pin_d5 = BoardPins::CAM_D5;
    config.pin_d6 = BoardPins::CAM_D6;
    config.pin_d7 = BoardPins::CAM_D7;

    config.pin_vsync = BoardPins::CAM_VSYNC;
    config.pin_href = BoardPins::CAM_HREF;
    config.pin_pclk = BoardPins::CAM_PCLK;

    config.xclk_freq_hz = XCLK_FREQUENCY_HZ;

    config.ledc_timer = LEDC_TIMER_0;
    config.ledc_channel = LEDC_CHANNEL_0;

    config.pixel_format = PIXFORMAT_JPEG;
    config.frame_size = INITIAL_FRAME_SIZE;
    config.jpeg_quality = JPEG_QUALITY;

    config.fb_count = FRAME_BUFFER_COUNT;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;

    return config;
}

const char* sensorNameFromPid(uint16_t pid)
{
    switch (pid) {
        case 0x5640:
            return "OV5640";

        case 0x2642:
            return "OV2640";

        case 0x3660:
            return "OV3660";

        default:
            return "Sensor desconhecido";
    }
}

const char* pixelFormatName(pixformat_t format)
{
    switch (format) {
        case PIXFORMAT_JPEG:
            return "JPEG";

        case PIXFORMAT_RGB565:
            return "RGB565";

        case PIXFORMAT_YUV422:
            return "YUV422";

        case PIXFORMAT_GRAYSCALE:
            return "GRAYSCALE";

        default:
            return "OUTRO";
    }
}

bool isValidJpeg(const camera_fb_t* frame)
{
    if (frame == nullptr || frame->buf == nullptr || frame->len < 4) {
        return false;
    }

    // SOI: Start Of Image
    const bool hasStartMarker =
        frame->buf[0] == 0xFF &&
        frame->buf[1] == 0xD8;

    // EOI: End Of Image
    const bool hasEndMarker =
        frame->buf[frame->len - 2] == 0xFF &&
        frame->buf[frame->len - 1] == 0xD9;

    return hasStartMarker && hasEndMarker;
}

bool initializeCamera()
{
    Serial.println();
    Serial.println("[CAMERA] Montando configuracao...");

    if (!psramFound()) {
        Serial.println("[ERRO] PSRAM nao detectada.");
        return false;
    }

    camera_config_t config = createCameraConfig();

    Serial.println("[CAMERA] Inicializando sensor...");
    Serial.printf(
        "[CAMERA] XCLK: %d Hz\n",
        config.xclk_freq_hz
    );

    const esp_err_t result = esp_camera_init(&config);

    if (result != ESP_OK) {
        Serial.printf(
            "[ERRO] esp_camera_init falhou: 0x%04X\n",
            static_cast<unsigned int>(result)
        );

        Serial.printf(
            "[ERRO] Descricao: %s\n",
            esp_err_to_name(result)
        );

        return false;
    }

    cameraInitialized = true;

    Serial.println("[OK] Driver da camera inicializado.");

    sensor_t* sensor = esp_camera_sensor_get();

    if (sensor == nullptr) {
        Serial.println(
            "[ERRO] Driver iniciou, mas o descritor do sensor e nulo."
        );

        esp_camera_deinit();
        cameraInitialized = false;
        return false;
    }

    const uint16_t pid = sensor->id.PID;

    Serial.println();
    Serial.println("========== SENSOR DETECTADO ==========");
    Serial.printf("PID                  : 0x%04X\n", pid);
    Serial.printf("Modelo interpretado  : %s\n", sensorNameFromPid(pid));
    Serial.printf("VER                  : 0x%02X\n", sensor->id.VER);
    Serial.printf("MIDH                 : 0x%02X\n", sensor->id.MIDH);
    Serial.printf("MIDL                 : 0x%02X\n", sensor->id.MIDL);
    Serial.println("======================================");

    if (pid != 0x5640) {
        Serial.println();
        Serial.println(
            "[ATENCAO] O sensor respondeu, mas nao foi identificado como OV5640."
        );
        Serial.println(
            "[ATENCAO] Confira o modelo efetivamente instalado."
        );

        // Não encerramos imediatamente, pois uma OV2640 opcional
        // também poderia estar instalada e ainda permitir captura.
    } else {
        Serial.println("[OK] Sensor identificado como OV5640.");
    }

    // Configurações conservadoras para o primeiro teste.
    //
    // O driver já aplica frame_size e JPEG quality na inicialização,
    // mas fazemos explicitamente para validar o acesso ao sensor.
    if (sensor->set_framesize(sensor, INITIAL_FRAME_SIZE) != 0) {
        Serial.println("[ERRO] Falha ao aplicar resolucao VGA.");
        return false;
    }

    if (sensor->set_quality(sensor, JPEG_QUALITY) != 0) {
        Serial.println("[ERRO] Falha ao aplicar qualidade JPEG.");
        return false;
    }

    // Mantemos ajustes automáticos habilitados.
    sensor->set_gain_ctrl(sensor, 1);
    sensor->set_exposure_ctrl(sensor, 1);
    sensor->set_whitebal(sensor, 1);
    sensor->set_awb_gain(sensor, 1);

    Serial.println("[OK] Configuracoes iniciais aplicadas.");

    return true;
}

bool captureAndValidate(uint8_t sequence)
{
    Serial.println();
    Serial.printf(
        "[CAPTURA %u/%u] Solicitando frame...\n",
        sequence,
        CAPTURE_COUNT
    );

    const uint32_t startTime = millis();

    camera_fb_t* frame = esp_camera_fb_get();

    const uint32_t elapsedTime = millis() - startTime;

    if (frame == nullptr) {
        Serial.println("[ERRO] esp_camera_fb_get retornou nullptr.");
        return false;
    }

    const bool bufferInExternalRam =
        esp_ptr_external_ram(frame->buf);

    Serial.printf(
        "[INFO] Resolucao          : %u x %u\n",
        static_cast<unsigned int>(frame->width),
        static_cast<unsigned int>(frame->height)
    );

    Serial.printf(
        "[INFO] Formato            : %s\n",
        pixelFormatName(frame->format)
    );

    Serial.printf(
        "[INFO] Tamanho JPEG       : %lu bytes\n",
        static_cast<unsigned long>(frame->len)
    );

    Serial.printf(
        "[INFO] Tempo de captura   : %" PRIu32 " ms\n",
        elapsedTime
    );

    Serial.printf(
        "[INFO] Buffer na PSRAM    : %s\n",
        bufferInExternalRam ? "SIM" : "NAO"
    );

    const bool dimensionsValid =
        frame->width == 640 &&
        frame->height == 480;

    const bool formatValid =
        frame->format == PIXFORMAT_JPEG;

    const bool lengthValid =
        frame->len > 1000;

    const bool jpegValid =
        isValidJpeg(frame);

    Serial.printf(
        "[INFO] Marcadores JPEG    : %s\n",
        jpegValid ? "VALIDOS" : "INVALIDOS"
    );

    // O frame deve sempre ser devolvido ao driver,
    // inclusive se alguma validação falhar.
    esp_camera_fb_return(frame);
    frame = nullptr;

    Serial.println("[OK] Frame buffer devolvido ao driver.");

    if (!dimensionsValid) {
        Serial.println("[ERRO] Dimensoes diferentes de 640 x 480.");
    }

    if (!formatValid) {
        Serial.println("[ERRO] Frame nao esta no formato JPEG.");
    }

    if (!lengthValid) {
        Serial.println("[ERRO] Frame JPEG anormalmente pequeno.");
    }

    if (!jpegValid) {
        Serial.println("[ERRO] Marcadores SOI/EOI do JPEG invalidos.");
    }

    if (!bufferInExternalRam) {
        Serial.println(
            "[ATENCAO] O buffer nao foi identificado na PSRAM."
        );
    }

    return dimensionsValid &&
           formatValid &&
           lengthValid &&
           jpegValid &&
           bufferInExternalRam;
}

void deinitializeCamera()
{
    if (!cameraInitialized) {
        return;
    }

    const esp_err_t result = esp_camera_deinit();

    if (result == ESP_OK) {
        Serial.println("[OK] Camera desinicializada.");
    } else {
        Serial.printf(
            "[ATENCAO] Falha ao desinicializar: 0x%04X (%s)\n",
            static_cast<unsigned int>(result),
            esp_err_to_name(result)
        );
    }

    cameraInitialized = false;
}

}  // namespace CameraTest

void setup()
{
    pinMode(BoardPins::STATUS_LED, OUTPUT);
    digitalWrite(BoardPins::STATUS_LED, LOW);

    Serial.begin(CameraTest::SERIAL_BAUD_RATE);

    const uint32_t serialWaitStart = millis();

    while (!Serial && millis() - serialWaitStart < 5000) {
        delay(10);
    }

    delay(1000);

    Serial.println();
    Serial.println("=======================================");
    Serial.println("     TESTE DA CAMERA OV5640");
    Serial.println("=======================================");

    Serial.printf("Chip                 : %s\n", ESP.getChipModel());

    Serial.printf(
        "Frequencia CPU       : %" PRIu32 " MHz\n",
        ESP.getCpuFreqMHz()
    );

    Serial.printf(
        "PSRAM detectada      : %s\n",
        psramFound() ? "SIM" : "NAO"
    );

    CameraTest::printMemoryStatus();

    if (!CameraTest::initializeCamera()) {
        Serial.println();
        Serial.println("=======================================");
        Serial.println("[RESULTADO] CAMERA REPROVADA NA INICIALIZACAO.");
        Serial.println("=======================================");

        digitalWrite(BoardPins::STATUS_LED, LOW);
        return;
    }

    CameraTest::printMemoryStatus();

    uint8_t successfulCaptures = 0;

    // O primeiro frame após inicialização pode apresentar exposição
    // ou balanço de branco ainda em estabilização. Ele é descartado
    // intencionalmente antes da sequência principal.
    Serial.println();
    Serial.println("[CAMERA] Descartando frame inicial...");

    camera_fb_t* warmupFrame = esp_camera_fb_get();

    if (warmupFrame != nullptr) {
        esp_camera_fb_return(warmupFrame);
        warmupFrame = nullptr;

        Serial.println("[OK] Frame inicial descartado.");
    } else {
        Serial.println(
            "[ATENCAO] Nao foi possivel obter o frame inicial."
        );
    }

    delay(500);

    for (uint8_t index = 1; index <= CameraTest::CAPTURE_COUNT; ++index) {
        if (CameraTest::captureAndValidate(index)) {
            ++successfulCaptures;
            digitalWrite(BoardPins::STATUS_LED, HIGH);
        } else {
            digitalWrite(BoardPins::STATUS_LED, LOW);
        }

        delay(CameraTest::DELAY_BETWEEN_CAPTURES_MS);
    }

    Serial.println();
    Serial.println("========== RESUMO DO TESTE ==========");
    Serial.printf(
        "Capturas aprovadas   : %u/%u\n",
        successfulCaptures,
        CameraTest::CAPTURE_COUNT
    );
    Serial.println("=====================================");

    CameraTest::printMemoryStatus();

    if (successfulCaptures == CameraTest::CAPTURE_COUNT) {
        Serial.println();
        Serial.println("=======================================");
        Serial.println("[RESULTADO] OV5640 APROVADA.");
        Serial.println("Camera pronta para o endpoint /capture.");
        Serial.println("=======================================");

        digitalWrite(BoardPins::STATUS_LED, HIGH);
    } else {
        Serial.println();
        Serial.println("=======================================");
        Serial.println("[RESULTADO] TESTE DA CAMERA INCOMPLETO.");
        Serial.println("Analise as capturas que apresentaram erro.");
        Serial.println("=======================================");

        digitalWrite(BoardPins::STATUS_LED, LOW);
    }

    // Mantemos a câmera inicializada para observar estabilidade.
    // Neste teste não chamamos deinitializeCamera() ao final.
}

void loop()
{
    // O diagnóstico completo é executado uma vez no setup().
    delay(1000);
}