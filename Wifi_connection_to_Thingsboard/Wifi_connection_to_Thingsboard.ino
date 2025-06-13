#define SWITCH_SCREEN 2

#include "DHT.h" // Include DHT sensor library
//=====================================================================
//      Definition and object instantiation of DHT11 module
//=====================================================================
#define DHTPIN 13 // Define pin number to which DHT sensor is connected
#define DHTTYPE DHT11 // Define type of DHT sensor in use, here DHT11
// Create DHT sensor object with defined pin and type
DHT dht(DHTPIN, DHTTYPE);
//======================================================================


//======================================================================
//                   Libraries, definition, variables and 
//              object instatiation for Thingsboard connection
//======================================================================
//Wifi connection using MQTT prtotcol pre-req
#include <WiFi.h>
#include <PubSubClient.h>

// Replace with your network credentials
const char* ssid = "TASIGEN's A15";
const char* wifiPassword = "106801ts";

// Replace with your ThingsBoard device token and server address
const char* mqttServer = "demo.thingsboard.io";
const char* token = "nxCZBt4rq7bVwHiskodJ";



WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
//=====================================================================


//=====================================================================
//  Function for setting up WiFi and etablishing MQTT communication
//=====================================================================

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
//==========================================================================

//==========================================================================
          // Prerequisites for the RFID library
//==========================================================================
#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 5
#define RST_PIN 22

MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::Uid acceptedUid;

bool tagScanned = false;
//==========================================================================


//==========================================================================
//        To enable communication between Arduino Serially
//==========================================================================
// Define TX and RX pins for UART (change if needed)
#define TXD1 17
#define RXD1 16

// Use Serial1 for UART communication
HardwareSerial mySerial(2);
//==========================================================================

void setup() {
  Serial.begin(9600);
  pinMode(SWITCH_SCREEN, OUTPUT);
  digitalWrite(SWITCH_SCREEN, HIGH);
  //=======================================================================
        // Part of th coe that ensure RFID to be scanned before proceeding
  //=======================================================================
  SPI.begin();       // Init SPI bus
  rfid.PCD_Init();   // Init MFRC522

  Serial.println("Scan your RFID tag to begin...");

  // Wait for RFID tag before proceeding to loop
  while (!tagScanned) {
    if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
      Serial.print("RFID UID: ");
      for (byte i = 0; i < rfid.uid.size; i++) {
        Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
        Serial.print(rfid.uid.uidByte[i], HEX);
        acceptedUid.uidByte[i] = rfid.uid.uidByte[i];
      }
      acceptedUid.size = rfid.uid.size;
      Serial.println();
      tagScanned = true;

      // Halt PICC
      rfid.PICC_HaltA();
      // Stop encryption on PCD
      rfid.PCD_StopCrypto1();
    }
  }

  Serial.println("Access granted.");
  //=======================================================================
  digitalWrite(SWITCH_SCREEN, LOW);
  setupWiFi();
  mqttClient.setServer(mqttServer, 1883);

  dht.begin(); // Initialize DHT sensor
  Serial.begin(9600);

  //=========================================================================
  //        Uart Communication initialization
  //=========================================================================
  mySerial.begin(9600, SERIAL_8N1, RXD1, TXD1);  // UART setup
  
  Serial.println("ESP32 UART Receiver");
  //=========================================================================

  delay(2000); // Wait for 2 seconds
}

void loop() {

  if (!mqttClient.connected()) {
    reconnectMQTT();
  }
  mqttClient.loop();

  delay(5000); // Wait for 5 seconds
  String message;
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
  if (mySerial.available()) {
    // Read data and display it
    message = mySerial.readStringUntil('\n');
    Serial.println("Received: " + message);
  }
  //String message = "{\"Sleepiness\": 25, \"Happiness\": 23, \"Hunger\":10 , \"Age\":40, \"BeardLength\":10, \"Expression\":\"Neutral\", \"Clothing\":\"Shirt\"}";
  // Do something with the humidity and temperature values here
  printf("%.2f%, %.2f Celsius\n", h, t);


  // Build a JSON payload with the random numbers
  char pld[300];
  snprintf(pld, sizeof(pld), "{\"Humidity\":%.2f, \"Temperature\":%.2f}", h, t);

  if (mqttClient.publish("v1/devices/me/telemetry", pld)) {
    Serial.println("Telemetry data published");
  } else {
    Serial.println("Publish failed");
  }

  delay(5000); // Wait for 5 second before the next loop iteration
}


