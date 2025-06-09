#include "DHT.h" // Include DHT sensor library
#define DHTPIN 18 // Define pin number to which DHT sensor is connected
#define DHTTYPE DHT11 // Define type of DHT sensor in use, here DHT11

//Wifi connection using MQTT prtotcol pre-req
#include <WiFi.h>
#include <PubSubClient.h>

// Replace with your network credentials
const char* ssid = "TASIGEN's A15";
const char* wifiPassword = "106801ts";

// Replace with your ThingsBoard device token and server address
const char* mqttServer = "demo.thingsboard.io";
const char* token = "9E4UYtX2F0tre1JyOlxh";

// Create DHT sensor object with defined pin and type
DHT dht(DHTPIN, DHTTYPE);

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// Function to connect to WiFi
void setupWiFi() {
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, wifiPassword);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

// Function to connect (or reconnect) to MQTT
void reconnectMQTT() {
  // Loop until reconnected to MQTT
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Connect with device token as username; password is not used
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);

    if (mqttClient.connect(clientId.c_str(), token, "")) {
      Serial.println("connected");
    } 
    
    else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" - trying again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(9600);

  setupWiFi();
  mqttClient.setServer(mqttServer, 1883);

  dht.begin(); // Initialize DHT sensor
  Serial.begin(9600);

  delay(2000); // Wait for 2 seconds
}

void loop() {

  if (!mqttClient.connected()) {
    reconnectMQTT();
  }
  mqttClient.loop();

  delay(5000); // Wait for 5 seconds

  // Read humidity value from the DHT sensor and store it in the variable h
  float h = dht.readHumidity();
  // Read temperature value from the DHT sensor and store it in the variable t
  float t = dht.readTemperature();

  // Check if any reading operation failed
  /*if (isnan(h) || isnan(t)) {
  // If a reading operation failed, exit the current loop to try again
    printf("Error in reading sensor!");
    return;
  }*/
  String message = "{\"Sleepiness\": 25, \"Happiness\": 23, \"Hunger\":10 , \"Age\":40, \"BeardLength\":10, \"Expression\":\"Neutral\", \"Clothing\":\"Shirt\"}";
  // Do something with the humidity and temperature values here
  printf("%.2f%, %.2f Celsius\n", h, t);


  // Build a JSON payload with the random numbers
  char pld[300];
  snprintf(pld, sizeof(pld), "%s", message.c_str());

  if (mqttClient.publish("v1/devices/me/telemetry", pld)) {
    Serial.println("Telemetry data published");
  } else {
    Serial.println("Publish failed");
  }

  delay(5000); // Wait for 5 second before the next loop iteration
}


