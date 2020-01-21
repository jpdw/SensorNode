/**
 * @file espHardwareHelper.cpp
 *
 * @brief Provide hardware-specific abstraction away from the main modeules
 *
 * @author Jon Wilkins
 * Contact: jon@jpdw.org
 *
 */

#include "espHardwareHelper.h"
#include "logging.h"

String _deviceId; // Store deviceId locally

// Callback function to reset the prcoessor
void restart(String s){
    ESP.restart();
}

#ifdef INCLUDE_OTA_PUSH
    void start_ota(){
        ArduinoOTA.onStart([]() {
            mlog("OTA Push request started");
        });
        ArduinoOTA.onEnd([]() {
            mlog("OTA Push request finished");
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
        mlog("OTA Push support ready at " + WiFi.localIP().toString());
    }
#endif

/** Generate a unique ID for this device
*
* This is usually device-specific so is abstracted to this module
* Global variable DeviceID is set to point to a buffer, created here,
* that contains the unique ID
*
*/
void generateDeviceId(){
    // Generate unique id based on MAC (e.g. A0B1C2)
    _deviceId = ESP.getChipId();
}

String getDeviceId(){
    return _deviceId;
}