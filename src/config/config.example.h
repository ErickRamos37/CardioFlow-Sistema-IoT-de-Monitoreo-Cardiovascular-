/*
 ARCHIVO DE CONFIGURACIÓN (PLANTILLA)
    INSTRUCCIONES:
 1. Crea una copia de este archivo y renómbrala como 'config.h'.
 2. Asegúrate de que 'config.h' esté incluido en tu archivo .gitignore.
 3. Completa los datos a continuación con tus credenciales reales.
 */

#ifndef CONFIG_H
#define CONFIG_H

// Configuración de Red WiFi
// El nombre (SSID) de tu red inalámbrica
#define WIFI_SSID  "Tu_Red_WiFi"
// La contraseña de tu red WiFi
#define WIFI_PASS  "Tu_Password_Seguro" 

// Configuración de Google Firebase
// URL de tu Realtime Database (ej: https://nombre-proyecto.firebaseio.com/)
#define FIREBASE_URL "TU_URL_DE_FIREBASE"
// Token secreto de la base de datos (Database Secret)
#define FIREBASE_SECRET "Tu_Secreto"

// Rutas de la Base de Datos (Paths)
// Ubicación donde se guardarán los datos en el árbol de Firebase
#define PATH_HR "/Monitoreo/HeartRate"
#define PATH_SPO2 "/Monitoreo/SPO2"

#endif