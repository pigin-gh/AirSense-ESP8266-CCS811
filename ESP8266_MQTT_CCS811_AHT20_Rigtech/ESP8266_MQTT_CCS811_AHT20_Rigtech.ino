#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include "ccs811.h"  // CCS811 library
#include <Wire.h>    // I2C library
#include "EspMQTTClient.h" 
#include "AH_EasyDriver.h"  //http://www.alhin.de/arduino/downloads/AH_EasyDriver_20120512.zip
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

void onConnectionEstablished() {
  Serial.println("connected");
}

void setup() {
  // Enable serial
  Serial.begin(115200);
  Serial.println("");
  Serial.println("setup: Starting CCS811 basic demo");
  Serial.print("setup: ccs811 lib  version: "); Serial.println(CCS811_VERSION);

  // Enable I2C
  Wire.begin(); 
  
  // Enable CCS811
  ccs811.set_i2cdelay(500); // Needed for ESP8266 because it doesn't handle I2C clock stretch correctly
  bool ok= ccs811.begin();
  if( !ok ) Serial.println("setup: CCS811 begin FAILED");

  // Print CCS811 versions
  Serial.print("setup: hardware    version: "); Serial.println(ccs811.hardware_version(), HEX);
  Serial.print("setup: bootloader  version: "); Serial.println(ccs811.bootloader_version(), HEX);
  Serial.print("setup: application version: "); Serial.println(ccs811.application_version(), HEX);
  
  // Start measuring
  ok= ccs811.start(CCS811_MODE_1SEC);
  if( !ok ) Serial.println("setup: CCS811 start FAILED");

    if (aht20.begin() == false)
  {
    Serial.println("AHT20 not detected. Please check wiring. Freezing.");
    while (1);
  }
  Serial.println("AHT20 acknowledged.");
}

void loop() {
  client.loop();
  
  ccs811.set_envdata(aht20.getTemperature(), aht20.getHumidity());

  // Read
  uint16_t eco2, etvoc, errstat, raw;
  ccs811.read(&eco2, &etvoc, &errstat, &raw); 

  if (errstat == CCS811_ERRSTAT_OK ) { 
    Serial.print("CCS811: ");
    Serial.print("eco2=");  Serial.print(eco2);     Serial.print(" ppm  ");
    Serial.print("etvoc="); Serial.print(etvoc);    Serial.print(" ppb  ");
    Serial.print("t="); Serial.print(aht20.getTemperature());    Serial.print(" C  ");
    Serial.print("h="); Serial.print(aht20.getHumidity());    Serial.print(" %  ");

    client.publish("ccs811/eco2", String(eco2));
    client.publish("ccs811/tvoc", String(etvoc));
    client.publish("aht20/temp", String(aht20.getTemperature()));
    client.publish("aht20/hum", String(aht20.getHumidity()));

    Serial.println();

  } else if( errstat == CCS811_ERRSTAT_OK_NODATA ) {
    Serial.println("CCS811: waiting for (new) data");

  } else if( errstat & CCS811_ERRSTAT_I2CFAIL ) { 
    Serial.println("CCS811: I2C error"); 

  } else {
    Serial.print("CCS811: errstat="); Serial.print(errstat,HEX); 
    Serial.print("="); Serial.println( ccs811.errstat_str(errstat) ); 
  }
  
  delay(10000);
}
