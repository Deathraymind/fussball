#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "DHT.h"




#define DHTPIN D2 // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11  // DHT 11

DHT dht(DHTPIN, DHTTYPE);

// Data wire is connected to Arduino pin D5
#define ONE_WIRE_BUS D3

// Setup a OneWire instance
OneWire oneWire(ONE_WIRE_BUS);

// Pass OneWire reference to DallasTemperature library
DallasTemperature sensors(&oneWire);


WiFiClient espClient;
PubSubClient client(espClient);

// Wifi Settings
const char* ssid = "OpenWrt";
const char* pass = "greentara";
const char* mqtt_serv = "172.16.20.10";
const char* client_id = "tempeture";

// Sets the topic as a variable
const char* topic = "sensors";


void setup() {
    // Sets up serial at a baud rate of 9600
    Serial.begin(9600);
    while (!Serial);
    Serial.println("");

   

    // Shows that the code is initializing the Wi-Fi connection
    Serial.print("Connecting to: ");
    Serial.println(ssid);

    WiFi.begin(ssid, pass);
    int i = 0;
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(++i);
        Serial.print(" ");
    }
    Serial.println("");
    Serial.print("WiFi Connected, IP: ");
    Serial.println(WiFi.localIP());

    // Code to connect to MQTT server
    client.setServer(mqtt_serv, 1883);
}

void loop() {
    // Calls reconnect loop if disconnected from Wi-Fi or MQTT server
    if (!client.connected()) {
        reconnect();
    }

    client.loop();




  

    // Function that PubSubClient will handle a lot of MQTT connection processes without interrupting the code
  float humidity = dht.readHumidity();
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.print(" %\t");

  sensors.requestTemperatures(); 
  float temperatureC = sensors.getTempCByIndex(0);
  // Print temperature in Celsius
  Serial.print("Temperature is: ");
  Serial.print(temperatureC);
  Serial.println("°C");
  // Wait a bit before reading again
  delay(1000);

        // Create a JSON object
    StaticJsonDocument<200> jsonDoc;
    
    // Add data to the JSON object
    jsonDoc["humidity"] = humidity; // "sensor" is the key, and "temperature" is the value
    jsonDoc["temperature"] = temperatureC; // "value" is the key, and 25.5 is the value (temperature reading)
    // Serialize the JSON object to a string
    char jsonBuffer[200];
    serializeJson(jsonDoc, jsonBuffer);
    
    // Publish the JSON string to the MQTT topic
    client.publish(topic, jsonBuffer);

    // Add other code as needed
    delay(3000);
}

// This void reconnects to MQTT server when it loses connection
void reconnect() {
    while (!client.connected()) {
        Serial.println("Attempting MQTT connection...");
        if (client.connect(client_id)) {
            Serial.println("Connected to MQTT server");
            // Subscribe to MQTT topics here if needed for receiving 
          

        } else {
            Serial.print("Connection to MQTT server failed, rc=");
            Serial.print(client.state());
            Serial.println(" Retrying in 5 seconds...");
            delay(5000);
        }
    }
}

