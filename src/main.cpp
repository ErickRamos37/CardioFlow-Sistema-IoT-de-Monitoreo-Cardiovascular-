#include <Arduino.h>

// Librerias Para el Sensor MAX30102
#include <Wire.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"

// Librerias para conexión a WiFi y a Firebase (Base de Datos)
#include<WiFi.h>
#include<FirebaseESP32.h>

// Libreria con las constantes para conexión a WiFi y a Firebase
#include"config/config.h"

// Constantes para conexión a WiFi y a Firebase
/*
#define wifi  "Tu_Red_WiFi"
#define contra "Tu_Password_Seguro"
#define URL "TU_URL_DE_FIREBASE"
#define secreto "Tu_Secreto"
*/

// Variables globales de para la configuración del Firebase (instancia de la clase Firebase)
FirebaseData myFireBaseData;
FirebaseConfig firebaseConfig;
FirebaseAuth firebaseAuth;

// Variables globales de para la configuración del sensor MAX30102 (instancia de la clase MAX30105)
MAX30105 particleSensor;

// El Tamaño Seguro Para La Librería Es de 100 muestras
#define BUFFER_SIZE 100
uint32_t irBuffer[BUFFER_SIZE];
uint32_t redBuffer[BUFFER_SIZE];

int32_t bufferLength = BUFFER_SIZE;
int32_t spo2;
int8_t validSPO2;
int32_t heartRate;
int8_t validHeartRate;

int32_t anterior_heartRate = 0;
int8_t anterior_spo2 = 0;

// Pin del LED
byte readLED = 2;

void setup() {
  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASS); // Conexión a WiFi
  pinMode(readLED, OUTPUT);

  Serial.print("Conectando a la red: ");
  Serial.println(WIFI_SSID);

  // Estableciendo Conexión al WiFi
  while(WiFi.status() != WL_CONNECTED) {
    Serial.print(". ");
    delay(500);
  }
  Serial.println("Conexión WiFi exitosa");
  
  firebaseConfig.database_url = FIREBASE_URL;
  firebaseConfig.signer.tokens.legacy_token = FIREBASE_SECRET;
  Firebase.begin(&firebaseConfig, &firebaseAuth);
  Firebase.reconnectWiFi(true);

  // Inicializar sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println(F("MAX30105 no fue encontrado. Revise cableado/alimentación."));
    while (1);
  }

  Serial.println("Fije el sensor.");
  // Serial.println(F("Fije el sensor. Presione cualquier tecla para iniciar."));
  // while (Serial.available() == 0);
  // Serial.read();

  // Configración más optima para sensor MAX30102 con el ESP32
  byte ledBrightness = 200;  // Alta potencia (Ajustar para la saturación)
  byte sampleAverage = 1;    // Sin promediado
  byte ledMode = 2;          // Rojo + IR
  byte sampleRate = 200;     // 200 SPS
  int pulseWidth = 411;      // Pulso más ancho
  int adcRange = 16384;      // Máxima resolución

  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);
}

void loop() {
  // Bloque 1: Llenar el buffer inicial con 100 muestras
  Serial.println(F("Llenando buffer inicial..."));

  for (byte i = 0; i < bufferLength; i++) {
    while (particleSensor.available() == false)
    particleSensor.check();
    redBuffer[i] = particleSensor.getRed();
    irBuffer[i] = particleSensor.getIR();
    particleSensor.nextSample();
  }
  Serial.println("Buffer Lleno. Iniciando cálculo continuo.");

  // Cálculo inicial.
  maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);

  // Bloque 2: Lectura continua y cálculo
  // Actualizaremos cada 0.25 segundos (50 muestras a 200 SPS).
  const byte SAMPLES_TO_UPDATE = 50;
  const byte NEW_SAMPLES_START_INDEX = BUFFER_SIZE - SAMPLES_TO_UPDATE;  // 100 - 50 = 50

  while (1) {
    // 1. Desplazar: Mover las últimas 50 muestras al inicio del array.
    for (byte i = SAMPLES_TO_UPDATE; i < BUFFER_SIZE; i++) {
      redBuffer[i - SAMPLES_TO_UPDATE] = redBuffer[i];
      irBuffer[i - SAMPLES_TO_UPDATE] = irBuffer[i];
    }

    // 2. Leer: Tomar 50 conjuntos de muestras nuevas, empezando en el índice 50.
    for (byte i = NEW_SAMPLES_START_INDEX; i < BUFFER_SIZE; i++) {
      while (particleSensor.available() == false)
      particleSensor.check();

      digitalWrite(readLED, !digitalRead(readLED));

      redBuffer[i] = particleSensor.getRed();
      irBuffer[i] = particleSensor.getIR();
      particleSensor.nextSample();
    }

    // 3. Recalcular HR y SP02
    maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);

    // 4. Imprimir y enviar resultados filtrados (solo si VHR y VSPO2 son 1)
    if (validHeartRate == 1 && validSPO2 == 1) {
      // Salida Limpia: Lista para enviar a la Base de Datos
      Serial.print(F("HR_OK="));
      Serial.print(heartRate, DEC);
      Serial.print(F(",SPO2_OK="));
      Serial.print(spo2, DEC);
      Serial.println();

      // Enviar datos a la nube (Si cambio el valor)
      if(heartRate != anterior_heartRate && heartRate > 59 && heartRate < 150) {
        // if(heartRate != anterior_heartRate) {
        if(WiFi.status() == WL_CONNECTED) {
            if (Firebase.setInt(myFireBaseData, "/Monitoreo/HeartRate", heartRate)) {
              Serial.print("Ritmo Cardiaco: ");
              Serial.println(heartRate);
            } else {
              Serial.print("Error de conexión: ");
              Serial.println(myFireBaseData.errorReason());
            }
            anterior_heartRate = heartRate;
            // delay(1000);
        }
      }
      if(spo2 != anterior_spo2 && heartRate > 59 && heartRate < 150) {
      // if(spo2 != anterior_spo2) {
        if(WiFi.status() == WL_CONNECTED) {
            if (Firebase.setInt(myFireBaseData, "/Monitoreo/SPO2", spo2)) {
              Serial.print("Oxigenación En la Sangré: ");
              Serial.println(spo2);
            } else {
              Serial.print("Error de conexión: ");
              Serial.println(myFireBaseData.errorReason());
            }
            anterior_spo2 = spo2;
            // delay(1000);
        }
      }
    } else {
      // Señal inválida
      Serial.println(F("HR_NOK=-999,SPO2_NOK=-999"));
    }
  }
}