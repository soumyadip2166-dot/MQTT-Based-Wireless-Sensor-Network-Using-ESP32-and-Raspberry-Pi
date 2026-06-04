#include <ESP8266WiFi.h>
#include <MQTT.h>
#include <SPI.h>
#include <Wire.h>
#include "MQ135.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include <Adafruit_Sensor.h>
#define SEALEVELPRESSURE_HPA (1013.25)
#define DHT_SENSOR_PIN  12 // ESP8266 pin GPIO12 connected to DHT22 sensor
#define DHT_SENSOR_TYPE DHT22


WiFiClient net;
MQTTClient client;

String ssid="ProjectWifi";
String pass="237a27d13B";
unsigned long lastMillis = 0;
 
#define SCREEN_WIDTH 128    // OLED display width, in pixels
#define SCREEN_HEIGHT 64    // OLED display height, in pixels
#define OLED_RESET -1       // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
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
 
float tem = 27.0; // Assume current temperature.
float hum = 64.0; // Assume current humidity. 

 
void setup()
{
  Serial.begin(115200);
  dht_sensor.begin(); // initialize the DHT sensor
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); //initialize with the I2C addr 0x3C (128x64)
  display.clearDisplay();
  WiFi.begin(ssid, pass);
  client.begin("192.168.1.102", net);
  connect();
  delay(10);
} 
 
void loop()
{
  float humi  = dht_sensor.readHumidity();    // read humidity
  float tempC = dht_sensor.readTemperature(); // read temperature
  MQ135 gasSensor = MQ135(A0);
  float air_quality = gasSensor.getCorrectedPPM(tempC, humi);
  Serial.print("Air Quality: ");  
  Serial.print(air_quality);
  Serial.println("  PPM");   
  Serial.println();
  Serial.print("Temperature: " + (String) tempC);
  Serial.print("  ");
  Serial.print("Humidity: " + (String) humi);
  Serial.print("\n");
  display.clearDisplay();
  display.setCursor(0,0);  //oled display
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.println("Air Quality Index");
  display.setCursor(0,10);  //oled display
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.print(air_quality);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.println(" PPM");
  display.display();
  // check whether the reading is successful or not
  if (isnan(tempC) || isnan(humi)) {
    display.setCursor(0,20);  //oled display
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.println("Failed");
  } else {
    display.setCursor(0,20);  //oled display
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.println("Temp: ");
    display.setCursor(0,30);  //oled display
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.print(tempC);     // display the temperature
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.println("°C");
    display.display();
    display.setCursor(0,40);  //oled display
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.println("Humidity: ");
    display.setCursor(0,50);  //oled display
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.print(humi);      // display the humidity
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.println("%");
    display.display();
  }


  if (!client.connected()) {
    connect();
  }

  // publish a message roughly every 5 second.
  if (millis() - lastMillis > 5000) {
    lastMillis = millis();
    if (isnan(air_quality)) {
      Serial.println("Failed to read from MQ135 sensor!");
      return;
    }
    client.publish("nodemcu/mq","{\"airquality\":"+String(air_quality)+",\"temperature\":"+String(tempC)+", \"humidity\":"+String(humi)+"}");
  }  
  delay(2000);      
}
