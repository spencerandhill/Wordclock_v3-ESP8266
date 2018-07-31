int oldHour = 0;
int oldMinute = 1;   //this is a different default-value than defined in main. => first hasTimeChanged-call returns true

boolean hasTimeChanged() {
  boolean result = false;

  int newHour = hour();
  int newMinute = minute();

  if (oldHour != newHour) {
    result = true;
    oldHour = newHour;
  }

  if (oldMinute != newMinute) {
    result = true;
    oldMinute = newMinute;
  }
  return result;
}

int receiveHour() {
  return hour();
}

int receiveMinute() {
  return minute();
}