// Incluye las bibliotecas necesarias
#include <WiFi.h>
#include <FirebaseESP32.h>
#include <TinyGPS++.h>

// Define las constantes para la conexión WiFi
#define WIFI_SSID "HERETICO"
#define WIFI_PASSWORD "244466666"

// Define las constantes para la conexión a Firebase
#define FIREBASE_HOST "xxx"
#define FIREBASE_AUTH "xxx"

// Define los pines para la conexión con el módulo GPS
#define RXD2 16
#define TXD2 17

// Inicializa el puerto serie para la comunicación con el módulo GPS
HardwareSerial neogps(1);
char datoCmd = 0;

#define NMEA 0
// Inicializa la biblioteca TinyGPS++
TinyGPSPlus gps;

// Inicializa la conexión con Firebase
FirebaseData firebaseData;
// Define el nodo de Firebase donde se almacenarán los datos del GPS
String nodo = "/gps";

// Esta función imprime los datos del GPS en el monitor serie y los envía a Firebase
void Visualizacion_Serial(void) {
  // Si la ubicación GPS es válida...
  if (gps.location.isValid() == 1) {
    // Imprime la latitud y la longitud en el monitor serie
    Serial.print("Lat: ");
    Serial.println(gps.location.lat(), 6);
    Serial.print("Lng: ");
    Serial.println(gps.location.lng(), 6);
    // Imprime el número de satélites en el monitor serie
    Serial.print("SAT: ");
    Serial.println(gps.satellites.value());

    // Imprime la hora en el monitor serie
    Serial.print("Hour: ");
    Serial.print(gps.time.hour());
    Serial.print(":");
    Serial.print(gps.time.minute());
    Serial.print(":");
    Serial.println(gps.time.second());
    Serial.println("---------------------------");

    // Envía la latitud, la longitud y la hora a Firebase
    Firebase.setDouble(firebaseData, nodo + "/lat", gps.location.lat());
    Firebase.setDouble(firebaseData, nodo + "/lng", gps.location.lng());
    Firebase.setInt(firebaseData, nodo + "/hour", gps.time.hour());
    Firebase.setInt(firebaseData, nodo + "/min", gps.time.minute());
    Firebase.setInt(firebaseData, nodo + "/sec", gps.time.second());
  }
  else {
    // Si la ubicación GPS no es válida, imprime un mensaje de error en el monitor serie
    Serial.println("Sin señal GPS");
  }
}

// Esta función se ejecuta una vez al inicio del programa
void setup() {
  // Inicializa el monitor serie y el puerto serie para el GPS
  Serial.begin(115200);
  neogps.begin(9600, SERIAL_8N1, RXD2, TXD2);

  Serial.println();

  // Conéctate a la red WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Conectado al WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }

  Serial.println();
  // Conéctate a Firebase
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
}

// Esta función se ejecuta en un bucle una vez que el programa ha comenzado
void loop() {
  // Si NMEA es verdadero...
  if (NMEA) {
    // Lee los datos del GPS y los imprime en el monitor serie
    while (neogps.available()) {
      datoCmd = (char)neogps.read();
      Serial.print(datoCmd);
    }
  } else {
    boolean newData = false;
    for (unsigned long start = millis(); millis() - start < 1000;) {
      while (neogps.available()) {
        if (gps.encode(neogps.read())) {
          newData = true;
        }
      }
    }
    // Si hay nuevos datos del GPS...
    if (newData == true) {
      newData = false;
      // Imprime el número de satélites en el monitor serie
      Serial.println(gps.satellites.value());
      // Llama a la función para visualizar los datos del GPS
      Visualizacion_Serial();
    }
    else { }
  }
}