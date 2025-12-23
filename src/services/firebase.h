#ifndef FIREBASE_H
#define FIREBASE_H

// Libreria para conexión a Firebase (Base de Datos)
#include<FirebaseESP32.h>
// Libreria con la conección al wifi
#include "services/wifiManager.h"

// Variables globales de para la configuración del Firebase (instancia de la clase Firebase)
FirebaseData myFireBaseData;
FirebaseConfig firebaseConfig;
FirebaseAuth firebaseAuth;

void connectionFirebase(const char* url, const char* secreto) {
  // Configuración de la conexión a Firebase
  firebaseConfig.database_url = url;
  firebaseConfig.signer.tokens.legacy_token = secreto;
  Firebase.begin(&firebaseConfig, &firebaseAuth);
  Firebase.reconnectWiFi(true);
}

void sendToFirebase(const char* path, int value, const char* label) {
  if(!isConnected()) return;

  if (Firebase.setInt(myFireBaseData, path, value)) {
    Serial.println(label);          
    Serial.print(": ");
    Serial.println(value);
    } else {
    Serial.print("Error de conexión: ");
    Serial.println(myFireBaseData.errorReason());
    }
}

#endif