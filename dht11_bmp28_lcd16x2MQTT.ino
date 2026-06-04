#include <Wire.h>
#include <WiFi.h>
#include <MQTT.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#define SEALEVELPRESSURE_HPA (1013.25)
#define DHT_SENSOR_PIN  2 // ESP32 pin GPIO23 connected to DHT11 sensor
#define DHT_SENSOR_TYPE DHT11

WiFiClient net;
MQTTClient client;

String ssid="ProjectWifi";
String pass="237a27d13B";
unsigned long lastMillis = 0;

LiquidCrystal_I2C lcd(0x27, 16, 2);  // I2C address 0x24, 16 column and 2 rows
DHT dht_sensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);
Adafruit_BMP280 bme;
float temperature, humidity, pressure, altitude;


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
  bme.begin(0x76);  ; // initialize the BME sensor
  lcd.init();         // initialize the lcd
  lcd.backlight();    // open the backlight
  WiFi.begin(ssid, pass);
  client.begin("192.168.1.102",net);
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
  temperature = bme.readTemperature();
  pressure = bme.readPressure() / 100.0F;
  altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
  Serial.print("Temperature: " + (String) temperature);
  Serial.print("\n");
  Serial.print("Pressure: " + (String) pressure);
  Serial.print("\n");
  Serial.print("Altitude: " + (String) altitude);
  Serial.print("\n");
  lcd.clear();
  // check whether the reading is successful or not
  if (isnan(tempC) || isnan(humi) || isnan(temperature) || isnan(humidity)) {
    lcd.setCursor(0, 0);
    lcd.print("Failed");
  } else {
    lcd.setCursor(0, 0);  // display position
    lcd.print("T: ");
    lcd.print(tempC);     // display the temperature
    lcd.print("°C");

    lcd.setCursor(8, 0);  // display position
    lcd.print("H: ");
    lcd.print(humi);      // display the humidity
    lcd.print("%");

    lcd.setCursor(0, 1);  // display position
    lcd.print("Tem: ");
    lcd.print(temperature);     // display the temperature
    lcd.print("°C");
    
    lcd.setCursor(8, 1);  // display position
    lcd.print("P: ");
    lcd.print(pressure);     // display the pressure
    lcd.print("bar");
  }

  client.loop();
  if (!client.connected()) {
    connect();
  }

  // publish a message roughly every 5 second.
  if (millis() - lastMillis > 5000) {
    lastMillis = millis();
    if (isnan(humi) || isnan(tempC) || isnan(pressure)) {
      Serial.println("Failed to read from sensors!");
      return;
    }
    client.publish("esp/dht","{\"temperature\":"+String(tempC)+", \"humidity\":"+String(humi)+", \"pressure\":"+String(pressure)+"}");
  } 

  // wait a 2 seconds between readings
  delay(2000);
}
