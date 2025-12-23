#ifndef MAX30102_h
#define MAX30102_h

// Librerias Para el Sensor MAX30102
#include <Wire.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"

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

// Pin del LED
byte readLED = 2;

void initializationMAX30102() {
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

void prechargeBuffer() {
  for (byte i = 0; i < bufferLength; i++) {
    while (particleSensor.available() == false)
    particleSensor.check();
    redBuffer[i] = particleSensor.getRed();
    irBuffer[i] = particleSensor.getIR();
    particleSensor.nextSample();
  }
}

// Actualizaremos cada 0.25 segundos (50 muestras a 200 SPS).
  const byte SAMPLES_TO_UPDATE = 50;
  const byte NEW_SAMPLES_START_INDEX = BUFFER_SIZE - SAMPLES_TO_UPDATE;  // 100 - 50 = 50

void updateSamples() {
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
}

#endif