#include <WiFi.h>
#include <PubSubClient.h>

// Pin Definitions
#define BUTTON_PIN 18    // GPIO Pin for the emergency button
#define LED_GREEN 2      // GPIO Pin for status LED (green)
#define LED_RED 4        // GPIO Pin for status LED (red)

// WiFi Credentials
const char* ssid = "S4-Family";
const char* password = "sreenu78";

// MQTT Configuration
const char* mqtt_server = "10.108.12.43";  // Your local Mosquitto broker IP
const int mqtt_port = 1883;  // Standard MQTT port for Mosquitto
const char* mqtt_topic = "home/elderly/emergency";

// Global Variables
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastPressTime = 0;
const unsigned long debounceDelay = 500;
unsigned long lastReconnectAttempt = 0;
const unsigned long reconnectInterval = 5000; // 5 seconds

void setup() {
  // Initialize Serial
  Serial.begin(115200);
  delay(1000); // Give time for serial to initialize
  Serial.println("\n🚀 Emergency Button System Starting...");

  // Initialize Pins
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  
  // Initial LED states
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_RED, HIGH);  // Red LED on until connected

  // Connect to WiFi
  connectWiFi();
  
  // Setup MQTT
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);  // Set callback for incoming messages
  
  // Initial MQTT connection
  connectMQTT();
}

void loop() {
  // Check MQTT connection
  if (!client.connected()) {
    unsigned long now = millis();
    if (now - lastReconnectAttempt > reconnectInterval) {
      lastReconnectAttempt = now;
      connectMQTT();
    }
  } else {
    lastReconnectAttempt = 0;
  }
  
  client.loop();

  // Check button press
  if (digitalRead(BUTTON_PIN) == LOW) {
    unsigned long currentTime = millis();
    if (currentTime - lastPressTime > debounceDelay) {
      sendEmergencyAlert();
      lastPressTime = currentTime;
    }
  }
}

void sendEmergencyAlert() {
  if (client.connected()) {
    Serial.println("🚨 Emergency Button Pressed! Sending MQTT Alert...");
    
    // Blink red LED to indicate sending
    digitalWrite(LED_RED, HIGH);
    delay(100);
    digitalWrite(LED_RED, LOW);
    
    // Publish emergency message
    bool success = client.publish(mqtt_topic, "SOS", true);  // true for retain flag
    
    if (success) {
      Serial.println("✅ MQTT Alert Sent Successfully!");
      // Visual feedback - quick green blink
      digitalWrite(LED_GREEN, HIGH);
      delay(100);
      digitalWrite(LED_GREEN, LOW);
    } else {
      Serial.println("❌ MQTT Alert Failed! Check Connection.");
      // Visual feedback - quick red blink
      digitalWrite(LED_RED, HIGH);
      delay(100);
      digitalWrite(LED_RED, LOW);
    }
  } else {
    Serial.println("❌ Cannot send alert - MQTT disconnected!");
    // Visual feedback - red LED on
    digitalWrite(LED_RED, HIGH);
  }
}

void connectWiFi() {
  Serial.print("📡 Connecting to WiFi");
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    // Blink red LED while connecting
    digitalWrite(LED_RED, !digitalRead(LED_RED));
  }
  
  Serial.println("\n✅ Connected to WiFi");
  Serial.print("📱 IP Address: ");
  Serial.println(WiFi.localIP());
  
  // Test connection to MQTT server
  Serial.print("Pinging MQTT server...");
  IPAddress mqttIP;
  if(WiFi.hostByName(mqtt_server, mqttIP)) {
    Serial.print("IP: ");
    Serial.println(mqttIP);
  } else {
    Serial.println("DNS lookup failed");
  }
  
  // Visual feedback - green LED on
  digitalWrite(LED_GREEN, HIGH);
  digitalWrite(LED_RED, LOW);
}

void connectMQTT() {
  if (client.connected()) return;

  Serial.print("🔌 Connecting to MQTT...");
  
  // Generate a unique client ID
  String clientId = "ESP32Client-";
  clientId += String(random(0xffff), HEX);
  
  // Set a longer timeout for the connection
  client.setSocketTimeout(10);
  
  if (client.connect(clientId.c_str())) {
    Serial.println("\n✅ Connected to MQTT broker!");
    
    // Subscribe to topic
    client.subscribe(mqtt_topic);
    
    // Visual feedback
    digitalWrite(LED_GREEN, HIGH);
    digitalWrite(LED_RED, LOW);
  } else {
    Serial.println("\n❌ MQTT Connection Failed!");
    Serial.print("   Error code: ");
    Serial.println(client.state());
    
    // Print more detailed error information
    switch(client.state()) {
      case -4: Serial.println("   MQTT_CONNECTION_TIMEOUT - the server didn't respond within the keepalive time"); break;
      case -3: Serial.println("   MQTT_CONNECTION_LOST - the network connection was broken"); break;
      case -2: Serial.println("   MQTT_CONNECT_FAILED - the network connection failed"); break;
      case -1: Serial.println("   MQTT_DISCONNECTED - the client is disconnected"); break;
      case 0: Serial.println("   MQTT_CONNECTED - the client is connected"); break;
      case 1: Serial.println("   MQTT_CONNECT_BAD_PROTOCOL - the server doesn't support the requested version of MQTT"); break;
      case 2: Serial.println("   MQTT_CONNECT_BAD_CLIENT_ID - the server rejected the client identifier"); break;
      case 3: Serial.println("   MQTT_CONNECT_UNAVAILABLE - the server was unable to accept the connection"); break;
      case 4: Serial.println("   MQTT_CONNECT_BAD_CREDENTIALS - the username/password were rejected"); break;
      case 5: Serial.println("   MQTT_CONNECT_UNAUTHORIZED - the client was not authorized to connect"); break;
      default: Serial.println("   Unknown error"); break;
    }
    
    // Visual feedback
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_RED, HIGH);
  }
}

// Callback function for incoming MQTT messages
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("📩 Message received on topic: ");
  Serial.println(topic);
  
  // Convert payload to string
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  Serial.print("📝 Message: ");
  Serial.println(message);
  
  // Visual feedback for received message
  digitalWrite(LED_GREEN, HIGH);
  delay(100);
  digitalWrite(LED_GREEN, LOW);
} 