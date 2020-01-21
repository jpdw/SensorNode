// include this for logging (production logging) and debugging (no production)
//

#include <Arduino.h>
#include "buildConfig.h"
#include "logging.h"
#include "espHardwareHelper.h"

#include <PubSubClient.h>   // need mqtt header
#include "wlan.h"           // defines ConnectedStatus enum


// Global variables
extern ConnectedStates state;
extern PubSubClient client;
extern boolean enableDebug;

//
// Mlog - log to mqtt
//
// Send log message (to mqtt) - allowing the node to log events remotely
// in a 'syslog' type way (albeit to mqtt)
// 
// Mlog requires a String contain the log message to be sent
//
// Mlog also sends the message to serial/remote console for debugging purposes
// is flags for this are enabled.
//

// Define template for logging topic. Simple sprintf substitution
const char* mqttTopicLogTemplate = "device/%s/log";
char* mqttTopicLog;


void mlog(const char* msg){

  char * msgBuffer=NULL;
#ifdef INCLUDE_DEBUG
  if((state == CONNECTED)||(enableDebug)){
#else
  if(state == CONNECTED){
#endif
    const char* msgTemplate = "[%d] %s";
    msgBuffer = (char*)malloc(strlen(msg)+14);
    sprintf(msgBuffer, msgTemplate, millis(), msg);
#ifdef INCLUDE_DEBUG
  }
  if(state == CONNECTED){
#endif
    client.publish(mqttTopicLog, msgBuffer);
  }
#ifdef INCLUDE_DEBUG
  if(enableDebug){
    debugV("%s", msgBuffer);
  }
#endif
  if(msgBuffer){
    free(msgBuffer);
  }
}

// Overloaded variant to support use of Arduino String
void mlog(String msg){
  mlog(msg.c_str());
}


void setup_logging(){

  // Build a cstring with the logging topic
  mqttTopicLog = (char*)malloc( strlen(mqttTopicLogTemplate) + 7 );
  sprintf(mqttTopicLog, mqttTopicLogTemplate, getDeviceId().c_str());
  
}
