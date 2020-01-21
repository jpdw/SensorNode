/*
  WLAN Config & set-up
*/

#include "wlan.h"

#define SETUP_WLAN_PREFIX "SETUP-"
#define INCLUDE_OTA_PUSH

#ifdef INCLUDE_OTA_PUSH
  #include <ESP8266mDNS.h>
  #include <WiFiUdp.h>
  #include <ArduinoOTA.h>
#endif

#ifdef INCLUDE_DEBUG
#include <RemoteDebug.h>
extern RemoteDebug Debug;;
#endif


extern String getDeviceId();

boolean settingMode;

ESP8266WebServer webServer(80);

WiFiClient wifiClient;

String ssidList;

ConnectedStates state;


const IPAddress apIP(192, 168, 1, 1);
DNSServer dnsServer;


struct WlanConfig {
    String ssid = "";
    String pass = "";
    String ipaddr = "";
    //uint8_t ipaddr[4];

};



WlanConfig wlanConfig[2];
unsigned int wlanConfigCount = 0;
unsigned int eepromMapVersion = 0;


void serialLog(String s){
  String l;
  l = (String)"[" + millis() + "] " + s;
  Serial.println(l);
}

/*
 *   Basic EEPROM structure
 *
 *   0-3   : Identifies EEPROM structure
 *            0 => AA
 *            1 => 55
 *            2 => AA
 *            3 => structure version 1-255 (0) if blanked
 *   4-99  : WLAN #1
 * 100-195 : WLAN #2
 * 
 *  Where WLAN:
 *   0 - 31 => SSID
 *  32 - 95 => WPA/2 password
 *  96 - 99 => Dotted quad IP of MQTT server
 * 
 */


boolean readWlanConfigInstance(unsigned int index){
    unsigned int offset = 4 + (index * 96);

    wlanConfig[index].ssid = "";
    wlanConfig[index].pass = "";

    if (EEPROM.read(offset) != 0 && EEPROM.read(offset) != 255 ) {
        for (int i = 0; i < 32; ++i) {
            wlanConfig[index].ssid += char(EEPROM.read(offset + i));
        }

        for (int i = 32; i < 96; ++i) {
            wlanConfig[index].pass += char(EEPROM.read(offset + i));
        }
      
        unsigned int a[4];
        for (int i = 0; i < 4; ++i) {
            wlanConfig[index].ipaddr[0] = (uint8_t) char(EEPROM.read(offset + 96 + i));
            a[i] = char(EEPROM.read(offset + 96 + i));
        }
        wlanConfig[index].ipaddr = String(a[0])   +  "." +  String(a[1]) + "." + String(a[2]) + "." + String(a[3]);
    
        serialLog("Config profile " + (String)index + ": SSID = " + wlanConfig[index].ssid);
      
        return true;
    }else{
        serialLog("Config profile " + (String)index + ": Empty");
        return false;
    }

}

/*
  readWlanConfig

  Read network config from EEPROM
  - check if 'fingerprint' identifies a valid config
  - read each (of 2) wlan configs

  returns
  - true if config read
  - false if no config read
*/

boolean readWlanConfig() {
    serialLog("Reading EEPROM...");

    serialLog("Checking for EEPROM map... ");
    if(EEPROM.read(0)==0xAA && EEPROM.read(1)==0x55 && EEPROM.read(2)==0xAA){
        eepromMapVersion = EEPROM.read(3);
        serialLog("map version " + eepromMapVersion);
    }else{
        serialLog("no map or unknown version");
        return false;
    }

    if(eepromMapVersion > 0){

        for (int i = 0; i < 2; ++i) {
            if(readWlanConfigInstance(i) == true){
            /*
            Serial.print("Config read for ");
            Serial.print(i);
            Serial.print(": ssid = ");
            Serial.print(wlanConfig[i].ssid);
            */
         
            }
        }
    }else{
        return false;
    }
    return true;
}



/*
 * wlanConnect 
 * 
 * Starts the attempt to connect to a WLAN
 * 
 * ssid and password are specified in the arguements
 * 
 * Function is non-blocking, returing the result
 * of the call to WiFi.begin()
 * 
 */
boolean wlanConnect(const char * ssid,const char * pass) {
    boolean r;
    serialLog((String)"Trying to associate with AP (" + ssid + ")");
    //Serial.print("\"");
    //Serial.print(ssid);
    //Serial.println("\"");
    //Serial.print("\"");
    //Serial.print(pass);
    //Serial.println("\"");
    r = WiFi.begin(ssid, pass);
    //Serial.println("Returned from WiFi.begin");
    
    return r; //(WiFi.begin(ssid, pass)); 
     
}


/*
  checkWlanConnection

  Blocking while WLAN connection is attempted, exiting
  on success or timeout of 15 seconds

  returns true/false to indicate whether connection was successful
*/
#define WAIT_FOR_WLAN 25 /* seconds */

boolean checkWlanConnection() {

    serialLog("entered checkWlanConnection()");

    unsigned long millis_now = millis(); 
    static unsigned long millis_at_start = 0; 
    static unsigned long timeout_at;
    int count=0;

    // Get millis at this moment so we can be consistent with what is
    // the 'current' time
    millis_now = millis();

    
    // Record start of the connection attempt (we will block till its finished)
    if (millis_at_start == 0){
        millis_at_start = millis_now;
        timeout_at = millis_at_start + (WAIT_FOR_WLAN * 1000);

        serialLog("Connection attempted start at " + (String)millis_at_start);
    }

    while(1){

        // Update current millis as this is the start of the blocking loop
        millis_now = millis();


        // Have we timed out?
        if (millis_now > timeout_at){
              //Serial.println("Connection attempt duration has exceed timeout... exiting");
            // Reset millis_at_start in case we will re-call this process
            serialLog("timeout");
            millis_at_start = 0;
            return false;
        }

        // Now check status of connection
        if (WiFi.status() == WL_CONNECTED){
            serialLog("WLAN connected!");

            unsigned int a = millis_now - millis_at_start;
            String connectionStats = (String)"Association stats: Start=" + millis_at_start + "; time=" + a;
#ifdef INCLUDE_DEBUG
            debugV("%s",connectionStats.c_str());
#endif
            //Serial.print(" start     : "); Serial.println(millis_at_start);
            //Serial.print(" fimish    : "); Serial.println(millis_now);
            //Serial.print(" time      : "); Serial.println(millis_now - millis_at_start);
            //Serial.print(" iterations: "); Serial.println(count);

            return true;

        }else{
            count++;
            delay(100);
            yield();
        }
    }
}

boolean wlanAssociate(unsigned int configIndex){
  serialLog("Attempting to connect to index "+ (String)configIndex);

  wlanConnect(wlanConfig[configIndex].ssid.c_str(), wlanConfig[configIndex].pass.c_str());

  // Blocking call to wait for (a) success or (b) timeout
  if (checkWlanConnection()) {
    serialLog("wlanAssociate for index's mqtt " + wlanConfig[configIndex].ipaddr);
    settingMode = false;
    //startWebServer(); <--- this gets called in setup_wlan in a few moments anyway
    return true;
  }else{
    return false;
  }
}


void showEeprom(int addr_from, int length){
  Serial.print("Reading EEPROM from ");
  Serial.println(addr_from);
  Serial.print(" for ");
  Serial.println(length);
  Serial.println(" bytes:");
  for (int i = 0; i < 12; ++i) {
    Serial.print(addr_from+i);
    Serial.print(": ");
    Serial.print(EEPROM.read(addr_from+i));
    Serial.print(" ");
    Serial.println(char(EEPROM.read(addr_from+i)));
  }
}

void clearConfig(unsigned int index){
  unsigned int offset = 4 + (index * 96);
  for (int i = 0; i < 32; ++i) {
    EEPROM.write(offset+i,0x00);
  }
  EEPROM.commit();
}

void writeEeprom(unsigned int index, String ssid, String pass, const unsigned int mqttIp[4]){
  unsigned int offset = 4 + (index * 96);
  unsigned int i;

  // Write signature
  EEPROM.write(0,0xAA);
  EEPROM.write(1,0x55);
  EEPROM.write(2,0xAA);
  EEPROM.write(3,0x01);

  // Clear EEPROM for this record/index
  for (int i = 0; i < 100; ++i) {
    EEPROM.write(offset+i, 0);
  }

  EEPROM.commit();

  Serial.println("Writing ESSID to EEPROM...");

  for (i = 0; i < ssid.length(); i++) {
    EEPROM.write(offset + i, ssid[i]);
    //Serial.print("Addr ");
    //Serial.print(offset+i);
    //Serial.print(": ");
    //Serial.println(ssid[i]);
  }
  Serial.print(offset+i);
  
  EEPROM.write(offset+i,0);

  Serial.println("Writing Password to EEPROM...");
  for (i = 0; i < pass.length(); ++i) {
    EEPROM.write(offset + 32 + i, pass[i]);
  }
  EEPROM.write(offset+32+i,0);

  Serial.println("Writing MQTT IP to EEPROM...");
  for (i = 0; i < 4; ++i) {
    EEPROM.write(offset + 96 + i, mqttIp[i]);
  }
  EEPROM.write(offset+96+i,0);
  
  EEPROM.commit();
  Serial.println("Write EEPROM done!");
}

void startWebServer() {
  
  
    if (settingMode) {
        serialLog("IP Address: " + WiFi.softAPIP().toString());
        
        webServer.on("/settings", []() {
            String s = "<h1>Device Wi-Fi Settings</h1><p>";
            s += getDeviceId();
            s += "</p><p>Please select the ESSID from the scanned list and then enter the password.</p>";
            s += "<form method=\"get\" action=\"apsetup\"><label>ESSID: </label><select name=\"essid\">";
            s += ssidList;
            Serial.println(ssidList);
            s += "</select><br>Password: <input name=\"pass\" length=64 type=\"password\"><br>MQTT IP Addr: <input name=\"ipaddr\" length=16 type=\"text\"><input type=\"submit\"></form>";
            webServer.send(200, "text/html", makePage("Wi-Fi Settings", s));
      });
      webServer.on("/apsetup", []() {
        
          String ssid = urlDecode(webServer.arg("essid")) + "\0";
          Serial.print("ESSID: ");
          Serial.println(ssid);
          String pass = urlDecode(webServer.arg("pass"));
          Serial.print("Password: ");
          Serial.println(pass);
          String ipaddr_s = urlDecode(webServer.arg("ipaddr"));
          Serial.print("MQTT IP Address: ");
          Serial.println(ipaddr_s);
          char ipaddr[16];
          unsigned int ip[4];
          ipaddr_s.toCharArray(ipaddr, 16);
          sscanf(ipaddr, "%u.%u.%u.%u", &ip[0], &ip[1], &ip[2], &ip[3]);

          Serial.print(ip[0]);
          Serial.print(".");
          Serial.print(ip[1]);
          Serial.print(".");
          Serial.print(ip[2]);
          Serial.print(".");
          Serial.println(ip[3]);
          Serial.print("done");

          writeEeprom(0, ssid, pass, ip);

          String s = "<h1>Setup complete.</h1><p>device will be connected to \"";
          s += ssid;
          s += "\" after the restart.";
          webServer.send(200, "text/html", makePage("Wi-Fi Settings", s));
          ESP.restart();
      });
      webServer.onNotFound([]() {
          String s = "<h1>AP mode - " + getDeviceId() + "</h1><p><a href=\"/settings\">Wi-Fi Settings</a></p>";
          webServer.send(200, "text/html", makePage("AP mode - " + getDeviceId(), s));
      });
    }
    else {
        serialLog("IP Address: " + WiFi.localIP().toString());
        webServer.on("/", []() {
            String s = "<h1>STA mode</h1><p>";
            s += getDeviceId();
            s += "</p><p><a href=\"/reset\">Reset Wi-Fi Settings</a></p>";
            webServer.send(200, "text/html", makePage("STA mode", s));
          });
        webServer.on("/reset", []() {
            for (int i = 0; i < 4; ++i) {
              EEPROM.write(i, 0);
            }
            clearConfig(0);
            clearConfig(1);
            EEPROM.commit();
            String s = "<h1>Wi-Fi settings have been reset.</h1><p>Please reboot device.</p>";
            webServer.send(200, "text/html", makePage("Reset Wi-Fi Settings", s));
          });
    };

    webServer.begin();
}




int ScanNetworks::doScan(){
    int ret;
    ret = this->networkCount = WiFi.scanNetworks();
    Serial.println("Found " + String(ret) + " networks");
    return ret;
}

String ScanNetworks::getOptionList(){
    String optionList;
    for(int i = 0; i < this->networkCount; ++i){
        optionList += "<option value=\"" + WiFi.SSID(i) + "\">" + WiFi.SSID(i) + "</option>";
        Serial.println(WiFi.SSID(i));
    }
    return optionList;
}



/*             */
/*  SETUP MODE */
/*             */

void setupMode() {

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
  
        ScanNetworks scanNetworks;
        scanNetworks.doScan();
        ssidList = scanNetworks.getOptionList();

    delay(100);

    String apEssId = (String)SETUP_WLAN_PREFIX + getDeviceId();
    
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    WiFi.softAP(apEssId);

    dnsServer.start(53, "*", apIP);
    
    serialLog("Starting Access Point at " + apEssId);
}

String makePage(String title, String contents) {
    String s = "<!DOCTYPE html><html><head>";
    s += "<meta name=\"viewport\" content=\"width=device-width,user-scalable=0\">";
    s += "<title>";
    s += title;
    s += "</title></head><body>";
    s += contents;
    s += "</body></html>";
    return s;
}

String urlDecode(String input) {
    String s = input;
    s.replace("%20", " ");
    s.replace("+", " ");
    s.replace("%21", "!");
    s.replace("%22", "\"");
    s.replace("%23", "#");
    s.replace("%24", "$");
    s.replace("%25", "%");
    s.replace("%26", "&");
    s.replace("%27", "\'");
    s.replace("%28", "(");
    s.replace("%29", ")");
    s.replace("%30", "*");
    s.replace("%31", "+");
    s.replace("%2C", ",");
    s.replace("%2E", ".");
    s.replace("%2F", "/");
    s.replace("%2C", ",");
    s.replace("%3A", ":");
    s.replace("%3A", ";");
    s.replace("%3C", "<");
    s.replace("%3D", "=");
    s.replace("%3E", ">");
    s.replace("%3F", "?");
    s.replace("%40", "@");
    s.replace("%5B", "[");
    s.replace("%5C", "\\");
    s.replace("%5D", "]");
    s.replace("%5E", "^");
    s.replace("%5F", "-");
    s.replace("%60", "`");
    return s;
}



/*
 *  wlan_setup
 * 
 *  Returns true if successfully connected, false if not
 */
boolean setup_wlan(){

  serialLog("setup_wlan");

  WiFi.softAPdisconnect(true);

  if (readWlanConfig()) {
    WiFi.persistent( false );

    WiFi.mode(WIFI_STA);
    WiFi.hostname("ESP-DEVICE");

    delay(1000);
    //WiFi.begin();
    unsigned int configIndex = 0;

    if(wlanAssociate(configIndex) == true){
      //serialLog("connected first time");
      settingMode = false;
    }else{
      if(wlanAssociate(!configIndex) == true){
        serialLog("connected second time");
        settingMode = false;
      }else{
        // failed twice
        serialLog("connection failed twice");
        settingMode = true;
      }
    }

    }else{
        // no configuration found (or parsable)
        // so need to invoke settings mode
        //serialLog("no connection found");
        settingMode = true;
    }

    // Successful WLAN connection will have returned (above) so if here,
    // there is no WLAN connected. So enter setup mode
    if( settingMode == true){    
        //serialLog("starting settings mode");
        setupMode();
    }else{
        //startWebServer(); // this is done in setUpMode if applicable
    }

    startWebServer();

    //serialLog("exiting setup_wlan");
    return(!settingMode);
}

void loop_wlan(){
    if(state == CONNECTED){
#ifdef INCLUDE_OTA_PUSH
    ArduinoOTA.handle();
#endif
    }

    if(settingMode==true){
        dnsServer.processNextRequest();
    }
 
    webServer.handleClient();
}




