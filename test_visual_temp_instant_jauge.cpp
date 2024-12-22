/*
 * Ce programme utilise un ESP8266 pour lire la température d'un capteur DHT22 et afficher cette température sur une page web.
 * La page web contient une jauge à aiguille qui est mise à jour automatiquement toutes les 5 secondes pour refléter les nouvelles mesures de température.
 * Un graphique des 10 dernières minutes est également affiché sous la jauge.
 * 
 * Démarche :
 * 1. Connexion au réseau Wi-Fi : Le programme se connecte au réseau Wi-Fi spécifié par les constantes `ssid` et `password`.
 * 2. Initialisation du capteur DHT22 : Le capteur DHT22 est initialisé pour lire les mesures de température.
 * 3. Configuration du serveur web : Un serveur web est configuré pour servir une page HTML contenant la jauge à aiguille et le graphique.
 * 4. Utilisation de JustGage et Chart.js : Les bibliothèques JavaScript JustGage et Chart.js sont utilisées pour créer et afficher la jauge à aiguille et le graphique.
 * 5. Mise à jour automatique de la jauge et du graphique : Un script JavaScript est utilisé pour mettre à jour la jauge et le graphique toutes les 5 secondes en récupérant les nouvelles mesures de température via une requête AJAX.
 * 6. Gestion des requêtes HTTP : Le serveur web gère les requêtes HTTP pour servir la page HTML et fournir les données de température en format JSON.
 * 7. Mise à jour OTA : Le programme supporte les mises à jour Over-The-Air (OTA) pour permettre la mise à jour du firmware via Wi-Fi.
 * 
 * Bibliothèques utilisées :
 * - ESP8266WiFi : Pour la connexion au réseau Wi-Fi.
 * - ESP8266WebServer : Pour la gestion du serveur web.
 * - ArduinoOTA : Pour les mises à jour OTA.
 * - DHT : Pour lire les mesures du capteur DHT22.
 * - Adafruit_Sensor : Bibliothèque de capteurs unifiés d'Adafruit.
 * - JustGage : Bibliothèque JavaScript pour créer des jauges à aiguille.
 * - Raphael.js : Bibliothèque JavaScript pour le rendu vectoriel (nécessaire pour JustGage).
 * - Chart.js : Bibliothèque JavaScript pour créer des graphiques.
 * 
 * Auteur : [Votre Nom]
 * Date : [Date]
 */

#include <Arduino.h>
#include <ESP8266WiFi.h> // Inclure la bibliothèque ESP8266WiFi
#include <ESP8266WebServer.h> // Inclure la bibliothèque ESP8266WebServer
#include <ArduinoOTA.h>  // Inclure la bibliothèque ArduinoOTA
#include <DHT.h> // Inclure la bibliothèque DHT
#include <Adafruit_Sensor.h> // Inclure la bibliothèque Adafruit Unified Sensor

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

// Variables pour stocker les messages Serial et les mesures de température
String serialOutput = "";
int compte = 0;
float temperature = 0.0;
float temperatureHistory[120] = {0}; // Historique des 10 dernières minutes (120 mesures à 5 secondes d'intervalle)

void setup() {
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
    String html = "<html><head><script src='https://cdnjs.cloudflare.com/ajax/libs/raphael/2.2.7/raphael.min.js'></script>";
    html += "<script src='https://cdnjs.cloudflare.com/ajax/libs/justgage/1.2.9/justgage.min.js'></script>";
    html += "<script src='https://cdn.jsdelivr.net/npm/chart.js'></script></head><body><h1>ESP8266 Temperature Gauge</h1>";
    html += "<div id='gauge' style='width:400px; height:320px;'></div>";
    html += "<canvas id='historyChart' width='400' height='200'></canvas>";
    html += "<script>";
    html += "var gauge = new JustGage({";
    html += "id: 'gauge',";
    html += "value: 0,";
    html += "min: 0,";
    html += "max: 50,";
    html += "title: 'Température (°C)'";
    html += "});";
    html += "var ctx = document.getElementById('historyChart').getContext('2d');";
    html += "var historyChart = new Chart(ctx, {";
    html += "type: 'line',";
    html += "data: {";
    html += "labels: Array.from({length: 120}, (_, i) => i + 1),";
    html += "datasets: [{";
    html += "label: 'Température (°C)',";
    html += "data: [],";
    html += "borderColor: 'rgba(75, 192, 192, 1)',";
    html += "borderWidth: 1";
    html += "}]";
    html += "},";
    html += "options: {";
    html += "scales: {";
    html += "y: {";
    html += "beginAtZero: true";
    html += "}";
    html += "}";
    html += "}";
    html += "});";
    html += "function updateGauge() {";
    html += "fetch('/temperature').then(response => response.json()).then(data => {";
    html += "gauge.refresh(data.temperature);";
    html += "historyChart.data.datasets[0].data.push(data.temperature);";
    html += "if (historyChart.data.datasets[0].data.length > 120) {";
    html += "historyChart.data.datasets[0].data.shift();";
    html += "}";
    html += "historyChart.update();";
    html += "});";
    html += "}";
    html += "setInterval(updateGauge, 5000);";
    html += "</script>";
    html += "</body></html>";
    server.send(200, "text/html", html);
  });

  // Configurer la route pour obtenir la température en JSON
  server.on("/temperature", []() {
    String json = "{\"temperature\":" + String(temperature) + "}";
    server.send(200, "application/json", json);
  });

  // Démarrer le serveur web
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  compte = compte + 1;
  ArduinoOTA.handle(); // Gérer les mises à jour OTA
  server.handleClient(); // Gérer les requêtes HTTP

  // Lire la température du capteur DHT22
  temperature = dht.readTemperature();
  if (isnan(temperature)) {
    serialOutput += "Erreur de lecture de la température\n";
  } else {
    serialOutput += " Température de la DHT22: " + String(temperature) + "°C\n";
  }

  // Mettre à jour l'historique des températures toutes les 5 secondes
  for (int i = 119; i > 0; i--) {
    temperatureHistory[i] = temperatureHistory[i - 1];
  }
  temperatureHistory[0] = temperature;

  Serial.println("ok ");
  delay(5000);                 // Attends 5 secondes
}