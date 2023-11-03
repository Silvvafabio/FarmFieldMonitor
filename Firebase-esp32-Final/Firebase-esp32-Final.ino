#include <WiFi.h>
#include "time.h"
#include "sntp.h"
#include <IOXhop_FirebaseESP32.h>
#include <cJSON.h>
#include "DHT.h"

#define DHTPIN 4  // pino de dados do DHT11
#define DHTTYPE DHT22 // define o tipo de sensor, no caso DHT22
DHT dht(DHTPIN, DHTTYPE);

// Autenticação Firebase
#define FIREBASE_HOST "exemplo.firebaseio.com"
#define FIREBASE_AUTH "Chave de autenticação do firebase"



const char* ssid = "SSID"; // Nome ddo Wi-fi
const char* password = "PASSWORD"; // senha do wi-fi
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -3 * 3600; // GMT-3 (3 horas de diferença em relação ao UTC) para o horário de Brasília

struct tm timeinfo;  // Estrutura de tempo global

void printLocalTime() {
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.printf("%04d/%02d/%02d %02d:%02d:%02d\n", timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
}

String getLocalTimeAsString() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return "Failed to obtain time";
  }
  
  char buffer[100];  // Buffer para armazenar a data e hora formatada
  strftime(buffer, sizeof(buffer), "%Y/%m/%d/ %H/%M:%S/", &timeinfo);
  
  return String(buffer);
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  
  // Configuração do NTP para o horário de Brasília
  configTime(gmtOffset_sec, 0, ntpServer);
  sntp_set_time_sync_notification_cb([](struct timeval *tv) {
    Serial.println("Got time adjustment from NTP!");
    printLocalTime();
  });

  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" CONNECTED");

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Serial.print("Conectando a ");
  Serial.println(FIREBASE_HOST);
}

void loop() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  
  String time = getLocalTimeAsString();
  String temperatura = time + " Temperatura";
  Firebase.setFloat(temperatura, t);
  String umidade = time + " Umidade";
  Firebase.setFloat(umidade, h);
  Firebase.setFloat("atual/Temperatura", t);
  Firebase.setFloat("atual/Umidade", h);

  
}
