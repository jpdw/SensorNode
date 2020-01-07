//
// Simple OneWire temp sensor functions
//

#include <OneWire.h>
#include <DallasTemperature.h>

#include <Arduino.h>

#include "driver_owb_ds18b20.h"
#include "scheduler.h"


extern PubSubClient client;

// Set up one wire
int oneWirePin = D4; // Wemos D4 = GPIO??
OneWire  oneWire(oneWirePin);  // on pin D4 (a 4.7K resistor is necessary)
DallasTemperature sensors(&oneWire);

// OneWire timer (to read requested input)
#define ONEWIRE_READ_INTERVAL 1000
unsigned long timer_onewiretemp_interval=ONEWIRE_READ_INTERVAL;


byte onewiretemp_addr[8];

struct owd_t {
	uint8_t address[8];		/* 8 byte address */
  uint8_t resolution;   /* resolution of device */
	//uint8_t str_id;			/* # of name in eeprom */
	float most_recent;		/* most recent reading */
	char sz_address[17];	/* a hex string representation of the address */
};

// Uninitialised pointer to owd_t structure
owd_t * onewire_devices;
uint8_t onewire_devices_count=0;

void printAddress(DeviceAddress deviceAddress)
{
    for (uint8_t i = 0; i < 8; i++){
        // zero pad the address if necessary
        if (deviceAddress[i] < 16){
            Serial.print("0");
        }
        Serial.print(deviceAddress[i], HEX);
    }
}


void onewire_info(){
    Serial.println(F("\n1Wire:"));
    for(uint8_t i=0;i<sensors.getDeviceCount();i++){
        Serial.print(i);
        Serial.print(F(" "));
        printAddress(onewire_devices[i].address);
        Serial.print(F(" "));
        Serial.print((unsigned int)onewire_devices[i].resolution);
        Serial.print(F(" "));

        Serial.println(onewire_devices[i].most_recent);
    }
}

/*
 * OneWireTemp_ScanBus
 *
 * Scan the 1-wire bus for temperature sensors and store details of all found
 * sensors in an array (of owd_t).
 *
 * The bus may be re-scanned and the sensors may respond in a different order
 * so the ordering of devices in the array must not be assumed.
 */
void OneWireTemp_ScanBus(){

	// Get the number of devices found on the bus
  uint8_t device_count = sensors.getDeviceCount();

	Serial.print("devices found = ");
	Serial.println(device_count);

	onewire_devices_count = device_count;	// store for later reference

	// Check if any devices were found and set states accordingly then exit
  if (onewire_devices_count){
		// Create an array to hold address details for each device found
	  onewire_devices=(owd_t*)malloc(device_count * sizeof(owd_t));
		if(onewire_devices==NULL){
	    // Unable to allocate memory
	    Serial.print("WARN! Memory allocation error");

		}else{
			// Iterate through every device found on the 1-wire bus
		  uint8_t i=0;
		  do {
		    // Get the 1-wire addresses of each device
		    sensors.getAddress(onewire_devices[i].address, i);
		    // Get other attributes
		    onewire_devices[i].resolution = sensors.getResolution(onewire_devices[i].address);

				// Build the string version of the address
				sprintf(onewire_devices[i].sz_address,"%02X%02X%02X%02X%02X%02X%02X%02X\0",
					onewire_devices[i].address[0], onewire_devices[i].address[1],
					onewire_devices[i].address[2], onewire_devices[i].address[3],
					onewire_devices[i].address[4], onewire_devices[i].address[5],
					onewire_devices[i].address[6], onewire_devices[i].address[7]);
				Serial.print(onewire_devices[i].sz_address);

		  } while(++i<device_count);
			/* bus scan completed */
		}

	}else{
		// No 1Wire
  	Serial.println("NO 1WIRE");
  }
}



void OneWireTemp_setup(){
#ifdef DEBUG
	Serial.println("Running - OneWireTemp_setup");
#endif

  // Initialise 1-wire temperature sensors
  sensors.begin();

	// Scan bus and store details of sensors
	OneWireTemp_ScanBus();

	// Display debug info
	onewire_info();

  // sets the dts library into non-blocking mode - calls for a conversion
  // will send a request to the device & then return -- we have to manage
  // a suitable delay and then retrieve the reading
  sensors.setWaitForConversion(false);

	// request the sensors do a first read - this is to try to avoid
	// getting an initial 85C reading later on
	sensors.requestTemperatures();
}

/*
 * Publish temperature
 */
void OneWireTemp_PublishTemperature(float value, uint8_t i){
  char topic[32];
  char msg[20];

  //printf("float = %.6f\n", value);

  sprintf(topic,"device/%06X/%s", ESP.getChipId(), onewire_devices[i].sz_address);
	sprintf(msg,"%.2f", value);
  //sprintf(msg,"%d.%02d", (int)value, (int)(value*100)%100);
  client.publish(topic,msg);

  //sprintf(topic,"device/%08X/")

}

#define PUBLISH_TEMPERATURE true

void OneWireTemp_ReadTemperatures(){
#ifdef DEBUG
	Serial.println("Reading temp request");
#endif
	for(uint8_t i=0;i<onewire_devices_count;i++){
	  float celcius = sensors.getTempCByIndex(i);
#ifdef DEBUG
		Serial.print("Index ");
		Serial.print(i);
		Serial.print(" = ");
		Serial.println(celcius);
#endif
		if (PUBLISH_TEMPERATURE){
			OneWireTemp_PublishTemperature(celcius, i);
		}
	}
}

void OneWireTemp_RequestTemperatures(){
#ifdef DEBUG
	Serial.println("Starting temp request");
#endif
  sensors.requestTemperatures();
  // Start a scheduler job to do the reading
  scheduler_setup(9, timer_onewiretemp_interval, 0, &OneWireTemp_ReadTemperatures, fSCHED0_enabled | fSCHED0_oneshot);
}
