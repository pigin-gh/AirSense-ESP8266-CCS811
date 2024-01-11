#include <PubSubClient.h>
#include "ccs811.h"  // CCS811 library
#include <Wire.h>    // I2C library
#include "EspMQTTClient.h" 
#include "AHT20.h"

EspMQTTClient client(
  "Begemot12",
  "9045557432",
  "dev.rightech.io",  // MQTT Broker server ip
  "pavelpigin",   // Can be omitted if not needed
  "Pavel35200002",  // Can be omitted if not needed
  "mqtt-pavel_pigin",
  1883
);

// Wiring for ESP8266 NodeMCU boards: VDD to 3V3, GND to GND, SDA to D2, SCL to D1, nWAKE to D3
CCS811 ccs811(D3);
AHT20 aht20;

void setup() {
  Serial.begin(115200);

  // Enable I2C
  Wire.begin(); 

  setupCCS811();

  setupAHT20();
}

void loop() {
  client.loop();

  float humidity = aht20.getHumidity();
  float temperature = aht20.getTemperature();

  ccs811.set_envdata(temperature, humidity);

  // Read
  uint16_t eco2, etvoc, errstat, raw;
  ccs811.read(&eco2, &etvoc, &errstat, &raw); 

  if (errstat == CCS811_ERRSTAT_OK ) { 
    Serial.print("CCS811: ");
    Serial.print("eco2=");  Serial.print(eco2); Serial.print(" ppm  ");
    Serial.print("etvoc="); Serial.print(etvoc); Serial.print(" ppb  ");
    Serial.print("temp="); Serial.print(temperature); Serial.print(" C  ");
    Serial.print("hum="); Serial.print(humidity); Serial.print(" %  ");

    client.publish("ccs811/eco2", String(eco2));
    client.publish("ccs811/tvoc", String(etvoc));
    client.publish("aht20/temp", String(temperature));
    client.publish("aht20/hum", String(humidity));

  } else if (errstat == CCS811_ERRSTAT_OK_NODATA) {
    Serial.println("CCS811: waiting for (new) data");

  } else if (errstat & CCS811_ERRSTAT_I2CFAIL) { 
    Serial.println("CCS811: I2C error"); 

  } else {
    Serial.print("CCS811: errstat ="); Serial.print(errstat, HEX); 
    Serial.print("="); Serial.println( ccs811.errstat_str(errstat)); 
  }
  
  delay(10000);
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
  ok = ccs811.start(CCS811_MODE_10SEC);
  if(!ok) Serial.println("setup: CCS811 start FAILED");
}

void setupAHT20() {
  if (aht20.begin() == false) {
    Serial.println("AHT20 not detected. Please check wiring. Freezing.");
    while (1);
    }
  Serial.println("AHT20 acknowledged.");
}
