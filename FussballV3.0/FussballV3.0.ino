#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Servo.h>

WiFiClient espClient;
PubSubClient client(espClient);

// Wifi Settings
const char* ssid = "stemlab24";
const char* pass = "cbaqcheen123opq";
const char* mqtt_serv = "192.168.100.71";
const char* client_id = "fussball";

// The Score Variable
int redscore = 0;
int bluescore = 0;
int allow = 1;

// Servo Pins
Servo redServo;
Servo blueServo;
int servoPinRed = D3;
int servoPinBlue = D1;
int open = 90;
int close = 180;


// Tilt Variables
int tiltSwitchRedPin = D6;
int tiltSwitchBluePin = D7;
// bool tiltSwitchRedState = LOW;
// bool tiltSwitchBlueState = LOW; Was probs cause issues


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

    redServo.attach(servoPinRed);
    blueServo.attach(servoPinBlue);
  
    // Set up MQTT subscription to the desired topic
    client.setCallback(callback);

    

}

void loop() {
    // Calls reconect loop if disconected from wifi or mqqt server
    if (!client.connected()) {
        reconnect();
    }

    

    // Closes both servos when either player gets 10 and ends the game. 
    if (bluescore >= 10 || redscore >= 10) {
        redServo.write(close);
        blueServo.write(close);
        redscore = 0;
        bluescore = 0;
        Serial.print("Game Over");
        allow = 0;
    }

    if (allow == 1){
      redServo.write(open);
      blueServo.write(open);
}

    int tiltRed = digitalRead(tiltSwitchRedPin);
    int tiltBlue = digitalRead(tiltSwitchBluePin);

    Serial.print("Tilt Switch Red: ");
    Serial.println(tiltRed);
    Serial.print("Tilt Switch Blue: ");
    Serial.println(tiltBlue);

    delay(3000);

    if (tiltRed == LOW) {
        redscore += 1;
        Serial.print("Red Score: ");
        Serial.println(redscore);
        Serial.print("Blue Score: ");
        Serial.println(bluescore);
        delay(200);
    }
    if (tiltBlue == LOW) {
        bluescore += 1;
        Serial.print("Red Score: ");
        Serial.println(redscore);
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
    if (bluescore >= 9) {
        blueServo.write(close);
        Serial.print("Red servo closed");
        
    }

    // Closes blue side servo if they get 9 points
    if (redscore >= 9) {
        redServo.write(close);
        Serial.print("Blue servo closed");
        
    }
    
    client.setCallback(callback);

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


void callback(char* topic, byte* payload, unsigned int length) {
  String response;

  for (int i = 0; i < length; i++) {
    response += (char)payload[i];
  }
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  Serial.println(response);
  
}



