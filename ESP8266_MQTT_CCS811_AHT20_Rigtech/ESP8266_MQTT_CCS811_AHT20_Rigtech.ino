#include <PubSubClient.h>
#include "ccs811.h"  // CCS811 library
#include <Wire.h>    // I2C library
#include "EspMQTTClient.h" 
#include "AHT20.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// EspMQTTClient client(
//   "Begemot12",
//   "9045557432",
//   "dev.rightech.io",  // MQTT Broker server ip
//   "mqtt-ccs811-esp8266",
//   1883
// );

// EspMQTTClient client(
//   "Begemot12",
//   "9045557432",
//   "dev.rightech.io",  // MQTT Broker server ip
//   "pavelpigin",   // Can be omitted if not needed
//   "35200002",  // Can be omitted if not needed
//   "mqtt-pavel_pigin",
//   1883
// );

EspMQTTClient client(
  "Begemot12", // Wifi SSID
  "9045557432", // Wifi Password
  "88.214.35.144",  // MQTT Broker server ip
  "pasha",   // Can be omitted if not needed
  "pigin",  // Can be omitted if not needed
  "house", // device name
  1883 // MQTT server port
);

// Wiring for ESP8266 NodeMCU boards: VDD to 3V3, GND to GND, SDA to D2, SCL to D1, nWAKE to D3
CCS811 ccs811(D3);
AHT20 aht20;

#define SCREEN_WIDTH 128    // OLED display width, in pixels
#define SCREEN_HEIGHT 64    // OLED display height, in pixels
#define OLED_RESET -1       // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

unsigned long previousMillis = 0;

void setup() {
  Serial.begin(115200);

  // Enable I2C
  Wire.begin(); 

  client.enableDebuggingMessages();

  setupCCS811();

  setupAHT20();

  setupDisplay();
}

void loop() {
  client.loop();

  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= 10000) {

    previousMillis = currentMillis;

    float humidity = aht20.getHumidity();
    float temperature = aht20.getTemperature();

    ccs811.set_envdata_Celsius_percRH(temperature, humidity);

    // Read
    uint16_t eco2, etvoc, errstat, raw;
    ccs811.read(&eco2, &etvoc, &errstat, &raw); 

    if (errstat == CCS811_ERRSTAT_OK ) { 
      Serial.print("CCS811: ");
      Serial.print("eco2=");  Serial.print(eco2); Serial.print(" ppm  ");
      Serial.print("etvoc="); Serial.print(etvoc); Serial.print(" ppb  ");
      Serial.print("temp="); Serial.print(temperature); Serial.print(" C  ");
      Serial.print("hum="); Serial.print(humidity); Serial.print(" %  ");
      Serial.println();

      // client.publish("airsense/eco2", String(eco2));
      // client.publish("airsense/tvoc", String(etvoc));
      // client.publish("airsense/temp", String(temperature));
      // client.publish("airsense/hum", String(humidity));

      client.publish("house/eco2", String(eco2));
      client.publish("house/tvoc", String(etvoc));
      client.publish("house/temp", String(temperature));
      client.publish("house/hum", String(humidity));

      display.clearDisplay();

      display.setTextSize(2);
      display.setCursor(0, 0);
      display.print(temperature);
      display.setTextSize(1);
      display.print("O");
      display.setTextSize(2);
      display.print("C");

      display.setCursor(90, 0);
      display.print((int) humidity);
      display.print("%");
      
      display.setTextSize(2);
      display.setCursor(0, 25);
      display.print("CO2:");
      display.print(eco2);
      display.setTextSize(1);
      display.print(" ppm");
 
      display.setTextSize(2);
      display.setCursor(0, 50);
      display.print("TVOC:");
      display.print(etvoc);
      display.display();

    } else if (errstat == CCS811_ERRSTAT_OK_NODATA) {
      Serial.println("CCS811: waiting for (new) data");

      display.clearDisplay();
      display.setTextSize(2);
      display.setCursor(0, 5);
      display.print("CCS811: waiting");

    } else if (errstat & CCS811_ERRSTAT_I2CFAIL) { 

      Serial.println("CCS811: I2C error"); 
      showErrorOnDisplay();

    } else {
      Serial.print("CCS811: errstat ="); Serial.print(errstat, HEX); 
      Serial.print("="); Serial.println( ccs811.errstat_str(errstat)); 
      showErrorOnDisplay();
    }
  }
}

void showErrorOnDisplay() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 5);
  display.print("ERROR!");
}

void setupDisplay() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); //initialize with the I2C addr 0x3C (128x64)
  delay(500);
  display.clearDisplay();
  display.setCursor(25, 15);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.println("CCS811 Sensor");
  display.setCursor(25, 35);
  display.setTextSize(1);
  display.print("Initializing");
  display.display();
}

void onConnectionEstablished() {
  Serial.println("CONNECTED MQTT");
}

void setupCCS811() {
  // Enable CCS811
  ccs811.set_i2cdelay(50);
  bool ok = ccs811.begin();
  if(!ok) Serial.println("setup: CCS811 begin FAILED");


  
  // Start measuring
  ok = ccs811.start(CCS811_MODE_1SEC);
  if(!ok) Serial.println("setup: CCS811 start FAILED");
}

void setupAHT20() {
  if (aht20.begin() == false) {
    Serial.println("AHT20 not detected. Please check wiring. Freezing.");
    while (1);
  }

  Serial.println("AHT20 acknowledged.");
}
