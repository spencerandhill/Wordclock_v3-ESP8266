void showDots(RgbColor color) {
    resetAllLEDs();

    setLEDs(LED_DOT_EINS_M, color);
    setLEDs(LED_DOT_ZWEI_M, color);
    setLEDs(LED_DOT_DREI_M, color);
    setLEDs(LED_DOT_VIER_M, color);


    RgbColor currentColor = readColorWithBrightness();
    Serial.print("Current color is:");
    Serial.print(" R: ");Serial.print(currentColor.R);
    Serial.print(" G: ");Serial.print(currentColor.G);
    Serial.print(" B: ");Serial.print(currentColor.B);
    Serial.println();

    showStrip();
}

void ledAnimationShowOffline(void) {
    Serial.println("ledAnimationShowOffline");
    showDots(RgbColor(255, 0, 0));  //RED
}

void ledAnimationBooting(void) {
    Serial.println("ledAnimationBooting");
    showDots(RgbColor(0, 0, 255));  //BLUE
}

void ledAnimationShowConnecting(void) {
    Serial.println("ledAnimationShowConnecting");
    showDots(RgbColor(255, 50, 0));  //ORANGE
}

void ledAnimationShowAP(void) {
    Serial.println("ledAnimationShowAP");
    showDots(RgbColor(255, 0, 150));  //MAGENTA
}

void ledAnimationShowOnline(void) {
    Serial.println("ledAnimationShowOnline");
    showDots(RgbColor(0, 255, 0));  //GREEN
}

void ledAnimationChangeTime(void) {
    Serial.println("ledAnimationChangeTime");
}
