#include <WiFi.h>
#include <HTTPClient.h>
#include <Update.h>

// Configurações do Wi-Fi Simulado
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// URL do firmware v2 no seu GitHub
const char* firmware_url = "https://raw.githubusercontent.com/hugobarra/esp32-fota-simulation/main/firmware_v2.bin";

#define LED_PIN 2
#define CURRENT_VERSION 1.0

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);

  Serial.println("\n=================================");
  Serial.printf(" FIRMWARE ATUAL: Versao %.1f\n", CURRENT_VERSION);
  Serial.println("=================================");

  WiFi.begin(ssid, password);
  Serial.print("Conectando ao Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi Conectado!");

  checkAndApplyOTA();
}

void loop() {
  // Versão 1.0: Pisca DEVAGAR (a cada 1 segundo)
  digitalWrite(LED_PIN, HIGH);
  delay(1000);
  digitalWrite(LED_PIN, LOW);
  delay(1000);
  Serial.println("[v1.0] Executando loop principal...");
}

void checkAndApplyOTA() {
  Serial.println("\n[FOTA] Verificando se existe nova versao...");
  HTTPClient http;
  http.begin(firmware_url);
  
  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK) {
    int contentLength = http.getSize();
    Serial.printf("[FOTA] Novo firmware encontrado! Tamanho: %d bytes\n", contentLength);

    bool canBegin = Update.begin(contentLength);
    if (canBegin) {
      Serial.println("[FOTA] Gravando firmware na memoria flash...");
      WiFiClient* client = http.getStreamPtr();
      size_t written = Update.writeStream(*client);

      if (written == contentLength) {
        Serial.println("[FOTA] Gravacao concluida com sucesso!");
      } else {
        Serial.printf("[FOTA] Erro: Gravados apenas %d/%d bytes\n", written, contentLength);
      }

      if (Update.end()) {
        if (Update.isFinished()) {
          Serial.println("[FOTA] Atualizacao concluida! Reiniciando ESP32...\n");
          ESP.restart();
        }
      } else {
        Serial.printf("[FOTA] Erro ao finalizar OTA: %s\n", Update.errorString());
      }
    } else {
      Serial.println("[FOTA] Espaco insuficiente na flash para o OTA.");
    }
  } else {
    Serial.printf("[FOTA] Nenhum firmware v2.0 disponivel no GitHub (HTTP %d).\n", httpCode);
  }
  http.end();
}