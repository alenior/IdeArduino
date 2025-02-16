#include <Wire.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>
#include <Keypad.h>
#include <MFRC522.h>

#define BLYNK_TEMPLATE_ID           "TMPL2Nvh-55Cv"
#define BLYNK_TEMPLATE_NAME         "Quickstart Template"
#define BLYNK_AUTH_TOKEN            "aNC-dTS08hARfXUIET1-fu_4GkUxP8Jj"
#define relayPin 13

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

#include <HTTPClient.h>
#include <UrlEncode.h>

// +international_country_code + phone number
// Brasil +55, example: +5585987288807
String phoneNumber = "+558587288807";
String apiKey = "2272372";

void sendMessage(String message){

  // Data to send with HTTP POST
  String url = "https://api.callmebot.com/whatsapp.php?phone=" + phoneNumber + "&apikey=" + apiKey + "&text=" + urlEncode(message);    
  HTTPClient http;
  http.begin(url);

  // Specify content-type header
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  
  // Send HTTP POST request
  int httpResponseCode = http.POST(url);
  if (httpResponseCode == 200){
    Serial.print("Message sent successfully");
  }
  else{
    Serial.println("Error sending the message");
    Serial.print("HTTP response code: ");
    Serial.println(httpResponseCode);
  }

  // Free resources
  http.end();
}

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "GCNET-Alencar";
char pass[] = "11223344";

BlynkTimer timer;

// Variáveis globais para controle do temporizador
unsigned long tempoAnterior = 0; // Armazena o tempo inicial
bool relayAtivado = false;       // Indica se o relé foi ativado

// This function is called every time the Virtual Pin 0 state changes (DA PLATAFORMA BLYNK PARA A PLACA ESP32)
BLYNK_WRITE(V0) {
  // Verifica se o valor recebido é 1 (relé ativado)
  if (param.asInt() == 1) {
    digitalWrite(relayPin, HIGH); // Ativa o relé

    // Send Message to WhatsAPP
    sendMessage("Porta aberta remotamente!");

    relayAtivado = true;          // Marca que o relé foi ativado
    tempoAnterior = millis();     // Registra o tempo atual
    Blynk.virtualWrite(V0, 1);   // Atualiza o estado do botão no Blynk
  } else {
    digitalWrite(relayPin, LOW);  // Desativa o relé (caso o botão seja desligado manualmente)

    // Send Message to WhatsAPP
    sendMessage("Porta fechada remotamente!");

    relayAtivado = false;         // Reseta o estado do relé
    Blynk.virtualWrite(V0, 0);   // Atualiza o estado do botão no Blynk
  }
}

// This function is called every time the device is connected to the Blynk.Cloud
BLYNK_CONNECTED()
{
  // Change Web Link Button message to "Congratulations!"
  Blynk.setProperty(V3, "offImageUrl", "https://static-image.nyc3.cdn.digitaloceanspaces.com/general/fte/congratulations.png");
  Blynk.setProperty(V3, "onImageUrl",  "https://static-image.nyc3.cdn.digitaloceanspaces.com/general/fte/congratulations_pressed.png");
  Blynk.setProperty(V3, "url", "https://docs.blynk.io/en/getting-started/what-do-i-need-to-blynk/how-quickstart-device-was-made");
}

// This function sends Arduino's uptime every second to Virtual Pin 2. (DA PLACA ESP32 PARA A PLATAFORMA BLYNK)
void myTimerEvent()
{
  // You can send any value at any time.
  // Please don't send more that 10 values per second.
  Blynk.virtualWrite(V2, millis() / 1000);
}

// Configurações de hardware
#define RST_PIN 22
#define SS_PIN 21
MFRC522 rfid(SS_PIN, RST_PIN);

hd44780_I2Cexp lcd(0x27, 16, 2);

const byte ROWS = 4;
const byte COLS = 3;
char keys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};
byte rowPins[ROWS] = {32, 33, 25, 26};
byte colPins[COLS] = {27, 14, 12};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

const int ledRed = 2;
const int ledYellow = 5;
const int ledGreen = 16;
const int buzzer = 17;
// const int relayPin = 13;
bool senhaDigitada = false;

// Senha padrão
String senha = "123456";
String entradaSenha = "";

// Lista de IDs de cartões autorizados
const int numCartoesAutorizados = 2;
String cartoesAutorizados[numCartoesAutorizados] = {
  "52 7D 83 54",  // ID do cartão
  // "9F 18 32 28"   // ID da tag
};

// Funções auxiliares
void acessoNegado() {
  lcd.clear();
  lcd.noCursor();
  lcd.print("Senha errada!");
  lcd.setCursor(0, 1);
  lcd.print("Acesso negado!");
  digitalWrite(buzzer, HIGH);
  delay(500);
  digitalWrite(buzzer, LOW);
  delay(250);
  digitalWrite(buzzer, HIGH);
  delay(500);
  digitalWrite(buzzer, LOW);

  Serial.println("Senha errada! Acesso não autorizado!");

  delay(2000);
  lcd.clear();
  lcd.print("Acesso restrito!");
  lcd.setCursor(0, 1);
  lcd.print("Credencial?");
}

void acessoLiberado() {
  lcd.clear();
  lcd.noCursor();
  lcd.print("Acesso liberado.");
  lcd.setCursor(0, 1);
  lcd.print("Bem vindo!");

  Serial.println("Acesso autorizado.");

  digitalWrite(ledRed, LOW);
  digitalWrite(ledYellow, LOW);
  digitalWrite(ledGreen, HIGH);
  digitalWrite(relayPin, HIGH);

  // Send Message to WhatsAPP
  sendMessage("Porta aberta!");

  digitalWrite(buzzer, HIGH);
  delay(100);
  digitalWrite(buzzer, LOW);
  delay(3000);
  digitalWrite(relayPin, LOW);

  // Send Message to WhatsAPP
  sendMessage("Porta fechada!");

  digitalWrite(ledGreen, LOW);
  digitalWrite(ledRed, HIGH);
  lcd.clear();
  lcd.print("Acesso restrito!");
  lcd.setCursor(0, 1);
  lcd.print("Credencial?");
}

bool verificarCartaoAutorizado(String idCartao) {
  for (int i = 0; i < numCartoesAutorizados; i++) {
    if (cartoesAutorizados[i] == idCartao) {
      lcd.clear();
      lcd.print("Usuario master");
      lcd.setCursor(0, 1);
      lcd.print("autorizado.");
      delay(1000);
      return true;
    }
  }
  return false;
}

void setup() {

  // Debug console
  // Serial.begin(115200);

  WiFi.begin(ssid, pass);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  // You can also specify server:
  //Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass, "blynk.cloud", 80);
  //Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass, IPAddress(192,168,1,100), 8080);

  // Setup a function to be called every second
  timer.setInterval(1000L, myTimerEvent);

  // Configuração inicial
  Wire.begin(4, 15); // Inicializa I2C no LCD, com portas diferentes das padrões (21 e 22).
  lcd.begin(16, 2);
  lcd.backlight();
  lcd.clear();
  lcd.print("Acesso restrito!");
  lcd.setCursor(0, 1);
  lcd.print("Credencial?");

  pinMode(ledRed, OUTPUT);
  pinMode(ledYellow, OUTPUT);
  pinMode(ledGreen, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(relayPin, OUTPUT);

  digitalWrite(ledRed, HIGH);

  SPI.begin();
  rfid.PCD_Init();

  Serial.begin(9600);
  Serial.println("Sistema ativo: aguardando tentativa de acesso.");
}

void loop() {
  Blynk.run();
  timer.run();

  // Verifica se o relé foi ativado e se já se passaram 3 segundos
  if (relayAtivado && (millis() - tempoAnterior >= 3000)) {
    digitalWrite(relayPin, LOW); // Desativa o relé

    // Send Message to WhatsAPP
    sendMessage("Porta fechada remotamente!");

    relayAtivado = false;        // Reseta o estado do relé
    Blynk.virtualWrite(V0, 0);  // Atualiza o estado do botão no Blynk
  }

  // You can inject your own code or combine it with other sketches.
  // Check other examples on how to communicate with Blynk. Remember
  // to avoid delay() function!

  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return;
  }

  String idCartao = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    idCartao += String(rfid.uid.uidByte[i], HEX);
    if (i < rfid.uid.size - 1) idCartao += " ";
  }
  idCartao.toUpperCase();

  Serial.print("Cartão detectado: ");
  Serial.println(idCartao);

  if (verificarCartaoAutorizado(idCartao)) {
    Serial.println("Usuario máster identificado.");
    acessoLiberado(); // Libera acesso sem senha para cartões autorizados

    // Send Message to WhatsAPP
    sendMessage("Acesso liberado ao usuário máster!");

    Serial.println("Sistema ativo: aguardando tentativa de acesso.");
  } else {
    lcd.clear();
    lcd.print("Tag detectada.");
    lcd.setCursor(0, 1);
    lcd.print("Exige senha!");

    digitalWrite(ledYellow, HIGH);
    delay(2000);

    entradaSenha = "";
    lcd.clear();
    lcd.print("Informe senha:");
    lcd.setCursor(0, 1);

    while (true) {
      lcd.cursor();
      char key = keypad.getKey();

      if (key) {
        if (key == '*') {
          Serial.println("");
          if (entradaSenha == senha) {
            acessoLiberado();

            // Send Message to WhatsAPP
            sendMessage("Acesso liberado por senha!");

          } else {
            acessoNegado();
            // Send Message to WhatsAPP
            sendMessage("Tentativa de acesso negada: senha errada!");
          }
          Serial.println("Sistema ativo: aguardando tentativa de acesso.");
          break;
        } else if (key == '#') {
          if (entradaSenha.length() > 0) {
            entradaSenha.remove(entradaSenha.length() - 1);

            digitalWrite(buzzer, HIGH);
            delay(250);
            digitalWrite(buzzer, LOW);

            lcd.setCursor(entradaSenha.length(), 1);
            lcd.print(" ");
            lcd.setCursor(entradaSenha.length(), 1);

            if (senhaDigitada == false) {
              Serial.print("Senha informada: ");
              senhaDigitada = true;
            }

            if (key != '#') {
              Serial.print(key);
            }
          }
        } else {
          if (entradaSenha.length() < 6) {
            entradaSenha += key;
            lcd.print("*");
            digitalWrite(buzzer, HIGH);
            delay(250);
            digitalWrite(buzzer, LOW);

            if (senhaDigitada == false) {
              Serial.print("Senha informada: ");
              senhaDigitada = true;
            }
            if (key != '#') {
              Serial.print(key);
            }
          }
        }
      }
      lcd.noCursor();
    }

    senhaDigitada = false;

    digitalWrite(ledYellow, LOW);
  }
  rfid.PICC_HaltA();
}