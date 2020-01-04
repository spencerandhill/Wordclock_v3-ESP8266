int oldHour = 0;
int oldMinute = 1;   //this is a different default-value than defined in main. => first hasTimeChanged-call returns true

boolean hasTimeChanged() {
  boolean timeHasChanged = false;

  int newHour = hour();
  int newMinute = minute();

  if (oldHour != newHour) {
    timeHasChanged = true;
    oldHour = newHour;
  }

  if (oldMinute != newMinute) {
    timeHasChanged = true;
    oldMinute = newMinute;
  }

  if(timeHasChanged) {
      Serial.println("Time has changed");
  }

  return timeHasChanged;
}

int receiveHour() {
  return hour();
}

int receiveMinute() {
  return minute();
}