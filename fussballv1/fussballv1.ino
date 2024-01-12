#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Servo.h>

WiFiClient espClient;
PubSubClient client(espClient);

// Wifi Settings
const char* ssid = "OpenWrt";
const char* pass = "12341234";
const char* mqtt_serv = "172.16.1.57";
const char* client_id = "fussball";

// The Score Variable
int redscore = 0;
int bluescore = 0;

// Servo Pins
Servo myservo;
int servoPinRed = 3;
int servoPinBlue = 2;

// Tilt Variables
int tiltSwitchRedPin = 7;
int tiltSwitchBluePin = 6;
bool tiltSwitchRedState = LOW;
bool tiltSwitchBlueState = LOW;


void setup() {
    Serial.begin(9600);
    while (!Serial);
    Serial.println("");
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

    client.setServer(mqtt_serv, 1883);
}

void loop() {
    // Calls reconect loop if disconected from wifi or mqqt server
    if (!client.connected()) {
        reconnect();
    }

    int tiltRed = digitalRead(tiltSwitchRedPin);
    int tiltBlue = digitalRead(tiltSwitchBluePin);

    Serial.print("Tilt Switch Red: ");
    Serial.println(tiltRed);
    Serial.print("Tilt Switch Blue: ");
    Serial.println(tiltBlue);

    delay(5000);

    if (tiltRed == HIGH) {
        redscore += 1;
        Serial.print("Red Score: ");
        Serial.println(redscore);
        delay(200);
    }
    if (tiltBlue == HIGH) {
        bluescore += 1;
        Serial.print("Blue Score: ");
        Serial.println(bluescore);
        delay(200);
    }

    // Create a JSON document
    StaticJsonDocument<80> jsonDoc;
    jsonDoc["redscore"] = redscore;
    jsonDoc["bluescore"] = bluescore;

    // Serialize JSON to a string
    char jsonStr[80];
    serializeJson(jsonDoc, jsonStr);

    // Publish the JSON message to MQTT
    client.publish("scores", jsonStr);
    
    // Closes blue side servo if they get 9 points
    if (bluescore > 9) {
        myservo.attach(servoPinBlue);
        myservo.write(90);
        Serial.print("blue servo closed");
        
    }

    // Closes blue side servo if they get 9 points
    if (redscore > 9) {
        myservo.attach(servoPinRed);
        myservo.write(90);
        Serial.print("red servo closed");
        
    }
    
    // Closes both servos when either player gets 10 and ends the game. 
    if (bluescore == 10 || redscore == 10) {
        myservo.attach(servoPinRed);
        myservo.write(90);
        myservo.attach(servoPinBlue);
        myservo.write(90);
        redscore = 0;
        bluescore = 0;
        Serial.print("Game Over");
    }

}
  

// This void reconnects to mqqt server when it looses connection
// Note this is not the item that reconnects to wifi the ESP8266 library itegrates that feature 
void reconnect() {
    while (!client.connected()) {
        Serial.println("Attempting MQTT connection...");
        if (client.connect(client_id)) {
            Serial.println("Connected to MQTT server");
            // Subscribe to MQTT topics here if needed
        } else {
            Serial.print("Connection to MQTT server failed, rc=");
            Serial.print(client.state());
            Serial.println(" Retrying in 5 seconds...");
            delay(5000);
        }
    }
}

//Made by Bowyn Rosch 2023-2024 With Blood Sweat and alot of hair pulling! Thanks Barns!
