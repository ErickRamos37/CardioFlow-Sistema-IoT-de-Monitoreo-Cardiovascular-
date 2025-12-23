#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H

// Libreria para conexión a WiFi
#include<WiFi.h>

void connectionWiFi(const char* ssid, const char* password) {
  WiFi.begin(ssid, password); // Conexión a WiFi

  Serial.print("Conectando a la red: ");
  Serial.println(ssid);

  // Estableciendo conexión al WiFi
  while(WiFi.status() != WL_CONNECTED) {
    Serial.print(". ");
    delay(500);
  }
  Serial.println("Conexión WiFi exitosa");
}

bool isConnected() {
  return (WiFi.status() == WL_CONNECTED);
}

#endif