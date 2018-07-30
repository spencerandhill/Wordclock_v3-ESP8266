void setupNTP(void)
{  
    controlNTPSyncing();
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
	NTP.setInterval(63);                //This will be changed automatically to 30 minutes after first sync is achieved

    Serial.println("NTP syncing started");
}

void stopNtpSyncing(void) {
    NTP.stop();

    Serial.println("NTP syncing stopped");
}