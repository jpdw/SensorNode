#ifndef ESPHARDWAREHELPER_H_INCLUDED
    #define ESPHARDWAREHELPER_H_INCLUDED
    #include "buildConfig.h"    
    #include <ESP8266mDNS.h>
    #include <WiFiUdp.h>
    #include <ArduinoOTA.h>
#endif

// Function prototypes
void restart(String);
#ifdef INCLUDE_OTA_PUSH
void start_ota();
#endif

