#include <Arduino.h>

// Librerias Para el Sensor MAX30102
#include <Wire.h>
// #include "MAX30105.h"
#include "spo2_algorithm.h"

// Libreria con las constantes para conexión a WiFi y a Firebase
#include "config/config.h"
// Libreria con la configuración y funciones para Firebase
#include "services/firebase.h"
// Libreria con la conección al wifi
#include "services/wifiManager.h"
// Libreria con la configuración y funciones para el sensor MAX30102
#include "sensors/max30102.h"

int32_t anterior_heartRate = 0;
int8_t anterior_spo2 = 0;

// Pin del LED
// byte readLED = 2;

void setup() {
  Serial.begin(115200);
  pinMode(readLED, OUTPUT);
  
  // Conección al WiFi
  connectionWiFi(WIFI_SSID, WIFI_PASS);
  
  // Conección a la base de datos (Firebase)
  connectionFirebase(FIREBASE_URL, FIREBASE_SECRET);
  
  // Inicualizacion y configuración del sensor MAX30102
  initializationMAX30102();
}

void loop() {
  // Bloque 1: Llenar el buffer inicial con 100 muestras
  Serial.println(F("Llenando buffer inicial..."));
  prechargeBuffer();
  Serial.println("Buffer Lleno. Iniciando cálculo continuo.");

  // Cálculo inicial.
  maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);

  // Bloque 2: Lectura continua y cálculo
  while (1) {
    // Desplaza y obtiene 50 nuevas muestras
    updateSamples();
    // Recalcular HR y SP02
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
        sendToFirebase(PATH_HR, heartRate, "Ritmo Cardiaco");
        anterior_heartRate = heartRate;
      }
      if(spo2 != anterior_spo2 && heartRate > 59 && heartRate < 150) {
      // if(spo2 != anterior_spo2) {
        sendToFirebase(PATH_SPO2, spo2, "Oxigenación");
        anterior_spo2 = spo2;
      }
    } else {
      // Señal inválida
      Serial.println(F("HR_NOK=-999,SPO2_NOK=-999"));
    }
  }
}