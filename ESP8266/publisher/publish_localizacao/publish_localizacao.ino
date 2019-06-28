/*
 * http://www.instructables.com/id/How-to-Use-MQTT-With-the-Raspberry-Pi-and-ESP8266/#CNLP6YQJFUAGAUB
 */
#include <ESP8266WiFi.h> // Habilita que o ESP8266 se conecte ao WiFi
#include <PubSubClient.h> // Biblioteca que habilita a publicação da mensagem no Broker
#include "EmonLib.h"
#include <SoftwareSerial.h>
#include <TinyGPS.h>

// Módulo GPS
int pinRX = D2;  //Pino conectado no TX do módulo GPS
int pinTX = D1;  //Pino conectado no RX do módulo GPS
SoftwareSerial serial1(pinRX,pinTX); // RX, TX
TinyGPS gps1;

// WiFi
const char* ssid = "Clericuzi";//O SSID da sua rede
const char* wifi_password = "pedromat"; //sua senha

// MQTT
const char* mqtt_server = "192.168.0.110"; //Endereço de IP do Broker
const char* mqtt_topic = "latlong";
const char* mqtt_username = "pi";
const char* mqtt_password = "raspberry";
const char* clientID = "04061995";

long randNumber;

// Inicializa o WiFi e o publisher
WiFiClient wifiClient;
PubSubClient client(mqtt_server, 3000, wifiClient); // 3001 é a porta que será liberada para a comunicação

void setup() {
  serial1.begin(9600);
  Serial.begin(115200);
  Serial.print("Connecting to ");
  Serial.println(ssid);

  // Conecta no WiFi
  WiFi.begin(ssid, wifi_password);

  // Wait until the connection has been confirmed before continuing
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Connect to MQTT Broker
  // client.connect retorna um boleano indicando o resultado da conexão.
  // Se a conexão falhou, certifique-se que o SSID e o Password estao corretos
  if (client.connect(clientID, mqtt_username, mqtt_password)) {
    Serial.println("Connected to MQTT Broker!");
  }
  else {
    Serial.println("Connection to MQTT Broker failed...");
  }
}

void loop() {
    bool recebido = false;
    while (serial1.available()) {
       //Serial.println("O GPS está disponível");
       char cIn = serial1.read();
       recebido = gps1.encode(cIn);
    }
    
    if (recebido) {
       Serial.println("----------------------------------------");
       
       //Latitude e Longitude
       long latitude, longitude;
       unsigned long idadeInfo;
       gps1.get_position(&latitude, &longitude, &idadeInfo);     
  
       if (latitude != TinyGPS::GPS_INVALID_F_ANGLE) {
          Serial.print("Latitude: ");
          Serial.println(float(latitude) / 1000000, 6);
       }
  
       if (longitude != TinyGPS::GPS_INVALID_F_ANGLE) {
          Serial.print("Longitude: ");
          Serial.println(float(longitude) / 1000000, 6);
       }
  
       //altitude
       float altitudeGPS;
       altitudeGPS = gps1.f_altitude();
  
       if ((altitudeGPS != TinyGPS::GPS_INVALID_ALTITUDE) && (altitudeGPS != 1000000)) {
          Serial.print("Altitude (cm): ");
          Serial.println(altitudeGPS);
       }

       String SerialData="";
       SerialData += String(float(latitude) / 1000000, 6);
       SerialData += F("/");
       SerialData += String(float(longitude) / 1000000, 6);
       const char* localizacao = SerialData.c_str();
       mqtt_communicacao(localizacao);
    } else {
        /*String SerialData="-7.9957985/-34.8424998";
         //SerialData = String(potencia);
        const char* localizacao = SerialData.c_str();
        mqtt_communicacao(localizacao);*/
    }
    //delay(1000);
}

void mqtt_communicacao(const char *localizacao){
    // PUBLISH to the MQTT Broker (topic = mqtt_topic, defined at the beginning)
    // Here, "Button pressed!" is the Payload, but this could be changed to a sensor reading, for example.
    if (client.publish(mqtt_topic, localizacao)) {
      Serial.println("Valor da potencia enviado pro broken");
    }
    // Again, client.publish will return a boolean value depending on whether it succeded or not.
    // If the message failed to send, we will try again, as the connection may have broken.
    else {
      Serial.println("Message failed to send. Reconnecting to MQTT Broker and trying again");
      client.connect(clientID, mqtt_username, mqtt_password);
      delay(10); // This delay ensures that client.publish doesn't clash with the client.connect call
      client.publish(mqtt_topic, localizacao);
    }
}
