ESP8266WebServer server(80);

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

  //WiFiManager: Handles automatic WiFi Connection if available or creates an AP, if not
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  wifiManager.setAPCallback(configModeCallback);
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  //fetches ssid and pass from eeprom and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
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

  Serial.println("EEPROM Begin");
  EEPROM.begin(512);
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

  server.on("/brightness", HTTP_GET, []() {
    sendBrightness();
    Serial.printf("/brightness triggered\n");
  });

  server.on("/brightness", HTTP_POST, []() {
    String value = server.arg("value");
    setBrightness(value.toInt());
    sendBrightness();
    Serial.printf("/brightness (POST) triggered with value: brightness:%s\n", value.c_str());
  });

  server.on("/brightnessUp", HTTP_POST, []() {
    adjustBrightness(true);
    sendBrightness();
    Serial.printf("/brightnessUp triggered\n");
  });

  server.on("/brightnessDown", HTTP_POST, []() {
    adjustBrightness(false);
    sendBrightness();
    Serial.printf("/brightnessDown triggered\n");
  });

  Serial.println("Interface URL's defined");

  //Static Server Files
  Serial.println("Configure static server files");
  server.serveStatic("/", SPIFFS, "/index.htm");
  server.serveStatic("/index.htm", SPIFFS, "/index.htm");
  server.serveStatic("/fonts", SPIFFS, "/fonts", "max-age=86400");
  server.serveStatic("/js", SPIFFS, "/js");
  server.serveStatic("/css", SPIFFS, "/css", "max-age=86400");
  server.serveStatic("/images", SPIFFS, "/images", "max-age=86400");
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

void loadSettings()
{
  Serial.println("Loading Webserver Settings");

  LED_BRIGHTNESS = EEPROM.read(0);

  currentTimeServerIndex = EEPROM.read(1);
  if (currentTimeServerIndex < 0)
    currentTimeServerIndex = 0;
  else if (currentTimeServerIndex >= timeServerCount)
    currentTimeServerIndex = timeServerCount - 1;

  byte r = EEPROM.read(2);
  byte g = EEPROM.read(3);
  byte b = EEPROM.read(4);

  if (r != 0 || g != 0 || b != 0)
  {
    setColor(r, g, b);
  }

  LED_POWERED = EEPROM.read(5);
  manualTime = EEPROM.read(6);
  Serial.println("Webserver Settings loaded");
}

void sendAll()
{
  String json = "{";

  json += "\"power\":" + String(LED_POWERED) + ",";
  json += "\"manualTime\":" + String(manualTime) + ",";
  json += "\"brightness\":" + String(LED_BRIGHTNESS) + ",";

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

void sendBrightness()
{
  String json = String(LED_BRIGHTNESS);
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

  EEPROM.write(6, manualTime);
  EEPROM.commit();

  controlNTPSyncing();
}

void setPower(uint8_t value)
{
  LED_POWERED = value == 0 ? 0 : 1;

  EEPROM.write(5, LED_POWERED);
  EEPROM.commit();

  calculateLEDsToSet(receiveMinute(), receiveHour());
}

void setSolidColor(uint8_t r, uint8_t g, uint8_t b)
{
  setColor(r, g, b);

  EEPROM.write(2, r);
  EEPROM.write(3, g);
  EEPROM.write(4, b);
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

  EEPROM.write(1, currentTimeServerIndex);
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

  EEPROM.write(1, currentTimeServerIndex);
  EEPROM.commit();

  reloadNtpSyncing();
  calculateLEDsToSet(receiveMinute(), receiveHour());
}

// adjust the brightness, and wrap around at the ends
void adjustBrightness(bool up)
{
  if (up)
    brightnessIndex++;
  else
    brightnessIndex--;

  // wrap around at the ends
  if (brightnessIndex < 0)
    brightnessIndex = brightnessCount - 1;
  else if (brightnessIndex >= brightnessCount)
    brightnessIndex = 0;

  LED_BRIGHTNESS = brightnessMap[brightnessIndex];

  //  FastLED.setBrightness(LED_BRIGHTNESS);
  //strip.SetBrightness(LED_BRIGHTNESS);

  Serial.println("Brightness adjusted: ");
  Serial.println(String(LED_BRIGHTNESS));

  setBrightness(LED_BRIGHTNESS);
}

void setBrightness(int value)
{
  // don't wrap around at the ends
  if (value > 255)
    value = 255;
  else if (value < 0)
    value = 0;

  LED_BRIGHTNESS = value;

  //  FastLED.setBrightness(LED_BRIGHTNESS);
  //strip.SetBrightness(LED_BRIGHTNESS);

  EEPROM.write(0, LED_BRIGHTNESS);
  EEPROM.commit();

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
