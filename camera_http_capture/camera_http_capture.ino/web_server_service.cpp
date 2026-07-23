#include "web_server_service.h"

#include <Arduino.h>
#include <inttypes.h>

#include "esp_camera.h"
#include "esp_http_server.h"
#include "esp_timer.h"

#include "camera_service.h"

namespace WebServerService
{

    namespace
    {

        httpd_handle_t server = nullptr;

        const char INDEX_HTML[] PROGMEM = R"HTML(
<!DOCTYPE html>
<html lang="pt-BR">
<head>
    <meta charset="utf-8">
    <meta
        name="viewport"
        content="width=device-width,initial-scale=1"
    >

    <title>ESP32-S3 OV5640</title>

    <style>
        :root {
            color-scheme: dark;
            font-family: Arial, Helvetica, sans-serif;
        }

        body {
            max-width: 900px;
            margin: 0 auto;
            padding: 24px;
            background: #111827;
            color: #f3f4f6;
        }

        h1 {
            margin-bottom: 8px;
        }

        .description {
            color: #cbd5e1;
            margin-bottom: 24px;
        }

        .panel {
            padding: 18px;
            border: 1px solid #374151;
            border-radius: 12px;
            background: #1f2937;
        }

        img {
            display: block;
            width: 100%;
            max-width: 640px;
            min-height: 240px;
            margin: 16px auto;
            border-radius: 8px;
            object-fit: contain;
            background: #000;
        }

        button,
        a.button {
            display: inline-block;
            margin: 6px;
            padding: 12px 18px;
            border: 0;
            border-radius: 8px;
            background: #2563eb;
            color: white;
            font-size: 16px;
            text-decoration: none;
            cursor: pointer;
        }

        button:hover,
        a.button:hover {
            background: #1d4ed8;
        }

        button:disabled {
            cursor: not-allowed;
            opacity: 0.55;
        }

        #status {
            margin-top: 16px;
            white-space: pre-wrap;
            font-family: monospace;
            color: #d1fae5;
        }
    </style>
</head>

<body>
    <h1>ESP32-S3 + OV5640</h1>

    <p class="description">
        Teste de captura JPEG individual pela rede local.
    </p>

    <section class="panel">
        <img
            id="cameraImage"
            alt="Imagem capturada pela OV5640"
        >

        <div>
            <button
                id="captureButton"
                type="button"
                onclick="captureImage()"
            >
                Capturar imagem
            </button>

            <a
                class="button"
                href="/capture"
                target="_blank"
                rel="noopener"
            >
                Abrir JPEG
            </a>

            <button type="button" onclick="loadStatus()">
                Atualizar status
            </button>
        </div>

        <div id="status">Aguardando consulta...</div>
    </section>

    <script>
    const image = document.getElementById("cameraImage");
    const statusElement = document.getElementById("status");
    const captureButton =
        document.getElementById("captureButton");

    let captureInProgress = false;
    let currentImageUrl = null;

    async function captureImage() {
        if (captureInProgress) {
            return;
        }

        captureInProgress = true;
        captureButton.disabled = true;
        captureButton.textContent = "Capturando...";
        statusElement.textContent =
            "Solicitando uma nova imagem...";

        const startTime = performance.now();

        try {
            const response = await fetch(
                "/capture?t=" + Date.now(),
                {
                    method: "GET",
                    cache: "no-store"
                }
            );

            if (!response.ok) {
                throw new Error(
                    "HTTP " + response.status
                );
            }

            const imageBlob = await response.blob();

            if (!imageBlob.type.startsWith("image/jpeg")) {
                throw new Error(
                    "Resposta recebida não é JPEG."
                );
            }

            const newImageUrl =
                URL.createObjectURL(imageBlob);

            image.onload = () => {
                if (currentImageUrl !== null) {
                    URL.revokeObjectURL(currentImageUrl);
                }

                currentImageUrl = newImageUrl;
            };

            image.src = newImageUrl;

            const elapsedTime =
                Math.round(performance.now() - startTime);

            statusElement.textContent =
                "Captura recebida: " +
                imageBlob.size +
                " bytes em " +
                elapsedTime +
                " ms.";
        } catch (error) {
            statusElement.textContent =
                "Falha na captura: " + error.message;
        } finally {
            captureInProgress = false;
            captureButton.disabled = false;
            captureButton.textContent =
                "Capturar imagem";
        }
    }

    async function loadStatus() {
        statusElement.textContent =
            "Consultando status...";

        try {
            const response = await fetch(
                "/status?t=" + Date.now(),
                {
                    cache: "no-store"
                }
            );

            if (!response.ok) {
                throw new Error(
                    "HTTP " + response.status
                );
            }

            const data = await response.json();

            statusElement.textContent =
                JSON.stringify(data, null, 2);
        } catch (error) {
            statusElement.textContent =
                "Falha: " + error.message;
        }
    }

    loadStatus();
</script>
</body>
</html>
)HTML";

        void setNoCacheHeaders(httpd_req_t *request)
        {
            httpd_resp_set_hdr(
                request,
                "Cache-Control",
                "no-store, no-cache, must-revalidate, max-age=0");

            httpd_resp_set_hdr(
                request,
                "Pragma",
                "no-cache");
        }

        esp_err_t indexHandler(httpd_req_t *request)
        {
            httpd_resp_set_type(
                request,
                "text/html; charset=utf-8");

            setNoCacheHeaders(request);

            return httpd_resp_send(
                request,
                INDEX_HTML,
                HTTPD_RESP_USE_STRLEN);
        }

        esp_err_t healthHandler(httpd_req_t *request)
        {
            httpd_resp_set_type(
                request,
                "text/plain; charset=utf-8");

            setNoCacheHeaders(request);

            if (!CameraService::isInitialized())
            {
                httpd_resp_set_status(
                    request,
                    "503 Service Unavailable");

                return httpd_resp_sendstr(
                    request,
                    "camera unavailable");
            }

            return httpd_resp_sendstr(request, "ok");
        }

        esp_err_t statusHandler(httpd_req_t *request)
        {
            const CameraService::CaptureStatistics &statistics =
                CameraService::getStatistics();

            sensor_t *sensor = CameraService::getSensor();

            const int sensorPid =
                sensor != nullptr
                    ? sensor->id.PID
                    : 0;

            char json[768];

            const int written = snprintf(
                json,
                sizeof(json),
                "{"
                "\"device\":{"
                "\"chip\":\"%s\","
                "\"cpu_mhz\":%" PRIu32 ","
                "\"uptime_ms\":%" PRIu32 ","
                "\"heap_free\":%lu,"
                "\"psram_total\":%lu,"
                "\"psram_free\":%lu"
                "},"
                "\"camera\":{"
                "\"initialized\":%s,"
                "\"sensor_pid\":\"0x%04X\","
                "\"resolution\":\"640x480\","
                "\"pixel_format\":\"JPEG\","
                "\"xclk_hz\":16000000,"
                "\"jpeg_quality\":12"
                "},"
                "\"capture\":{"
                "\"requests\":%" PRIu32 ","
                "\"successes\":%" PRIu32 ","
                "\"failures\":%" PRIu32 ","
                "\"last_frame_bytes\":%lu,"
                "\"last_capture_ms\":%" PRIu32
                "}"
                "}",
                ESP.getChipModel(),
                ESP.getCpuFreqMHz(),
                millis(),
                static_cast<unsigned long>(ESP.getFreeHeap()),
                static_cast<unsigned long>(ESP.getPsramSize()),
                static_cast<unsigned long>(ESP.getFreePsram()),
                CameraService::isInitialized() ? "true" : "false",
                sensorPid,
                statistics.totalRequests,
                statistics.successfulCaptures,
                statistics.failedCaptures,
                static_cast<unsigned long>(statistics.lastFrameSize),
                statistics.lastCaptureTimeMs);

            if (written < 0 ||
                static_cast<size_t>(written) >= sizeof(json))
            {

                Serial.println(
                    "[HTTP] Falha ao montar JSON de status.");

                httpd_resp_send_500(request);
                return ESP_FAIL;
            }

            httpd_resp_set_type(
                request,
                "application/json; charset=utf-8");

            setNoCacheHeaders(request);

            return httpd_resp_send(
                request,
                json,
                static_cast<ssize_t>(written));
        }

        esp_err_t captureHandler(httpd_req_t *request)
        {
            const int64_t requestStartUs =
                esp_timer_get_time();

            const int64_t captureStartUs =
                esp_timer_get_time();

            camera_fb_t *frame =
                CameraService::acquireFrame();

            const int64_t captureElapsedUs =
                esp_timer_get_time() - captureStartUs;

            if (frame == nullptr)
            {
                Serial.println(
                    "[HTTP] Falha ao adquirir frame.");

                httpd_resp_set_status(
                    request,
                    "500 Internal Server Error");

                httpd_resp_set_type(
                    request,
                    "text/plain; charset=utf-8");

                return httpd_resp_sendstr(
                    request,
                    "Falha ao capturar imagem.");
            }

            if (frame->format != PIXFORMAT_JPEG ||
                frame->buf == nullptr ||
                frame->len < 4)
            {

                Serial.println(
                    "[HTTP] Frame JPEG invalido.");

                CameraService::releaseFrame(frame);

                httpd_resp_set_status(
                    request,
                    "500 Internal Server Error");

                httpd_resp_set_type(
                    request,
                    "text/plain; charset=utf-8");

                return httpd_resp_sendstr(
                    request,
                    "Frame JPEG invalido.");
            }

            const bool validStartMarker =
                frame->buf[0] == 0xFF &&
                frame->buf[1] == 0xD8;

            const bool validEndMarker =
                frame->buf[frame->len - 2] == 0xFF &&
                frame->buf[frame->len - 1] == 0xD9;

            if (!validStartMarker || !validEndMarker)
            {
                Serial.println(
                    "[HTTP] Marcadores JPEG invalidos.");

                CameraService::releaseFrame(frame);

                httpd_resp_set_status(
                    request,
                    "500 Internal Server Error");

                httpd_resp_set_type(
                    request,
                    "text/plain; charset=utf-8");

                return httpd_resp_sendstr(
                    request,
                    "JPEG incompleto.");
            }

            const size_t frameLength = frame->len;
            const size_t frameWidth = frame->width;
            const size_t frameHeight = frame->height;

            esp_err_t responseResult =
                httpd_resp_set_type(
                    request,
                    "image/jpeg");

            if (responseResult == ESP_OK)
            {
                responseResult = httpd_resp_set_hdr(
                    request,
                    "Content-Disposition",
                    "inline; filename=\"capture.jpg\"");
            }

            if (responseResult == ESP_OK)
            {
                responseResult = httpd_resp_set_hdr(
                    request,
                    "Cache-Control",
                    "no-store, no-cache, must-revalidate, max-age=0");
            }

            if (responseResult == ESP_OK)
            {
                responseResult = httpd_resp_set_hdr(
                    request,
                    "Pragma",
                    "no-cache");
            }

            if (responseResult == ESP_OK)
            {
                responseResult = httpd_resp_set_hdr(
                    request,
                    "Access-Control-Allow-Origin",
                    "*");
            }

            const int64_t sendStartUs =
                esp_timer_get_time();

            if (responseResult == ESP_OK)
            {
                responseResult = httpd_resp_send(
                    request,
                    reinterpret_cast<const char *>(frame->buf),
                    frame->len);
            }

            const int64_t sendElapsedUs =
                esp_timer_get_time() - sendStartUs;

            // O buffer precisa permanecer válido até o término
            // de httpd_resp_send().
            CameraService::releaseFrame(frame);

            const int64_t totalElapsedUs =
                esp_timer_get_time() - requestStartUs;

            if (responseResult == ESP_OK)
            {
                Serial.printf(
                    "[HTTP] /capture OK | "
                    "%lux%lu | "
                    "%lu bytes | "
                    "captura=%" PRId64 " ms | "
                    "envio=%" PRId64 " ms | "
                    "total=%" PRId64 " ms\n",
                    static_cast<unsigned long>(frameWidth),
                    static_cast<unsigned long>(frameHeight),
                    static_cast<unsigned long>(frameLength),
                    captureElapsedUs / 1000,
                    sendElapsedUs / 1000,
                    totalElapsedUs / 1000);

                return ESP_OK;
            }

            Serial.printf(
                "[HTTP] /capture FALHOU | "
                "%lux%lu | "
                "%lu bytes | "
                "captura=%" PRId64 " ms | "
                "envio=%" PRId64 " ms | "
                "total=%" PRId64 " ms | "
                "erro=0x%04X (%s)\n",
                static_cast<unsigned long>(frameWidth),
                static_cast<unsigned long>(frameHeight),
                static_cast<unsigned long>(frameLength),
                captureElapsedUs / 1000,
                sendElapsedUs / 1000,
                totalElapsedUs / 1000,
                static_cast<unsigned int>(responseResult),
                esp_err_to_name(responseResult));

            return responseResult;
        }

        esp_err_t notFoundHandler(
            httpd_req_t *request,
            httpd_err_code_t error)
        {
            static_cast<void>(error);

            httpd_resp_set_status(request, "404 Not Found");

            httpd_resp_set_type(
                request,
                "application/json; charset=utf-8");

            return httpd_resp_sendstr(
                request,
                "{\"error\":\"endpoint_not_found\"}");
        }

        httpd_uri_t createGetUri(
            const char *path,
            esp_err_t (*handler)(httpd_req_t *))
        {
            httpd_uri_t uri = {};

            uri.uri = path;
            uri.method = HTTP_GET;
            uri.handler = handler;
            uri.user_ctx = nullptr;

            return uri;
        }

    } // namespace

    bool begin()
    {
        if (server != nullptr)
        {
            return true;
        }

        httpd_config_t config = HTTPD_DEFAULT_CONFIG();

        config.server_port = 80;
        config.ctrl_port = 32768;
        config.max_uri_handlers = 8;
        config.stack_size = 8192;
        config.lru_purge_enable = true;

        // Limita o tempo durante o qual um cliente lento ou desconectado
        // pode manter o handler bloqueado.
        config.recv_wait_timeout = 5;
        config.send_wait_timeout = 5;

        Serial.println("[HTTP] Iniciando servidor na porta 80...");

        const esp_err_t startResult =
            httpd_start(&server, &config);

        if (startResult != ESP_OK)
        {
            server = nullptr;

            Serial.printf(
                "[HTTP] Falha ao iniciar servidor: 0x%04X (%s)\n",
                static_cast<unsigned int>(startResult),
                esp_err_to_name(startResult));

            return false;
        }

        const httpd_uri_t indexUri =
            createGetUri("/", indexHandler);

        const httpd_uri_t captureUri =
            createGetUri("/capture", captureHandler);

        const httpd_uri_t statusUri =
            createGetUri("/status", statusHandler);

        const httpd_uri_t healthUri =
            createGetUri("/health", healthHandler);

        esp_err_t result = httpd_register_uri_handler(
            server,
            &indexUri);

        if (result == ESP_OK)
        {
            result = httpd_register_uri_handler(
                server,
                &captureUri);
        }

        if (result == ESP_OK)
        {
            result = httpd_register_uri_handler(
                server,
                &statusUri);
        }

        if (result == ESP_OK)
        {
            result = httpd_register_uri_handler(
                server,
                &healthUri);
        }

        if (result != ESP_OK)
        {
            Serial.printf(
                "[HTTP] Falha ao registrar endpoint: 0x%04X (%s)\n",
                static_cast<unsigned int>(result),
                esp_err_to_name(result));

            httpd_stop(server);
            server = nullptr;
            return false;
        }

        httpd_register_err_handler(
            server,
            HTTPD_404_NOT_FOUND,
            notFoundHandler);

        Serial.println("[HTTP] Servidor iniciado.");
        return true;
    }

    bool isRunning()
    {
        return server != nullptr;
    }

} // namespace WebServerService