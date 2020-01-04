float_t oldSensorBrightnessValue = 0;
unsigned long oldMillis = 0;

void setupBrightness() {
  //Setup the LIGHTSENSORPIN to INPUT-Mode as this is the brightness sensor!
  pinMode(LIGHTSENSORPIN, INPUT);
  updateSensorBrightnessColorValues();
}

//We're gonna read 20 sensor values and calculate the meridian to avoid flicker values
float_t receiveSensorBrightness() {
  float_t sensorValuesSummedUp = 0;
  float_t result = 0;

  for(int i=0;i<20;i++) {
    sensorValuesSummedUp += analogRead(LIGHTSENSORPIN); //Read light level
  }
  return sensorValuesSummedUp/20;
}

boolean hasSensorBrightnessChanged() {
 unsigned long newMillis = millis();
 boolean sensorBrightnessHasChanged;

  //Only read the sensor values every BRIGHTNESS_SENSOR_DELAY_PER_SECOND seconds
  if (newMillis > (oldMillis + BRIGHTNESS_SENSOR_DELAY_PER_SECOND) || newMillis < 2000) {
    oldMillis = newMillis;
    float_t newSensorBrightnessValue = receiveSensorBrightness();

    sensorBrightnessHasChanged = newSensorBrightnessValue >= oldSensorBrightnessValue+5 || newSensorBrightnessValue <= oldSensorBrightnessValue-5;
    if(sensorBrightnessHasChanged) {
        oldSensorBrightnessValue = newSensorBrightnessValue;
    }
  }

  return sensorBrightnessHasChanged;
}

void updateSensorBrightnessColorValues() {
  Serial.println("Starting to update Sensor Brightness Color Values");
  float_t sensorBrightness = receiveSensorBrightness();

  if((uint16_t) sensorBrightness < sensorThreshold - 10) {
    //Low Light
    //Compute the brightness based on the sensor value

    RED_LED_BRIGHTNESS = (uint8_t) ((float_t) (RED_LED * userBrightnessLow) / 100);
    GREEN_LED_BRIGHTNESS = (uint8_t) ((float_t) (GREEN_LED * userBrightnessLow) / 100);
    BLUE_LED_BRIGHTNESS = (uint8_t) ((float_t) (BLUE_LED * userBrightnessLow) / 100);
  } else if((uint16_t) sensorBrightness > sensorThreshold + 10) {
    //High Light
    //Compute the brightness based on the sensor value

    RED_LED_BRIGHTNESS = (uint8_t) ((float_t) (RED_LED * userBrightnessHigh) / 100);
    GREEN_LED_BRIGHTNESS = (uint8_t) ((float_t) (GREEN_LED * userBrightnessHigh) / 100);
    BLUE_LED_BRIGHTNESS = (uint8_t) ((float_t) (BLUE_LED * userBrightnessHigh) / 100);
  }
}