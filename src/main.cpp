#define INCLUDE_OTA_PUSH

#include <ESP8266WiFi.h>
#ifdef INCLUDE_OTA_PUSH
  #include <ESP8266mDNS.h>
  #include <WiFiUdp.h>
  #include <ArduinoOTA.h>
#endif

#include <EEPROM.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <ArduinoJson.h>

#include "driver_owb_ds18b20.h"
#include "scheduler.h"
#include "webserver.h"

#include "wlan.h"

#include "espHardwareHelper.h"
#include "build_info.h"

#ifdef INCLUDE_OTA_PUSH
  bool enable_ota_push = true;
#endif

#define SERIAL_BAUD 115200  /* default serial debug baud rate */
#define APP_STRING "uNode2" /* prepend hostname with this */



// Function prototypes
void publish_hello();
void cb_test_function();

// Define the WLAN
const char* mqtt_server = "10.1.1.33";

static long clock_base=0;


WiFiClient espClient;
PubSubClient client(espClient);

#define TEMPERATURE_INTERVAL 60000
//#define TEMPERATURE_INTERVAL 5000

// Configure "Hello" publish
#define HELLO_INTERVAL 120000
//#define HELLO_INTERVAL 2000


/*
 * Define a structure for each individual command that will be available.  This has been kept in the .cpp
 * file instead of the .h deliberataly, so that modules which use the console (include the .h) don't have
 * to have - and dont need - knowledge of the underlying struct
 */
#define CONSOLE_CMD_STRMAX 10
#define FIXED_COMMAND_SLOTS 10

struct command_t {
  char * command;                 // Pointer to command
  void (*callback)();             // address to call (if cmdid == 0)
  uint8_t callback_args;                      // number of arguments required
};
command_t commands[FIXED_COMMAND_SLOTS];
uint8_t defined_commands=0;            // number of defined commands

bool define_command(void (*cbfunction)(), char *command, uint8_t args=0){
  if(defined_commands>=FIXED_COMMAND_SLOTS){
    return(false);
  }
  commands[defined_commands].callback = cbfunction;
  commands[defined_commands].command = command;
  commands[defined_commands].callback_args=args;  // minimum required args
  defined_commands++;
  return(true);
}

void build_commands(){
//  define_command(&OneWireTemp_RequestTemperatures,"temp");
  }


/* ==========================================================================
 * ==========================================================================
 *
 * SCHEDULER RELATED FUNCTIONS
 *
 * ==========================================================================
 * ==========================================================================
 */

/*
 * scheduler_start
 *
 * Initialise schedule jobs (which implicitly also starts each job in the
 * scheduler -- ie. the scheduler itself "always runs" even if there are no
 * jobs defined)
 */
void scheduler_start(){
  // Publish hello regularly
  scheduler_setup(1, HELLO_INTERVAL, SCHEDULER_NEXT_FROM_NOW, &publish_hello, fSCHED0_enabled);

  // Get & send temperature readings for all attached temperature devices
  scheduler_setup(2, TEMPERATURE_INTERVAL, 0, &OneWireTemp_RequestTemperatures, fSCHED0_enabled);

  // Call the test function periodically
  //scheduler_setup(3, 10000, 0, &cb_test_function, fSCHED0_enabled);
}

/*
 * Command received (in payload)
 * Act on it!
 */
void command_received(byte* payload, unsigned int length, bool global){

  void (*callback)()=0;
  char string[50];
  memcpy(string, payload, length);  /* danger!!! */

  // Get length of first word in payload
  int wordlen = string - strchr(string, 32);

  int r = -1;
  // iterate through commands looking for the right one
  for(int i=0; i<defined_commands; i++){
    r = strncmp(commands[i].command, string, wordlen);
    if (r == 0){
      Serial.print("Matched command ");
      Serial.println(commands[i].command);

      callback = commands[i].callback;
    }
  }

  if(callback!=0){
    callback();
  }
}



/* ==========================================================================
 * ==========================================================================
 *
 * MQTT RELATED FUNCTIONS
 *
 * ==========================================================================
 * ==========================================================================
 */

/*
 * parse_mqtt_message_clock
 *
 * Parse the payload then try
 */

bool mqtt_parse_message_clock(byte * payload){

  return true;

}

/*
 * mqtt_callback
 *
 * Callback function that will be called by the mqtt object on receipt
 * of a message for a subscribed topic.  Function will either process
 * the message or make onward calls, depending on the specific topic
 *
 */
void mqtt_callback(char* topic, byte* payload, unsigned int length) {

  // Parse topic and dispatch appropriately

  // Look for some simple topics
  if(strncmp(topic,"device/global/clock",19)==0){
    mqtt_parse_message_clock(payload);
    return;
  }

  // Topic starts with "device/"?
  if(strncmp(topic,"device/",7)==0){
    //Serial.print("match so far");
    char * lch = strchr(topic,'/');
    char * pch = strrchr(topic,'/');

    bool global = false;
    if (strncmp(lch,"/global",pch-lch)==0){
      // global addressee
      global = true;
    }

    // dispatch to handle payload
    command_received(payload, length, global);
  }

}

/*
 * mqtt_reconnect()
 *
 * MQTT client has become disconnected for some reason -- attempt to
 * reconnect with the MQTT broker.
 *
 */
void mqtt_reconnect() {
 // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    char client_id[50];
    sprintf(client_id,"ESP8266 Client %08X", ESP.getChipId());

    if (client.connect(client_id)) {
      Serial.print("connected as ");
      Serial.println(client_id);
      // ... and subscribe to topics:

      char topic_private[50];
      sprintf(topic_private,"device/%06X/command", ESP.getChipId());

      client.subscribe(topic_private);

      // Publish hello to alert the network that this client has connected
      // (Note - should this differentiate between first-connection & reconnect?)
      publish_hello();

    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

/*
 * mqtt_start()
 *
 * Set up the MQTT client by connecting to the server and setting the callback
 * functions - one of these will be 'on connect' which will then set up the
 * subscriptions.
 *
 * The method shown in the examples for this particular library rely upon using
 * the reconnect method inside the main execution loop.  However, mqtt_start
 * will make the first call to the reconnect method. *
 */
void mqtt_start(){

  client.setServer(mqtt_server, 1883);
  client.setCallback(mqtt_callback);

  mqtt_reconnect();
}


/*
 *  cb_test_function
 ^
 *  Simple function to be used as a callback for testing things...
 *
 *
 */
void cb_test_function(){
  Serial.print("Time: ");
  unsigned long t = clock_base + (millis()/1000);
  Serial.println(t);
}

char deviceType [] = "uNode2";

void publish_hello() {
  char msg[95];
  char topic[32];
  //Serial.print(WiFi.localIP());
  sprintf(msg,"{'type':'%s','device':'%06X','IP':'%s','RSSI':'%d','BLD':'%s'}", \
    deviceType,ESP.getChipId(), WiFi.localIP().toString().c_str(), WiFi.RSSI(), __BI__DATEANDTIMESTAMP_STR);
    //

  sprintf(topic,"device/%06X/hello", ESP.getChipId());
  client.publish(topic,msg);
#ifdef DEBUG
  Serial.print("'Hello' playload = ");
  Serial.println(msg);
#endif
}



/* ==========================================================================
 * ==========================================================================
 *
 * MAIN EXECUTION METHODS
 *
 * ==========================================================================
 * ==========================================================================
 */

char * device_id;  // Global store for device ID
/*
 * setup
 *
 */
void setup() {

  // Setup serial-output
  Serial.begin(SERIAL_BAUD);
  EEPROM.begin(512);

  char string_buffer[50];
  sprintf(string_buffer,"%s-%08X", APP_STRING, ESP.getChipId());

   // Generate device id based on MAC (e.g. A0B1C2)
  device_id = new char[7];
  sprintf(device_id,"%06X",ESP.getChipId());
  
  build_commands();

  // New style connection using the library
  if(setup_wlan() == true){
    state = CONNECTED;
#ifdef INCLUDE_OTA_PUSH
    if(enable_ota_push){
      start_ota();
    }
#endif
    //mqtt_setup();
    mqtt_start();
  }else{
    state = UNCONNECTED;
    settingMode = true;
  }

  // Initialise 1-wire temperature sensors
  OneWireTemp_setup();

  Serial.println("setup - end");
}



void loop() {

  // Process network-related requests
  loop_wlan();

  // Some loop functions should only run when in CONNECTED mode
  if(state == CONNECTED){

#ifdef INCLUDE_OTA_PUSH
    ArduinoOTA.handle();
#endif
      // things to do if we're connected
      client.loop();
  
  }

  // Any non-blocking calls
  scheduler_loop();


}
