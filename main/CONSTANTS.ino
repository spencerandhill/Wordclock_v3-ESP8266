//#############SECTION LEDS#############
#define NUM_LEDS 114  //NUMBER OF LEDS

//#define CHIPSET     NEOPIXEL
#define CHIPSET     NeoEsp8266Dma800KbpsMethod
#define LIGHTSENSORPIN A0 //Ambient light sensor reading
#define LED_PIN     14 //This is DataPin 5, aka GPIO14
//LED_PIN is currently not in use, as the default PIN of the LED Library "NeoPixelBrightnessBus" is used
//The Library does use the DMA-Method (Hardware Method to output LED-signals) and therefore it uses RDX0/GPIO3 for LED output

#define BRIGHTNESS_SENSOR_DELAY_PER_SECOND 150

//#############SECTION EEPROM STORAGE ADDRESSES#############
#define EEPROM_BRIGHTNESS_LOW 0
#define EEPROM_BRIGHTNESS_HIGH 1
#define EEPROM_SENSOR_THRESHOLD 2
#define EEPROM_CURRENT_TIME_SERVER_INDEX 3
#define EEPROM_RED_COLOR_VALUE 4
#define EEPROM_GREEN_COLOR_VALUE 5
#define EEPROM_BLUE_COLOR_VALUE 6
#define EEPROM_LED_POWER_STATE 7
#define EEPROM_MANUEL_TIME_STATE 8

//#############SECTION LED WORDS AND DOTS#############
//This is NOT the patching of the LED-output itself, these are just
//constants to differentiate in code, which words should be activated
//GENERAL WORDS
#define LED_WORD_ES 1
#define LED_WORD_IST 2
#define LED_WORD_UHR 3

//MINUTE WORDS
#define LED_WORD_FUENF 4
#define LED_WORD_ZEHN 5
#define LED_WORD_ZWANZIG 6
#define LED_WORD_VIERTEL 7
#define LED_WORD_NACH 8
#define LED_WORD_VOR 9
#define LED_WORD_HALB 10

//HOUR WORDS
#define LED_WORD_ZWOELF_S 11
#define LED_WORD_EINS_S 12
#define LED_WORD_ZWEI_S 13
#define LED_WORD_DREI_S 14
#define LED_WORD_VIER_S 15
#define LED_WORD_FUENF_S 16
#define LED_WORD_SECHS_S 17
#define LED_WORD_SIEBEN_S 18
#define LED_WORD_ACHT_S 19
#define LED_WORD_NEUN_S 20
#define LED_WORD_ZEHN_S 21
#define LED_WORD_ELF_S 22

//MINUTE_DOTS
#define LED_DOT_EINS_M 23
#define LED_DOT_ZWEI_M 24
#define LED_DOT_DREI_M 25
#define LED_DOT_VIER_M 26
