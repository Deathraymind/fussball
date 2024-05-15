#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Servo.h>

WiFiClient espClient;
PubSubClient client(espClient);

// WiFi Settings
const char* ssid = "stemlab24";
const char* pass = "cbaqcheen123opq";
const char* mqtt_serv = "192.168.100.71";
const char* client_id = "fussball";
const char* topic = "allow";

// The Score Variable
int redscore = 0;
int bluescore = 0;
int allow = 1;

// Servo Pins
Servo redServo;
Servo blueServo;
int servoPinRed = D0; 
int servoPinBlue = D1;
int open = 110;
int close = 0;

// Tilt Variables
int switchRed = D6; // N03 is the input, COM is ground.
int switchBlue = D7;

void setup() {
    Serial.begin(9600);
    Serial.println("");
    Serial.print("Connecting to: ");
    Serial.println(ssid);

    // Pullup input for switches
    pinMode(switchRed, INPUT_PULLUP);
    pinMode(switchBlue, INPUT_PULLUP);

    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.print("WiFi Connected, IP: ");
    Serial.println(WiFi.localIP());

    client.setServer(mqtt_serv, 1883);
    client.setCallback(callback);

    redServo.attach(servoPinRed);
    blueServo.attach(servoPinBlue);

    redServo.write(open);
    blueServo.write(open);
}

void loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();

    int redState = digitalRead(switchRed);
    int blueState = digitalRead(switchBlue);

    // Closes both servos when either player gets 10 and ends the game. 
    if (bluescore >= 10 || redscore >= 10) {
        redServo.write(close);
        blueServo.write(close);
        redscore = 0;
        bluescore = 0;
        Serial.println("Game Over");
        delay(3000);
    }

    // Closes red side servo if blue gets 9 points
    if (bluescore >= 9) {
        redServo.write(close);
        Serial.println("Red servo closed");
    }

    // Closes blue side servo if red gets 9 points
    if (redscore >= 9) {
        blueServo.write(close);
        Serial.println("Blue servo closed");
    }

    if (allow == 1) {
        redServo.write(open);
        blueServo.write(open);
        allow = 0;
        delay(1000);
    }

    if (redState == LOW) {
        redscore += 1;
        Serial.print("Red Score: ");
        Serial.println(redscore);
        Serial.print("Blue Score: ");
        Serial.println(bluescore);
        delay(200);

        // Create a JSON document
        StaticJsonDocument<128> jsonDoc;
        jsonDoc["redscore"] = redscore;
        jsonDoc["bluescore"] = bluescore;

        // Serialize JSON to a string
        char jsonStr[128];
        serializeJson(jsonDoc, jsonStr);

        // Publish the JSON message to MQTT
        client.publish("scores", jsonStr);
    }

    if (blueState == LOW) {
        bluescore += 1;
        Serial.print("Red Score: ");
        Serial.println(redscore);
        Serial.print("Blue Score: ");
        Serial.println(bluescore);
        delay(200);

        // Create a JSON document
        StaticJsonDocument<128> jsonDoc;
        jsonDoc["redscore"] = redscore;
        jsonDoc["bluescore"] = bluescore;

        // Serialize JSON to a string
        char jsonStr[128];
        serializeJson(jsonDoc, jsonStr);

        // Publish the JSON message to MQTT
        client.publish("scores", jsonStr);
    }
}

void reconnect() {
    while (!client.connected()) {
        Serial.println("Attempting MQTT connection...");
        if (client.connect(client_id)) {
            Serial.println("Connected to MQTT server");
            client.subscribe(topic);
        } else {
            Serial.print("Connection to MQTT server failed, rc=");
            Serial.print(client.state());
            Serial.println(" Retrying in 5 seconds...");
            delay(5000);
        }
    }
}

void callback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    String message;
    for (int i = 0; i < length; i++) {
        message += (char)payload[i];
    }
    Serial.println(message);

    StaticJsonDocument<128> doc;
    DeserializationError error = deserializeJson(doc, message);
    if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        return;
    }

    if (String(topic) == "allow") {
        allow = doc["allow"];
        Serial.print("Allow: ");
        Serial.println(allow);
    }
}