#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

#define DHTPIN D7          // GPIO pin where the DHT sensor is connected
#define DHTTYPE DHT22      // DHT22 sensor type

// WiFi credentials
const char* ssid = "AIHome";
const char* password = "**********";

// MQTT server details
const char* mqtt_server = "aihome.local";
const char* mqtt_user = "*****";
const char* mqtt_password = "*****";

// Initialize objects
WiFiClient espClient;
PubSubClient mqttClient(espClient);
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);       // Start the serial communication
  connectToWiFi();            // Connect to WiFi
  mqttClient.setServer(mqtt_server, 1883); // Setup MQTT + port
  dht.begin();                // Start the DHT sensor
}

void connectToWiFi() {
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  if (!mqttClient.connected()) {
    reconnectToMQTT();
  }
  mqttClient.loop();
  readAndSendSensorData();
  delay(15000);               // Wait 1 minute before next read
}

void reconnectToMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (mqttClient.connect("ESP8266ClientDHT1", mqtt_user, mqtt_password)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void readAndSendSensorData() {
  float temp = dht.readTemperature();
  float humidity = dht.readHumidity();
  if (isnan(temp) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Format temperature with one decimal place
  char tempPayload[10];
  snprintf(tempPayload, sizeof(tempPayload), "%.1f", temp);
  // Format humidity with percentage symbol
  char humidityPayload[10];
  snprintf(humidityPayload, sizeof(humidityPayload), "%.0f%%", humidity);

  mqttClient.publish("GuestRoom/Temperature/temperature", tempPayload);
  mqttClient.publish("GuestRoom/Humidity/humidity", humidityPayload);
}
