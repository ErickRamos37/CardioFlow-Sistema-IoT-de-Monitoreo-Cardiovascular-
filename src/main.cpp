#include <Arduino.h>

// Libreria con las constantes para conexión a WiFi y a Firebase
#include "config/config.h"
// Libreria con la configuración y funciones para Firebase
#include "services/firebase.h"
// Libreria con la conección al wifi
#include "services/wifiManager.h"
// Libreria con la configuración y funciones para el sensor MAX30102
#include "sensors/max30102.h"

// Variables globales para HR y SP02
int32_t heartRate;
int32_t spo2;

// Variables de validación de señal
int8_t validHeartRate;
int8_t validSPO2;


// Variables para almacenar los valores anteriores y evitar envíos repetidos
int32_t previousHeartRate = 0;
int8_t previousSPO2 = 0;

void setup() {
  Serial.begin(115200);
  pinMode(readLED, OUTPUT);

  // Conección al WiFi
  connectionWiFi(WIFI_SSID, WIFI_PASS);

  // Conección a la base de datos (Firebase)
  connectionFirebase(FIREBASE_URL, FIREBASE_SECRET);

  // Inicualizacion y configuración del sensor MAX30102
  initializationMAX30102();

  // Bloque 1: Llenar el buffer inicial con 100 muestras y hace el cálculo inicial HR y SP02
  Serial.println("Llenando buffer inicial...");
  prechargeBuffer(&heartRate, &spo2, &validHeartRate, &validSPO2);
  Serial.println("Iniciando cálculo continuo.");
}

void loop() {
  // Bloque 2: Lectura continua y cálculo
  // Desplaza, obtiene 50 nuevas muestras y Recalcular HR y SP02
  updateSamples(&heartRate, &spo2, &validHeartRate, &validSPO2);

  // 4. Imprimir y enviar resultados filtrados (solo si VHR y VSPO2 son 1)
  // Validar y enviar datos a la base de datos (Si cambio el valor)
  if (validateReading(heartRate, previousHeartRate, validHeartRate, 60, 150)) {
    sendToFirebase(PATH_HR, heartRate, "Ritmo Cardiaco");
    previousHeartRate = heartRate;
  } else {
    Serial.println("HR invalido, no enviado.");
  }

  if (validateReading(spo2, previousSPO2, validSPO2, 85, 100)) {
    sendToFirebase(PATH_SPO2, spo2, "Oxigenación");
    previousSPO2 = spo2;
  } else {
    Serial.println("SPO2 invalido, no enviado.");
  }
}