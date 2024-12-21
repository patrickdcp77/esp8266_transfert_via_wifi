#include <Arduino.h>
#include <ESP8266WiFi.h> // Inclure la bibliothèque ESP8266WiFi
#include <ESP8266WebServer.h> // Inclure la bibliothèque ESP8266WebServer
#include <ArduinoOTA.h>  // Inclure la bibliothèque ArduinoOTA

//const int LED_PIN = LED_BUILTIN; // LED intégrée (GPIO2)

// Définir les informations de connexion Wi-Fi
const char* ssid = "HONOR20PAT";
const char* password = "12345678";
int compte;
// Créer une instance du serveur web sur le port 80
ESP8266WebServer server(80);

// Variable pour stocker les messages Serial
String serialOutput = "";

void setup() {
  //pinMode(LED_PIN, OUTPUT); // Configure la broche comme sortie
  Serial.begin(9600);     // Initialise la communication série

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
    String html = "<html><body><h1>ESP8266 Serial Output</h1><pre>" + serialOutput + "</pre></body></html>";
    server.send(200, "text/html", html);
  });

  // Démarrer le serveur web
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  //digitalWrite(LED_PIN, HIGH); // Allume la LED
  compte = compte +1;
  ArduinoOTA.handle(); // Gérer les mises à jour OTA
  server.handleClient(); // Gérer les requêtes HTTP

  // Ajouter un message à serialOutput
  serialOutput += "ok code mis à jour \n";
  serialOutput += compte;
  serialOutput += "  \n";
  Serial.println("ok ");
  delay(1000);                 // Attends 1 seconde
  //digitalWrite(LED_PIN, LOW);  // Éteint la LED
  //delay(1000);                 // Attends 1 seconde
}