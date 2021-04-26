ESP8266WebServer server(80);

//WiFiManager: Handles automatic WiFi Connection if available or creates an AP, if not
//Local intialization. Once its business is done, there is no need to keep it around
WiFiManager wifiManager;

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

uint8_t currentTimeServerIndex = 1; // Index number of which timeServer is current

uint8_t manualTime = 0;

typedef struct
{
  String timeServerURL;
  String name;
} TimeServerAndName;
typedef TimeServerAndName TimeServerAndNameList[];

// List of timeServers to cycle through.  Each is dynamically allocated in web-interface from this list
TimeServerAndNameList timeServers = {
    {"ptbtime1.ptb.de", "PTB 1"},
    {"ptbtime2.ptb.de", "PTB 2"},
    {"ptbtime3.ptb.de", "PTB 3"},
    {"0.europe.pool.ntp.org", "Google 1"},
    {"1.europe.pool.ntp.org", "Google 2"},
};

const uint8_t timeServerCount = ARRAY_SIZE(timeServers);

void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());

  ledAnimationShowAP();

  //In AP mode, disable auto reconnect
  WiFi.setAutoReconnect (false);

  Serial.println(myWiFiManager->getConfigPortalSSID());
}

void saveConfigCallback () {
  Serial.println("Should save config");

  //When WiFi Network is set, enable auto reconnect
  WiFi.setAutoReconnect (true);
}

void setupWebserver(void)
{
  Serial.printf("Configuring WiFi");
  WiFi.setAutoReconnect (true); //By default this is set to true, so that a lost connection gets reconnected

  wifiManager.setAPCallback(configModeCallback);
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  //fetches ssid and pass from eeprom and tries to connect
  //if it does not connect it starts an access point with the specified name
  //and goes into a blocking loop awaiting configuration
  Serial.println("autoConnect");
  ledAnimationShowConnecting();
  String generatedSSID = "LR Wordclock " + String(ESP.getChipId());
  wifiManager.setConnectTimeout(30);
  wifiManager.autoConnect(generatedSSID.c_str());

  ledAnimationShowOnline();
  Serial.print("Connected! Open http://");
  Serial.print(WiFi.localIP());
  Serial.println(" in your browser");
  Serial.println("WiFi successful configured");

  //Start multiDNS (mDNS)
  if (!MDNS.begin("wordclock")) {             // Start the mDNS responder for wordclock.local
    Serial.println("Error setting up MDNS responder!");
  }
  Serial.println("mDNS responder started");

  Serial.println("EEPROM Begin");
  EEPROM.begin(512);
  initBrightnessSettings();
  loadSettings();

  Serial.println("Open SPIFFS");
  SPIFFS.begin();
  {
    Serial.println("Files found: ");
    Dir dir = SPIFFS.openDir("/");
    while (dir.next())
    {
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      Serial.printf("FS File: %s, size: %s\n", fileName.c_str(), String(fileSize).c_str());
    }
    Serial.printf("\n");
  }

  //Define URL's for interface
  Serial.println("Setting up Webserver");
  Serial.println("Define Interface URL's");
  server.on("/all", HTTP_GET, []() {
    sendAll();

    Serial.printf("/all triggered\n");
  });

  server.on("/power", HTTP_GET, []() {
    sendPower();
    Serial.printf("/power triggered\n");
  });

  server.on("/power", HTTP_POST, []() {
    String value = server.arg("value");
    setPower(value.toInt());
    sendPower();
    Serial.printf("/power (POST) triggered with value: power:%s\n", value.c_str());
  });

  server.on("/solidColor", HTTP_GET, []() {
    sendSolidColor();
    Serial.printf("/solidColor triggered\n");
  });

  server.on("/solidColor", HTTP_POST, []() {
    String r = server.arg("r");
    String g = server.arg("g");
    String b = server.arg("b");
    setSolidColor(r.toInt(), g.toInt(), b.toInt());
    sendSolidColor();
    Serial.printf("/solidColor (POST) triggered with values: r:%s g:%s b:%s\n", r.c_str(), g.c_str(), b.c_str());
  });

  server.on("/manualTime", HTTP_GET, []() {
    sendManualTime();
    Serial.printf("/manualTime triggered\n");
  });

  server.on("/manualTime", HTTP_POST, []() {
    String value = server.arg("value");
    setManualTime(value.toInt());
    sendManualTime();
    Serial.printf("/manualTime (POST) triggered with value: manualTime:%s\n", value.c_str());
  });

  server.on("/timeServer", HTTP_GET, []() {
    sendTimeServers();
    Serial.printf("/timeServer triggered\n");
  });

  server.on("/timeServer", HTTP_POST, []() {
    String value = server.arg("value");
    setTimeServer(value.toInt());
    sendTimeServers();
    Serial.printf("/timeServer (POST) triggered with value: timeServerId:%s", String(currentTimeServerIndex).c_str());
    Serial.printf(", timeServerName:%s", timeServers[currentTimeServerIndex].name.c_str());
    Serial.printf(", timeServerURL:%s\n", timeServers[currentTimeServerIndex].timeServerURL.c_str());
  });

  server.on("/timeServerUp", HTTP_POST, []() {
    adjustTimeServer(true);
    sendTimeServers();
    Serial.printf("/timeServerUp triggered\n");
  });

  server.on("/timeServerDown", HTTP_POST, []() {
    adjustTimeServer(false);
    sendTimeServers();
    Serial.printf("/timeServerDown triggered\n");
  });

  server.on("/userTimeAndSensorValue", HTTP_GET, []() {
    sendUserTimeAndSensorValue();
    Serial.printf("/userTimeAndSensorValue triggered\n");
  });

  server.on("/userTime", HTTP_GET, []() {
    sendUserTime();
    Serial.printf("/userTime triggered\n");
  });

  server.on("/userTime", HTTP_POST, []() {
    String hours = server.arg("hours");
    String minutes = server.arg("minutes");
    String manualTime = server.arg("manualTime");

    setManualTime(manualTime.toInt());

    setUserTime(hours.toInt(), minutes.toInt());
    sendUserTime();
    Serial.printf("/userTime (POST) triggered with value: hours:%s", String(receiveHour()).c_str());
    Serial.printf(", minutes:%s", String(receiveMinute()).c_str());
    Serial.printf(", manualTime:%s\n", String(manualTime).c_str());
  });

  server.on("/brightnessLow", HTTP_GET, []() {
    sendBrightnessLow();
    Serial.printf("/brightnessLow triggered\n");
  });

  server.on("/brightnessLow", HTTP_POST, []() {
    String value = server.arg("value");
    setBrightnessLow(value.toInt());
    sendBrightnessLow();
    Serial.printf("/brightnessLow (POST) triggered with value: brightnessLow:%s\n", value.c_str());
  });

  server.on("/brightnessHigh", HTTP_GET, []() {
    sendBrightnessHigh();
    Serial.printf("/brightnessHigh triggered\n");
  });

  server.on("/brightnessHigh", HTTP_POST, []() {
    String value = server.arg("value");
    setBrightnessHigh(value.toInt());
    sendBrightnessHigh();
    Serial.printf("/brightnessHigh (POST) triggered with value: brightnessHigh:%s\n", value.c_str());
  });

  server.on("/sensorThreshold", HTTP_GET, []() {
    sendSensorThreshold();
    Serial.printf("/sensorThreshold triggered\n");
  });

  server.on("/sensorThreshold", HTTP_POST, []() {
    String value = server.arg("value");
    setSensorThreshold(value.toInt());
    sendSensorThreshold();
    Serial.printf("/sensorThreshold (POST) triggered with value: sensorThreshold:%s\n", value.c_str());
  });
 
  server.on("/resetWifi", HTTP_GET, [](){
    resetAllLEDs();
//    wifiManager.resetSettings();


    server.send(200, "text/json");
    Serial.printf("/resetWifi triggered\n");
    ESP.restart();
  });

  Serial.println("Interface URL's defined");

  //Static Server Files
  Serial.println("Configure static server files");
  server.serveStatic("/", SPIFFS, "/index.htm", "max-age=31536000");
  server.serveStatic("/index.htm", SPIFFS, "/index.htm", "max-age=31536000");
  server.serveStatic("/fonts", SPIFFS, "/fonts", "max-age=31536000");
  server.serveStatic("/js", SPIFFS, "/js", "max-age=31536000");
  server.serveStatic("/css", SPIFFS, "/css", "max-age=31536000");
  server.serveStatic("/images", SPIFFS, "/images", "max-age=31536000");
  Serial.println("Static server files configured");

  //Start server
  Serial.println("Start Server");
  server.begin();

  Serial.println("HTTP server started");
}

void loopWebserver(void)
{
  server.handleClient();
}

void initBrightnessSettings() {

  Serial.println("Checking EEPROM Settings");

  uint8_t temp;
  EEPROM.get(EEPROM_BRIGHTNESS_LOW, temp);
  if(temp <= 0) { //Check, if the value is already set in EEPROM
    setBrightnessLow(userBrightnessLow); //If not, initialize the value in EEPROM from userBrightnessLow
    Serial.print("userBrightnessLow initialized to: ");
    Serial.println(userBrightnessLow);
  }

  EEPROM.get(EEPROM_BRIGHTNESS_HIGH, temp);
  if(temp <= 0) { //Check, if the value is already set in EEPROM
    setBrightnessHigh(userBrightnessHigh); //If not, initialize the value in EEPROM from userBrightnessHigh
    Serial.print("userBrightnessHigh initialized to: ");
    Serial.println(userBrightnessHigh);
  }

  uint16_t tempThreshold;
  EEPROM.get(EEPROM_SENSOR_THRESHOLD, tempThreshold);
  if(tempThreshold <= 0) { //Check, if the value is already set in EEPROM
    setSensorThreshold(sensorThreshold); //If not, initialize the value in EEPROM from sensorThreshold
    Serial.print("sensorThreshold initialized to: ");
    Serial.println(sensorThreshold);
  }

}

void loadSettings()
{
  Serial.println("Loading Webserver Settings");

  EEPROM.get(EEPROM_BRIGHTNESS_LOW, userBrightnessLow);
  EEPROM.get(EEPROM_BRIGHTNESS_HIGH, userBrightnessHigh);
  EEPROM.get(EEPROM_SENSOR_THRESHOLD, sensorThreshold);

  EEPROM.get(EEPROM_CURRENT_TIME_SERVER_INDEX, currentTimeServerIndex);
  if (currentTimeServerIndex < 0)
    currentTimeServerIndex = 0;
  else if (currentTimeServerIndex >= timeServerCount)
    currentTimeServerIndex = timeServerCount - 1;

  byte r;
  byte g;
  byte b;
  
  EEPROM.get(EEPROM_RED_COLOR_VALUE, r);
  EEPROM.get(EEPROM_GREEN_COLOR_VALUE, g);
  EEPROM.get(EEPROM_BLUE_COLOR_VALUE, b);

  if (r != 0 || g != 0 || b != 0)
  {
    setColor(r, g, b);
  }

  EEPROM.get(EEPROM_LED_POWER_STATE, LED_POWERED);
  EEPROM.get(EEPROM_MANUEL_TIME_STATE, manualTime);
  Serial.println("Webserver Settings loaded");
}

void sendAll()
{
  String json = "{";

  json += "\"power\":" + String(LED_POWERED) + ",";
  json += "\"manualTime\":" + String(manualTime) + ",";
  json += "\"brightnessLow\":" + String(userBrightnessLow) + ",";
  json += "\"brightnessHigh\":" + String(userBrightnessHigh) + ",";
  json += "\"sensorThreshold\":" + String(sensorThreshold) + ",";


  json += "\"currentTimeServer\":{";
  json += "\"index\":" + String(currentTimeServerIndex);
  json += ",\"URL\":\"" + timeServers[currentTimeServerIndex].timeServerURL + "\"";
  json += ",\"name\":\"" + timeServers[currentTimeServerIndex].name + "\"}";

  json += ",\"solidColor\":{";
  //  json += "\"r\":" + String(readColor().r);
  //  json += ",\"g\":" + String(readColor().g);
  //  json += ",\"b\":" + String(readColor().b);

  json += "\"r\":" + String(RED_LED);
  json += ",\"g\":" + String(GREEN_LED);
  json += ",\"b\":" + String(BLUE_LED);
  json += "}";

  json += ",\"userTime\":{";
  json += "\"hours\":" + String(receiveHour());
  json += ",\"minutes\":" + String(receiveMinute());
  json += "}";

  json += ",\"timeServers\":[";
  for (uint8_t i = 0; i < timeServerCount; i++)
  {
    json += "\"" + timeServers[i].name + "\"";
    if (i < timeServerCount - 1)
      json += ",";
  }
  json += "]";

  json += "}";

  server.send(200, "text/json", json);
  json = String();
}

void sendUserTimeAndSensorValue()
{

  server.sendHeader("Access-Control-Allow-Origin", "*");

  String json = "{";
  json += "\"hours\":" + String(receiveHour());
  json += ",\"minutes\":" + String(receiveMinute());
  json += ",\"manualTime\":" + String(manualTime);
  json += ",\"sensorValue\":" + String((uint16_t) receiveSensorBrightness());
  json += "}";
  server.send(200, "text/json", json);
  json = String();

}

void sendUserTime()
{
  server.sendHeader("Access-Control-Allow-Origin", "*");

  String json = "{";
  json += "\"hours\":" + String(receiveHour());
  json += ",\"minutes\":" + String(receiveMinute());
  json += ",\"manualTime\":" + String(manualTime);
  json += "}";
  server.send(200, "text/json", json);
  json = String();
}

void sendManualTime()
{
  String json = String(manualTime);
  server.send(200, "text/json", json);
  json = String();
}

void sendPower()
{
  String json = String(LED_POWERED);
  server.send(200, "text/json", json);
  json = String();
}

void sendTimeServers()
{
  String json = "{";
  json += "\"index\":" + String(currentTimeServerIndex);
  json += ",\"name\":\"" + timeServers[currentTimeServerIndex].name + "\"";
  json += "}";
  server.send(200, "text/json", json);
  json = String();
}

void sendBrightnessLow()
{
  String json = String(userBrightnessLow);
  server.send(200, "text/json", json);
  json = String();
}

void sendBrightnessHigh()
{
  String json = String(userBrightnessHigh);
  server.send(200, "text/json", json);
  json = String();
}

void sendSensorThreshold()
{
  String json = String(sensorThreshold);
  server.send(200, "text/json", json);
  json = String();
}

void sendSolidColor()
{
  String json = "{";
  json += "\"r\":" + String(RED_LED);
  json += ",\"g\":" + String(GREEN_LED);
  json += ",\"b\":" + String(BLUE_LED);
  json += "}";
  server.send(200, "text/json", json);
  json = String();
}

void setManualTime(uint8_t value)
{
  manualTime = value == 0 ? 0 : 1;

  EEPROM.put(EEPROM_MANUEL_TIME_STATE, manualTime);
  EEPROM.commit();

  controlNTPSyncing();
}

void setPower(uint8_t value)
{
  LED_POWERED = value == 0 ? 0 : 1;

  EEPROM.put(EEPROM_LED_POWER_STATE, LED_POWERED);
  EEPROM.commit();

  calculateLEDsToSet(receiveMinute(), receiveHour());
}

void setSolidColor(uint8_t r, uint8_t g, uint8_t b)
{
  setColor(r, g, b);

  EEPROM.put(EEPROM_RED_COLOR_VALUE, r);
  EEPROM.put(EEPROM_GREEN_COLOR_VALUE, g);
  EEPROM.put(EEPROM_BLUE_COLOR_VALUE, b);
  EEPROM.commit();

  calculateLEDsToSet(receiveMinute(), receiveHour());
}

void setUserTime(uint8_t hours, uint8_t minutes)
{
  setTime(hours, minutes, 0, 1, 1, 2017); //We don't care about the date

  calculateLEDsToSet(receiveMinute(), receiveHour());
}

// increase or decrease the current timeServer number, and wrap around at the ends
void adjustTimeServer(bool up)
{
  if (up)
    currentTimeServerIndex++;
  else
    currentTimeServerIndex--;

  // wrap around at the ends
  if (currentTimeServerIndex < 0)
    currentTimeServerIndex = timeServerCount - 1;
  if (currentTimeServerIndex >= timeServerCount)
    currentTimeServerIndex = 0;

  EEPROM.put(EEPROM_CURRENT_TIME_SERVER_INDEX, currentTimeServerIndex);
  EEPROM.commit();

  calculateLEDsToSet(receiveMinute(), receiveHour());
}

void setTimeServer(int value)
{
  // don't wrap around at the ends
  if (value < 0)
    value = 0;
  else if (value >= timeServerCount)
    value = timeServerCount - 1;

  currentTimeServerIndex = value;

  EEPROM.put(EEPROM_CURRENT_TIME_SERVER_INDEX, currentTimeServerIndex);
  EEPROM.commit();

  reloadNtpSyncing();
  calculateLEDsToSet(receiveMinute(), receiveHour());
}

void setBrightnessLow(uint8_t receivedBrightnessLow) {
  userBrightnessLow = receivedBrightnessLow;
  EEPROM.put(EEPROM_BRIGHTNESS_LOW, receivedBrightnessLow);
  EEPROM.commit();
  
  updateSensorBrightnessColorValues();
  calculateLEDsToSet(receiveMinute(), receiveHour());
}

void setBrightnessHigh(uint8_t receivedBrightnessHigh) {
  userBrightnessHigh = receivedBrightnessHigh;
  EEPROM.put(EEPROM_BRIGHTNESS_HIGH, receivedBrightnessHigh);
  EEPROM.commit();
  
  updateSensorBrightnessColorValues();
  calculateLEDsToSet(receiveMinute(), receiveHour());
}

void setSensorThreshold(uint16_t receivedThreshold) {
  sensorThreshold = receivedThreshold;
  EEPROM.put(EEPROM_SENSOR_THRESHOLD, receivedThreshold);
  EEPROM.commit();
  
  updateSensorBrightnessColorValues();
  calculateLEDsToSet(receiveMinute(), receiveHour());
}

boolean ntpSyncIsActive(void)
{
  Serial.print("Check, if NTP sync is active or not. Manual Time: ");
  Serial.println(manualTime);

  if (manualTime == 0)
    return true;
  else
    return false;
}

String getCurrentTimeserverURL(void)
{
  return timeServers[currentTimeServerIndex].timeServerURL;
}
