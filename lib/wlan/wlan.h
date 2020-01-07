/*
  WLAN Config & set-up
*/

#ifndef WLAN_H_INCLUDED
#define WLAN_H_INCLUDED

#include <WiFiClient.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <EEPROM.h>
#include <ESP8266WebServer.h>

enum ConnectedStates { UNCONNECTED, CONNECTED, MANAGED };
extern ConnectedStates state;

/*

    How to use the wlan library

    1. include header in main.coo
        #include "wlan.h"

    2. Add the following "early" in setup()

        if(setup_wlan() == true){
            state = CONNECTED;
        #ifdef INCLUDE_OTA_PUSH
            if(enable_ota_push){
                start_ota();
            }
        #endif
            mqtt_setup();
        }else{
            state = UNCONNECTED;
            settingMode = true;
        }

    3. Add the following in loop()

        // Process network-related requests
        loop_wlan();

        // Some loop functions should only run when in CONNECTED mode
        if(state == CONNECTED){

            // things to do if we're connected
            
        }


 */

class ScanNetworks{
    private:
        int networkCount = 0;
    public:
        int doScan();
        String getOptionList();
        String getArray();
};

void setupMode();

extern char * device_id;

boolean restoreConfig();
boolean checkWlanConnection();
void setupMode();
void startWebServer();
String makePage(String, String);
String urlDecode(String);
String deviceID();
boolean wlanConnect(const char *,const char *);
boolean restoreConfig();
boolean setup_wlan();
void loop_wlan();

extern WiFiClient wifiClient;
extern boolean settingMode;
extern char * device_id;

typedef enum WlanState 
{
    WLAN_OFF = 0, WLAN_STA_CONNECTING = 3, WLAN_STA_CONNECTED = 4, WLAN_AP_MODE = 2, WLAN_STARTUP = 1
} WlanState_t;

#endif





