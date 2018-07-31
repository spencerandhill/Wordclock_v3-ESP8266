#include <SoftwareSerial.h>

#include <NeoPixelBrightnessBus.h>
#include <Wire.h>
#include <TimeLib.h>
#include <NtpClientLib.h>

#define BAUD (115200) // Baudrate from Arduino to Host-System

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

  //Setup Webserver-Modul Stuff
  setupWebserver();

  //Setup NTP Stuff for syncing time from the internet (this must be done after
  //Webserver initialization as the wifi connection isn't available earlier)
  setupNTP();

  setManualTime(1);
  setUserTime(12, 59);

  //Setup LED-Modul Stuff
  delay(1000);
}

void loop()
{
  loopWebserver(); //Check Webserver activities
  loopNTP();       //NTP Event Handling. This is NO sync-trigger, but the handling of NTP result (if available)
  loopLED();       //Get Time from RTC Module and SET the related LED's
}

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