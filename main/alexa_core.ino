//Bezeichnung des Gerätes in der Alexa App
#define ID_PLUG "Uhr"

fauxmoESP fauxmo;

void setupAlexa()
{
    fauxmo.createServer(true); // not needed, this is the default value
    fauxmo.setPort(80); // This is required for gen3 devices
    fauxmo.enable(true);
    fauxmo.addDevice(ID_PLUG);
    
    fauxmo.onSetState([](unsigned char device_id, const char * device_name, bool state, unsigned char value) {
        if (strcmp(device_name, ID_PLUG)==0) {
            Serial.printf("[MAIN] Device #%d (%s) state: %s value: %d\n", device_id, device_name, state ? "ON" : "OFF", value);
            setPower(state);
            sendPower();
        }  
    });
}

void loopAlexa()
{
    fauxmo.handle();
}
