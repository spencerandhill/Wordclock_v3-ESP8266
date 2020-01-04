//RGB Values for the Color of the Clock
uint8_t RED_LED = 255;
uint8_t GREEN_LED = 255;
uint8_t BLUE_LED = 255;

//RGB Values with calculated Brightness for the Color of the Clock (see sensor_brightness_core.ino)
uint8_t RED_LED_BRIGHTNESS = 255;
uint8_t GREEN_LED_BRIGHTNESS = 255;
uint8_t BLUE_LED_BRIGHTNESS = 255;

uint8_t userBrightnessLow = 10; //lower percentage of brightness
uint8_t userBrightnessHigh = 100; //upper percentage of brightness
uint16_t sensorThreshold = 40; //the threshold value where the low/high mode is flipped

uint8_t LED_POWERED = 1;

NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(NUM_LEDS);

RgbColor readColorWithBrightness() {
  //Compute RGB Color from RED_LED_BRIGHTNESS, GREEN_LED_BRIGHTNESS, BLUE_LED_BRIGHTNESS
  //Values and return as RGB-Value
  return RgbColor(RED_LED_BRIGHTNESS, GREEN_LED_BRIGHTNESS, BLUE_LED_BRIGHTNESS);
}

RgbColor readColor() {
  //Compute RGB Color from RED_LED, GREEN_LED, BLUE_LED
  //Values and return as RGB-Value
  return RgbColor(RED_LED, GREEN_LED, BLUE_LED);
}

void setColor(uint8_t r, uint8_t g, uint8_t b) {
  RED_LED = r;
  GREEN_LED = g;
  BLUE_LED = b;

  updateSensorBrightnessColorValues();
}

void setupLED() {
  strip.Begin();
  strip.Show();
}

void loopLED() {
  boolean brightnessHasChanged = hasSensorBrightnessChanged();
  if(brightnessHasChanged) {
    updateSensorBrightnessColorValues();
  }

  if (hasTimeChanged() || brightnessHasChanged) {
    calculateLEDsToSet(receiveMinute(), receiveHour());
  }

  //strip.setBrightness(USER_BRIGHTNESS);
  strip.Show();  
}

void calculateLEDsToSet(int minutes, int hours) {
  resetAllLEDs();

  if(LED_POWERED == 0) {
    return;
  }

  setLEDs(LED_WORD_ES);
  setLEDs(LED_WORD_IST);

  int calculatedHour = calculateMinuteLEDsAndGetHour(minutes, hours);
  calculateHourLEDs(calculatedHour);
  calculateDotLEDS(minutes);

  if (minutes < 5)
  { //if we have less than 5 minutes (no special word for the minutes) show "Uhr"
    setLEDs(LED_WORD_UHR);
  }
}

int calculateMinuteLEDsAndGetHour(int minutes, int hours)
{
  if (minutes < 5)
  {
    return hours;
  }

  if (minutes < 10)
  {
    setLEDs(LED_WORD_FUENF);
    setLEDs(LED_WORD_NACH);
    return hours;
  }

  if (minutes < 15)
  {
    setLEDs(LED_WORD_ZEHN);
    setLEDs(LED_WORD_NACH);
    return hours;
  }

  if (minutes < 20)
  {
    setLEDs(LED_WORD_VIERTEL);
    setLEDs(LED_WORD_NACH);
    return hours;
  }

  if (minutes < 25)
  {
    setLEDs(LED_WORD_ZWANZIG);
    setLEDs(LED_WORD_NACH);
    return hours;
  }

  if (minutes < 30)
  {
    setLEDs(LED_WORD_FUENF);
    setLEDs(LED_WORD_VOR);
    setLEDs(LED_WORD_HALB);
    return hours + 1;
  }

  if (minutes < 35)
  {
    setLEDs(LED_WORD_HALB);
    return hours + 1;
  }

  if (minutes < 40)
  {
    setLEDs(LED_WORD_FUENF);
    setLEDs(LED_WORD_NACH);
    setLEDs(LED_WORD_HALB);
    return hours + 1;
  }

  if (minutes < 45)
  {
    setLEDs(LED_WORD_ZWANZIG);
    setLEDs(LED_WORD_VOR);
    return hours + 1;
  }

  if (minutes < 50)
  {
    setLEDs(LED_WORD_VIERTEL);
    setLEDs(LED_WORD_VOR);
    return hours + 1;
  }

  if (minutes < 55)
  {
    setLEDs(LED_WORD_ZEHN);
    setLEDs(LED_WORD_VOR);
    return hours + 1;
  }
  else
  {
    setLEDs(LED_WORD_FUENF);
    setLEDs(LED_WORD_VOR);
    return hours + 1;
  }
}

void calculateHourLEDs(int calculatedHour)
{
  if (calculatedHour == 0 || calculatedHour == 12 || calculatedHour == 24)
  {
    setLEDs(LED_WORD_ZWOELF_S);
    return;
  }
  if (calculatedHour == 1 || calculatedHour == 13)
  {
    setLEDs(LED_WORD_EINS_S);
    return;
  }
  if (calculatedHour == 2 || calculatedHour == 14)
  {
    setLEDs(LED_WORD_ZWEI_S);
    return;
  }
  if (calculatedHour == 3 || calculatedHour == 15)
  {
    setLEDs(LED_WORD_DREI_S);
    return;
  }
  if (calculatedHour == 4 || calculatedHour == 16)
  {
    setLEDs(LED_WORD_VIER_S);
    return;
  }
  if (calculatedHour == 5 || calculatedHour == 17)
  {
    setLEDs(LED_WORD_FUENF_S);
    return;
  }
  if (calculatedHour == 6 || calculatedHour == 18)
  {
    setLEDs(LED_WORD_SECHS_S);
    return;
  }
  if (calculatedHour == 7 || calculatedHour == 19)
  {
    setLEDs(LED_WORD_SIEBEN_S);
    return;
  }
  if (calculatedHour == 8 || calculatedHour == 20)
  {
    setLEDs(LED_WORD_ACHT_S);
    return;
  }
  if (calculatedHour == 9 || calculatedHour == 21)
  {
    setLEDs(LED_WORD_NEUN_S);
    return;
  }
  if (calculatedHour == 10 || calculatedHour == 22)
  {
    setLEDs(LED_WORD_ZEHN_S);
    return;
  }
  if (calculatedHour == 11 || calculatedHour == 23)
  {
    setLEDs(LED_WORD_ELF_S);
    return;
  }
}

void calculateDotLEDS(int minutes)
{
  int calculatedMinutes = minutes % 5;

  if (calculatedMinutes == 1)
  {
    setLEDs(LED_DOT_EINS_M);
    return;
  }

  if (calculatedMinutes == 2)
  {
    setLEDs(LED_DOT_ZWEI_M);
    return;
  }

  if (calculatedMinutes == 3)
  {
    setLEDs(LED_DOT_DREI_M);
    return;
  }

  if (calculatedMinutes == 4)
  {
    setLEDs(LED_DOT_VIER_M);
    return;
  }
}

//DEBUG MISC CAN BE REMOVED
void debugPrintColors()
{
  Serial.print("RED VALUE:");
  Serial.println(RED_LED);
  Serial.print("GREEN VALUE :");
  Serial.println(GREEN_LED);
  Serial.print("BLUE VALUE :");
  Serial.println(BLUE_LED);
}

void showStrip() {
  strip.Show();
}
