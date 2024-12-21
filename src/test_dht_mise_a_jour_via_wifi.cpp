/*
Pour mettre à jour automatiquement la page web à chaque fois qu'il y a une nouvelle mesure, vous pouvez utiliser JavaScript pour rafraîchir la page à intervalles réguliers. Voici comment vous pouvez modifier votre code pour inclure cette fonctionnalité :

1. Ajouter un script JavaScript pour rafraîchir la page automatiquement.
2. Modifier la fonction de gestion des requêtes HTTP pour inclure le script JavaScript.

Voici le code modifié :


Explications des modifications :
Ajout d'un script JavaScript pour rafraîchir la page automatiquement :

Ce script rafraîchit la page toutes les secondes (content='1').
<head><meta http-equiv='refresh' content='1'></head>
Modification de la fonction de gestion des requêtes HTTP :
server.on("/", []() {
  String html = "<html><head><meta http-equiv='refresh' content='1'></head><body><h1>ESP8266 Serial Output</h1><pre>" + serialOutput + "</pre></body></html>";
  server.send(200, "text/html", html);
});

*/




#include <Arduino.h>
#include <ESP8266WiFi.h> // Inclure la bibliothèque ESP8266WiFi
#include <ESP8266WebServer.h> // Inclure la bibliothèque ESP8266WebServer
#include <ArduinoOTA.h>  // Inclure la bibliothèque ArduinoOTA
#include <DHT.h> // Inclure la bibliothèque DHT

//const int LED_PIN = LED_BUILTIN; // LED intégrée (GPIO2)

// Définir les informations de connexion Wi-Fi
const char* ssid = "HONOR20PAT";
const char* password = "12345678";

// Définir les broches et le type de capteur DHT
#define DHTPIN 4     // GPIO4 broche D2  Broche à laquelle le capteur DHT est connecté
#define DHTTYPE DHT22   // Définir le type de capteur DHT

// Créer une instance du capteur DHT
DHT dht(DHTPIN, DHTTYPE);

// Créer une instance du serveur web sur le port 80
ESP8266WebServer server(80);

// Variable pour stocker les messages Serial
String serialOutput = "";
int compte = 0;

void setup() {
  //pinMode(LED_PIN, OUTPUT); // Configure la broche comme sortie
  Serial.begin(9600);     // Initialise la communication série

  // Initialiser le capteur DHT
  dht.begin();

  // Connexion au réseau Wi-Fi
  Serial.println();
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);

  // Attendre la connexion
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Connected to WiFi");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Initialiser ArduinoOTA
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
    }
    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();

  // Configurer la route pour la page web
  server.on("/", []() {
    String html = "<html><head><meta http-equiv='refresh' content='1'></head><body><h1>ESP8266 Serial Output</h1><pre>" + serialOutput + "</pre></body></html>";
    server.send(200, "text/html", html);
  });

  // Démarrer le serveur web
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  //digitalWrite(LED_PIN, HIGH); // Allume la LED
  compte = compte + 1;
  ArduinoOTA.handle(); // Gérer les mises à jour OTA
  server.handleClient(); // Gérer les requêtes HTTP

  // Ajouter un message à serialOutput
  serialOutput += "ok code mis à jour \n";
  serialOutput += String(compte);

  // Lire la température du capteur DHT22
  float temperature = dht.readTemperature();
  if (isnan(temperature)) {
    serialOutput += "Erreur de lecture de la température\n";
  } else {
    serialOutput += " Température de la DHT22: " + String(temperature) + "°C\n";
  }

  Serial.println("ok ");
  delay(1000);                 // Attends 1 seconde
  //digitalWrite(LED_PIN, LOW);  // Éteint la LED
  //delay(1000);                 // Attends 1 seconde
}