#WSN - 3

#include <ESP8266WiFi.h>
#include <MQTT.h>
#include <SPI.h>
#include <Wire.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#define SEALEVELPRESSURE_HPA (1013.25)
#define DHT_SENSOR_PIN  12 // ESP8266 pin GPIO12 connected to DHT22 sensor
#define DHT_SENSOR_TYPE DHT22


WiFiClient net;
MQTTClient client;

String ssid="SSID";
String pass="PASSWORD";
unsigned long lastMillis = 0;
 
LiquidCrystal_I2C lcd(0x27, 16, 2);  // I2C address 0x27, 16 column and 2 rows
DHT dht_sensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);
float temperature, humidity;
 
void connect() {
  Serial.print("checking wifi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }

  Serial.print("\nconnecting...");
  while (!client.connect("nodemcu_client")) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("\nconnected!");
}
 

void setup() {
  Serial.begin(115200);
  dht_sensor.begin(); // initialize the DHT sensor
  lcd.init();         // initialize the lcd
  lcd.backlight();    // open the backlight
  WiFi.begin(ssid, pass);
  client.begin("IP Address of the MQTT Server",net);
  client.onMessage(messageReceived);
  connect();
  delay(10);
}



void messageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);

  // Note: Do not use the client in the callback to publish, subscribe or
  // unsubscribe as it may cause deadlocks when other things arrive while
  // sending and receiving acknowledgments. Instead, change a global variable,
  // or push to a queue and handle it in the loop after calling `client.loop()`.
}



void loop() {
  float humi  = dht_sensor.readHumidity();    // read humidity
  float tempC = dht_sensor.readTemperature(); // read temperature
  Serial.print("Temperature: " + (String) tempC);
  Serial.print("  ");
  Serial.print("Humidity: " + (String) humi);
  Serial.print("\n");
  // check whether the reading is successful or not
  if (isnan(tempC) || isnan(humi)) {
    lcd.setCursor(0, 0);
    lcd.print("Failed");
  } else {
    lcd.setCursor(0, 0);  // display position
    lcd.print("Temp: ");
    lcd.print(tempC);     // display the temperature
    lcd.print("°C");

    lcd.setCursor(0, 1);  // display position
    lcd.print("Humidity: ");
    lcd.print(humi);      // display the humidity
    lcd.print("%");
  }

  client.loop();
  if (!client.connected()) {
    connect();
  }

  // publish a message roughly every 5 second.
  if (millis() - lastMillis > 5000) {
    lastMillis = millis();
    if (isnan(humi) || isnan(tempC)) {
      Serial.println("Failed to read from sensors!");
      return;
    }
    client.publish("esp/dht2","{\"temperature\":"+String(tempC)+", \"humidity\":"+String(humi)+"}");
  } 

  // wait a 2 seconds between readings
  delay(2000);
}
