
#include "espHardwareHelper.h"



// Callback function to reset the prcoessor
void restart(String s){
    ESP.restart();
}

#ifdef INCLUDE_OTA_PUSH
    void start_ota(){
        ArduinoOTA.onStart([]() {
            Serial.println("Start");
        });
        ArduinoOTA.onEnd([]() {
            Serial.println("\nEnd");
        });
        ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
            Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
        });
        ArduinoOTA.onError([](ota_error_t error) {
            Serial.printf("Error[%u]: ", error);
            if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
            else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
            else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
            else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
            else if (error == OTA_END_ERROR) Serial.println("End Failed");
        });
        ArduinoOTA.begin();
        Serial.print("\nOTA push ready at ");
        Serial.println(WiFi.localIP());
    }
#endif


