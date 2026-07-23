#include "camera_service.h"

#include "board_config.h"

namespace CameraService
{

    namespace
    {

        constexpr uint32_t XCLK_FREQUENCY_HZ = 20000000;
        constexpr framesize_t INITIAL_FRAME_SIZE = FRAMESIZE_XGA;
        constexpr int JPEG_QUALITY = 10;
        constexpr size_t FRAME_BUFFER_COUNT = 1;

        bool initialized = false;

        CaptureStatistics statistics = {
            .totalRequests = 0,
            .successfulCaptures = 0,
            .failedCaptures = 0,
            .lastFrameSize = 0,
            .lastCaptureTimeMs = 0};

        camera_config_t createConfig()
        {
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

            config.xclk_freq_hz =
                static_cast<int>(XCLK_FREQUENCY_HZ);

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

        bool applyInitialSensorSettings()
        {
            sensor_t *sensor = esp_camera_sensor_get();

            if (sensor == nullptr)
            {
                Serial.println(
                    "[CAMERA] Erro: descritor do sensor indisponivel.");
                return false;
            }

            bool success = true;

            if (sensor->set_framesize(sensor, INITIAL_FRAME_SIZE) != 0)
            {
                Serial.println(
                    "[CAMERA] Erro ao configurar resolucao VGA.");
                success = false;
            }

            if (sensor->set_quality(sensor, JPEG_QUALITY) != 0)
            {
                Serial.println(
                    "[CAMERA] Erro ao configurar qualidade JPEG.");
                success = false;
            }

            if (sensor->set_gain_ctrl(sensor, 1) != 0)
            {
                Serial.println(
                    "[CAMERA] Aviso: ganho automatico nao configurado.");
            }

            if (sensor->set_exposure_ctrl(sensor, 1) != 0)
            {
                Serial.println(
                    "[CAMERA] Aviso: exposicao automatica nao configurada.");
            }

            if (sensor->set_whitebal(sensor, 1) != 0)
            {
                Serial.println(
                    "[CAMERA] Aviso: balanco de branco nao configurado.");
            }

            if (sensor->set_awb_gain(sensor, 1) != 0)
            {
                Serial.println(
                    "[CAMERA] Aviso: ganho AWB nao configurado.");
            }

            if (sensor->set_brightness(sensor, 0) != 0)
            {
                Serial.println(
                    "[CAMERA] Aviso: brilho nao configurado.");
            }

            if (sensor->set_contrast(sensor, 0) != 0)
            {
                Serial.println(
                    "[CAMERA] Aviso: contraste nao configurado.");
            }

            if (sensor->set_saturation(sensor, 0) != 0)
            {
                Serial.println(
                    "[CAMERA] Aviso: saturacao nao configurada.");
            }

            if (sensor->set_lenc(sensor, 1) != 0)
            {
                Serial.println(
                    "[CAMERA] Aviso: correcao de lente nao configurada.");
            }

            if (sensor->set_raw_gma(sensor, 1) != 0)
            {
                Serial.println(
                    "[CAMERA] Aviso: gamma automatico nao configurado.");
            }

            if (sensor->set_bpc(sensor, 1) != 0)
            {
                Serial.println(
                    "[CAMERA] Aviso: correcao de pixel preto nao configurada.");
            }

            if (sensor->set_wpc(sensor, 1) != 0)
            {
                Serial.println(
                    "[CAMERA] Aviso: correcao de pixel branco nao configurada.");
            }

            return success;
        }

    } // namespace

    bool begin()
    {
        if (initialized)
        {
            return true;
        }

        if (!psramFound())
        {
            Serial.println("[CAMERA] Erro: PSRAM nao encontrada.");
            return false;
        }

        camera_config_t config = createConfig();

        Serial.println("[CAMERA] Inicializando OV5640...");
        Serial.printf(
            "[CAMERA] XCLK: %d Hz\n",
            config.xclk_freq_hz);

        const esp_err_t result = esp_camera_init(&config);

        if (result != ESP_OK)
        {
            Serial.printf(
                "[CAMERA] Falha na inicializacao: 0x%04X (%s)\n",
                static_cast<unsigned int>(result),
                esp_err_to_name(result));

            return false;
        }

        initialized = true;

        if (!applyInitialSensorSettings())
        {
            Serial.println(
                "[CAMERA] Falha ao aplicar configuracoes iniciais.");

            esp_camera_deinit();
            initialized = false;
            return false;
        }

        const uint16_t pid = getSensorPid();

        Serial.printf("[CAMERA] Sensor PID: 0x%04X\n", pid);

        if (pid != 0x5640)
        {
            Serial.println(
                "[CAMERA] Aviso: sensor nao identificado como OV5640.");
        }

        // Descarta um primeiro frame para permitir convergência
        // inicial de exposição e balanço de branco.
        camera_fb_t *warmupFrame = esp_camera_fb_get();

        if (warmupFrame != nullptr)
        {
            esp_camera_fb_return(warmupFrame);
            warmupFrame = nullptr;
        }

        Serial.println("[CAMERA] Inicializacao concluida.");
        return true;
    }

    bool isInitialized()
    {
        return initialized;
    }

    camera_fb_t *acquireFrame()
    {
        ++statistics.totalRequests;

        if (!initialized)
        {
            ++statistics.failedCaptures;
            return nullptr;
        }

        const uint32_t startTime = millis();

        camera_fb_t *frame = esp_camera_fb_get();

        statistics.lastCaptureTimeMs = millis() - startTime;

        if (frame == nullptr)
        {
            ++statistics.failedCaptures;
            statistics.lastFrameSize = 0;
            return nullptr;
        }

        ++statistics.successfulCaptures;
        statistics.lastFrameSize = frame->len;

        return frame;
    }

    void releaseFrame(camera_fb_t *&frame)
    {
        if (frame == nullptr)
        {
            return;
        }

        esp_camera_fb_return(frame);
        frame = nullptr;
    }

    sensor_t *getSensor()
    {
        if (!initialized)
        {
            return nullptr;
        }

        return esp_camera_sensor_get();
    }

    uint16_t getSensorPid()
    {
        sensor_t *sensor = getSensor();

        if (sensor == nullptr)
        {
            return 0;
        }

        return sensor->id.PID;
    }

    const CaptureStatistics &getStatistics()
    {
        return statistics;
    }

} // namespace CameraService