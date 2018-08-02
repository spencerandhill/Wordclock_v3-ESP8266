void processSyncEvent (NTPSyncEvent_t ntpEvent) {
    if (ntpEvent) {
        Serial.print ("Time Sync error: ");
        Serial.print ("Uptime: ");
        Serial.print (NTP.getUptimeString ());Serial.print (" since ");
        Serial.print("DateTime: "); Serial.println (NTP.getTimeDateString (NTP.getFirstSync ()).c_str ());
        if (ntpEvent == noResponse)
            Serial.println ("NTP server not reachable");
        else if (ntpEvent == invalidAddress)
            Serial.println ("Invalid NTP server address");
    } else {
        Serial.print ("Got NTP time: ");
        Serial.println (NTP.getTimeDateString (NTP.getLastNTPSync ()));
    }
}

boolean syncEventTriggered = false; // True if a time even has been triggered
NTPSyncEvent_t ntpEvent; // Last triggered event

void setupNTP(void)
{  
    Serial.println("Configuring NTP");
    NTP.onNTPSyncEvent ([](NTPSyncEvent_t event) {
        ntpEvent = event;
        syncEventTriggered = true;
        Serial.println("New NTP Time achieved");
    });

    controlNTPSyncing();
}

void loopNTP(void) {
    if (syncEventTriggered) {
        processSyncEvent (ntpEvent);
        syncEventTriggered = false;
    }
}

void controlNTPSyncing(void) {
    if(ntpSyncIsActive()) {
        startNtpSyncing();
    }
    else {
        stopNtpSyncing();
    }
}

//This is to reload the ntp syncing in case the ntp-server has changed
void reloadNtpSyncing(void) {
    if(ntpSyncIsActive()) {
        stopNtpSyncing();
        startNtpSyncing();
    }
}

void startNtpSyncing(void) {
    String currentTimeServer = getCurrentTimeserverURL();
    Serial.print("current TimeServer: ");
    Serial.println(currentTimeServer);    

    NTP.begin(currentTimeServer, 1, true);
  	NTP.setInterval(10, 1800); //This will change the interval automatically to 30 minutes after first sync is achieved

    Serial.println("NTP syncing started");
}

void stopNtpSyncing(void) {
    NTP.stop();

    Serial.println("NTP syncing stopped");

    Serial.println("Showing Manual Time");
    calculateLEDsToSet(receiveMinute(), receiveHour());
}
