extern "C"
{
#include "user_interface.h"
}

#include <SoftwareSerial.h>
#include <NeoPixelBrightnessBus.h>
#include <TimeLib.h>
#include <NtpClientLib.h>
#include "fauxmoESP.h"

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include <EEPROM.h>
#include <ESP8266mDNS.h>

#include "WiFiManager.h" //https://github.com/tzapu/WiFiManager

#define BAUD (115200) // Baudrate from Arduino to Host-System

void logDebugData()
{
  Serial.println();
  Serial.print(F("Heap: "));
  Serial.println(system_get_free_heap_size());
  Serial.print(F("Boot Vers: "));
  Serial.println(system_get_boot_version());
  Serial.print(F("CPU: "));
  Serial.println(system_get_cpu_freq());
  Serial.print(F("SDK: "));
  Serial.println(system_get_sdk_version());
  Serial.print(F("Chip ID: "));
  Serial.println(system_get_chip_id());
  Serial.print(F("Flash ID: "));
  Serial.println(spi_flash_get_id());
  Serial.print(F("Flash Size: "));
  Serial.println(ESP.getFlashChipRealSize());
  Serial.print(F("Vcc: "));
  Serial.println(ESP.getVcc());
  Serial.println();
}

void setup()
{
  // General Setup
  // Open the Serial-interface to host
  Serial.begin(BAUD);
  delay(100);
  Serial.setDebugOutput(true);

  logDebugData();

  Serial.println();
  Serial.println("setup called. Showing current Data now...");

  Serial.print("Hours from Time: ");
  Serial.println(receiveHour());

  Serial.print("Minutes from Time: ");
  Serial.println(receiveMinute());

  Serial.println("Setting up Basic LED stuff");
  setupLED();
  ledAnimationShowOffline();
  delay(2000);
  ledAnimationBooting();
  delay(2000);
  
  //Setup Webserver-Modul Stuff
  setupWebserver();

  //Configure Brightness Sensor and calculate the values
  setupBrightness();

  //Setup NTP Stuff for syncing time from the internet (this must be done after
  //Webserver initialization as the wifi connection isn't available earlier)
  setupNTP();

  //Setup Alexa Stuff
  setupAlexa();

  //Setup LED-Modul Stuff
  delay(1000);
}

void loop()
{
  loopWebserver(); //Check Webserver activities
  loopAlexa();     //Check the incoming Alexa commands
  loopNTP();       //NTP Event Handling. This is NO sync-trigger, but the handling of NTP result (if available)
  loopLED();       //Get Time and SET the related LED's
}
