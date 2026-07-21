#include <Arduino.h>
#include "esp_heap_caps.h"
#include "esp_system.h"
#include <inttypes.h>

namespace PsramTest {

constexpr uint32_t SERIAL_BAUD_RATE = 115200;

// Testaremos um bloco suficientemente grande para confirmar
// que a alocação realmente está sendo feita na PSRAM.
constexpr size_t TEST_BUFFER_SIZE = 4 * 1024 * 1024;

// Padrão determinístico usado para validar toda a memória.
uint8_t expectedValue(size_t index)
{
    return static_cast<uint8_t>(
        ((index * 31U) + (index >> 8U) + 0x5AU) & 0xFFU
    );
}

void printBytes(const char* label, size_t bytes)
{
    Serial.printf(
        "%-32s: %10lu bytes  (%7.2f MB)\n",
        label,
        static_cast<unsigned long>(bytes),
        static_cast<double>(bytes) / (1024.0 * 1024.0)
    );
}

void printMemoryStatus()
{
    Serial.println();
    Serial.println("========== ESTADO DA MEMORIA ==========");

    printBytes("Heap total", ESP.getHeapSize());
    printBytes("Heap livre", ESP.getFreeHeap());
    printBytes("Menor heap livre", ESP.getMinFreeHeap());
    printBytes("Maior bloco interno",
               heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL));

    printBytes("PSRAM total", ESP.getPsramSize());
    printBytes("PSRAM livre", ESP.getFreePsram());

    printBytes(
        "PSRAM total via heap_caps",
        heap_caps_get_total_size(MALLOC_CAP_SPIRAM)
    );

    printBytes(
        "PSRAM livre via heap_caps",
        heap_caps_get_free_size(MALLOC_CAP_SPIRAM)
    );

    printBytes(
        "Maior bloco livre PSRAM",
        heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM)
    );

    printBytes(
        "Menor PSRAM livre",
        heap_caps_get_minimum_free_size(MALLOC_CAP_SPIRAM)
    );

    Serial.println("=======================================");
    Serial.println();
}

bool testPsramBuffer()
{
    Serial.printf(
    "[TESTE] Solicitando bloco de %lu bytes na PSRAM...\n",
    static_cast<unsigned long>(TEST_BUFFER_SIZE)
    );

    uint8_t* buffer = static_cast<uint8_t*>(
        heap_caps_malloc(
            TEST_BUFFER_SIZE,
            MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT
        )
    );

    if (buffer == nullptr) {
        Serial.println("[ERRO] Nao foi possivel alocar o bloco de teste.");
        return false;
    }

    if (!esp_ptr_external_ram(buffer)) {
        Serial.println(
            "[ERRO] O bloco foi alocado, mas nao esta localizado na PSRAM."
        );

        free(buffer);
        return false;
    }

    Serial.printf(
        "[OK] Bloco alocado na PSRAM. Endereco: %p\n",
        static_cast<void*>(buffer)
    );

    Serial.println("[TESTE] Gravando padrao em toda a memoria...");

    uint32_t startWrite = millis();

    for (size_t index = 0; index < TEST_BUFFER_SIZE; ++index) {
        buffer[index] = expectedValue(index);
    }

    uint32_t writeDuration = millis() - startWrite;

    Serial.printf(
        "[OK] Escrita concluida em %lu ms.\n",
        static_cast<unsigned long>(writeDuration)
    );

    Serial.println("[TESTE] Lendo e verificando toda a memoria...");

    uint32_t startRead = millis();
    size_t errorCount = 0;
    constexpr size_t MAX_REPORTED_ERRORS = 10;

    for (size_t index = 0; index < TEST_BUFFER_SIZE; ++index) {
        const uint8_t expected = expectedValue(index);
        const uint8_t received = buffer[index];

        if (received != expected) {
            if (errorCount < MAX_REPORTED_ERRORS) {
                Serial.printf(
                    "[ERRO] Posicao %lu: esperado 0x%02X, recebido 0x%02X\n",
                    static_cast<unsigned long>(index),
                    expected,
                    received
                );
            }

            ++errorCount;
        }
    }

    uint32_t readDuration = millis() - startRead;

    Serial.printf(
        "[INFO] Leitura concluida em %lu ms.\n",
        static_cast<unsigned long>(readDuration)
    );

    free(buffer);
    buffer = nullptr;

    Serial.println("[OK] Bloco liberado.");

    if (errorCount != 0) {
        Serial.printf(
            "[FALHA] Foram encontrados %lu erros de memoria.\n",
            static_cast<unsigned long>(errorCount)
        );

        return false;
    }

    Serial.println("[OK] Nenhum erro de leitura ou escrita encontrado.");
    return true;
}

}  // namespace PsramTest

void setup()
{
    Serial.begin(PsramTest::SERIAL_BAUD_RATE);

    // Necessário principalmente quando USB CDC está habilitado.
    const uint32_t waitStart = millis();

    while (!Serial && millis() - waitStart < 5000) {
        delay(10);
    }

    delay(1000);

    Serial.println();
    Serial.println("=======================================");
    Serial.println("     TESTE DE PSRAM DO ESP32-S3");
    Serial.println("=======================================");

    Serial.printf(
        "Chip                : %s\n",
        ESP.getChipModel()
    );

    Serial.printf(
        "Revisao             : %u\n",
        ESP.getChipRevision()
    );

    Serial.printf(
        "Nucleos              : %u\n",
        ESP.getChipCores()
    );

    Serial.printf(
        "Frequencia CPU       : %" PRIu32 " MHz\n",
        ESP.getCpuFreqMHz()
    );

    Serial.printf(
        "Flash detectada      : %u bytes\n",
        static_cast<unsigned int>(ESP.getFlashChipSize())
    );

    if (!psramFound()) {
        Serial.println();
        Serial.println("[FALHA] PSRAM nao detectada.");
        Serial.println();
        Serial.println("Verifique na Arduino IDE:");
        Serial.println("1. Tools > PSRAM > OPI PSRAM");
        Serial.println("2. Selecao correta da placa ESP32-S3");
        Serial.println("3. Configuracao de Flash/PSRAM do modulo");
        Serial.println("4. Alimentacao e cabo USB");
        Serial.println();
        return;
    }

    Serial.println();
    Serial.println("[OK] PSRAM detectada.");

    PsramTest::printMemoryStatus();

    const bool testPassed = PsramTest::testPsramBuffer();

    PsramTest::printMemoryStatus();

    Serial.println("=======================================");

    if (testPassed) {
        Serial.println("[RESULTADO] PSRAM APROVADA.");
        Serial.println(
            "A placa esta pronta para o teste isolado da OV5640."
        );
    } else {
        Serial.println("[RESULTADO] PSRAM REPROVADA.");
        Serial.println(
            "Nao prossiga para a camera antes de corrigir a configuracao."
        );
    }

    Serial.println("=======================================");
}

void loop()
{
    // Pisca o LED integrado apenas como sinal de que o firmware continua ativo.
    // Conforme a pinagem informada, LED ON está ligado ao GPIO 2.
    constexpr uint8_t LED_PIN = 2;

    static bool initialized = false;

    if (!initialized) {
        pinMode(LED_PIN, OUTPUT);
        initialized = true;
    }

    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    delay(1000);
}