#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Servo.h>

WiFiClient espClient;
PubSubClient client(espClient);

// Define the variables for the duration and the distance:
long redDuration;
long redDistance;
long blueDuration;
long blueDistance;

bool blueServoClosed = false;
bool redServoClosed = false;

int redScoreThreshold = 20; // Distance threshold for scoring, in centimeters
int blueScoreThreshold = 20;

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
int servoPinRed = D6;
int servoPinBlue = D7;
int open = 90;
int close = 180;

// Tilt Variables - Remove these as they are not needed anymore

// Define the Trig and Echo pin:
#define RED_TRIG_PIN D0
#define RED_ECHO_PIN D1
#define BLUE_TRIG_PIN D2
#define BLUE_ECHO_PIN D3

unsigned long previousMillis = 0;
const long interval = 150; // Interval between distance measurements in milliseconds

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

    pinMode(RED_TRIG_PIN, OUTPUT);
    pinMode(RED_ECHO_PIN, INPUT);
    pinMode(BLUE_TRIG_PIN, OUTPUT);
    pinMode(BLUE_ECHO_PIN, INPUT);

    // Set up MQTT subscription to the desired topic
    client.setCallback(callback);
}

void loop() {
    // Calls reconnect loop if disconnected from WiFi or MQTT server
    if (!client.connected()) {
        reconnect();
    }

    if (allow == 1){
        redServo.write(open);
        blueServo.write(open);
    }

    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;

        

        // Measure distances from ultrasonic sensors
        redDistance = measureDistance(RED_TRIG_PIN, RED_ECHO_PIN);
        blueDistance = measureDistance(BLUE_TRIG_PIN, BLUE_ECHO_PIN);

        // Serial.print("Distance Red: ");
        // Serial.println(redDistance);
        // Serial.print("Distance Blue: ");
        // Serial.println(blueDistance);

        // Check if a goal is scored
        if (redDistance < redScoreThreshold) {
            redscore++;
            Serial.print("Red Score: ");
            Serial.println(redscore);
            delay(2000);
        }

        if (blueDistance < blueScoreThreshold) {
            bluescore++;
            Serial.print("Blue Score: ");
            Serial.println(bluescore);
            delay(2000);
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

        // Close servos if one player gets 9 points
        if (bluescore == 9 && !blueServoClosed) {
            blueServo.write(close);
            Serial.println("Blue servo closed");
            blueServoClosed = true;
            delay(1000);
        }

        if (redscore == 9 && !redServoClosed) {
            redServo.write(close);
            Serial.println("Red servo closed");
            redServoClosed = true;
            delay(1000);
        }
    }

    if (bluescore >= 10 || redscore >= 10) {
            redServo.write(close);
            blueServo.write(close);
            redscore = 0;
            bluescore = 0;
            Serial.println("Game Over");
            allow = 0;
            bool redServoClosed = true;
            bool blueServoClosed = true;
            redServo.write(close);
            blueServo.write(close);
            delay(1000);
        }
    
    client.setCallback(callback);
}

// Function to measure distance using ultrasonic sensor
int measureDistance(int trigPin, int echoPin) {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    return pulseIn(echoPin, HIGH) / 58; // Convert pulse duration to distance (cm)
}

// This void reconnects to MQTT server when it loses connection
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