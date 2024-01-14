#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Servo.h>

WiFiClient espClient;
PubSubClient client(espClient);

// Wifi Settings
const char* ssid = "OpenWrt";
const char* pass = "greentara";
const char* mqtt_serv = "172.16.20.10";
const char* client_id = "fussball";

// Sets the topic as a variable
const char* topic = "scores";
const char* rectopic = "allow";


int redscore = 0;
int bluescore = 0;
int allow = 0;

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
bool tiltSwitchRedState = LOW;
bool tiltSwitchBlueState = LOW;

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
    client.setCallback(callback); // Set the callback function for message handling
}

void loop() {
    // Calls reconnect loop if disconnected from Wi-Fi or MQTT server
    if (!client.connected()) {
        reconnect();
    }

    client.loop();
    // Function that PubSubClient will handle a lot of MQTT connection processes without interrupting the code

        // Create a JSON object
    StaticJsonDocument<200> jsonDoc;
    
    // Add data to the JSON object
    jsonDoc["bluescore"] = bluescore; // "sensor" is the key, and "temperature" is the value
    jsonDoc["redscore"] = redscore; // "value" is the key, and 25.5 is the value (temperature reading)
    
    // Serialize the JSON object to a string
    char jsonBuffer[200];
    serializeJson(jsonDoc, jsonBuffer);
    
    // Publish the JSON string to the MQTT topic
    client.publish(topic, jsonBuffer);

    // Add other code as needed
    delay(3000);



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
      Serial.println("game started");
      allow = 0;
      Serial.println(allow);
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
}

// This void reconnects to MQTT server when it loses connection
void reconnect() {
    while (!client.connected()) {
        Serial.println("Attempting MQTT connection...");
        if (client.connect(client_id)) {
            Serial.println("Connected to MQTT server");
            // Subscribe to MQTT topics here if needed for receiving 
            client.subscribe(rectopic);

        } else {
            Serial.print("Connection to MQTT server failed, rc=");
            Serial.print(client.state());
            Serial.println(" Retrying in 5 seconds...");
            delay(5000);
        }
    }
}

// This function states the content of the subscribed topic
void callback(char* rectopic, byte* payload, unsigned int length) {
    Serial.print("Message received on topic: ");
    Serial.println(rectopic);

    Serial.print("Message payload: ");
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();

    // Parse the received JSON payload
    StaticJsonDocument<200> jsonDoc; // Create a JSON document to hold the parsed data
    DeserializationError error = deserializeJson(jsonDoc, payload, length);

    // Check for parsing errors
    if (error) {
        Serial.print("Error parsing JSON: ");
        Serial.println(error.c_str());
        return;
    }

    // Now you can access the parsed data
    float value = jsonDoc["allow"]; // Get the "value" field

    // Print the parsed data
    Serial.print("allow: ");
    if (value == 1){
      allow = 1;
      redscore = 0;
      bluescore = 0;
      Serial.println(allow);


    }

    // Add your code to handle the parsed data here
}