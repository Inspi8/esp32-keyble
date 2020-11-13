#include <Arduino.h>
#include <esp_log.h>
#include <sstream>
#include <queue>
#include <string>
#include "eQ3.h"
#include "WiFi.h"
#include "PubSubClient.h"
#include <esp_wifi.h>
#include <BLEDevice.h>

/*#define WIFI_SSID "x"
#define WIFI_PASSWORD "x"
#define MQTT_HOST "192.168.177.3"
#define MQTT_USER "x"
#define MQTT_PASSWORD "x"
#define MQTT_TOPIC_SUB "Smartlock/command"
#define MQTT_TOPIC_PUB "Smartlock"
#define ADDRESS "00:00:00:00:00:00"
#define USER_KEY "00000000000000000000000000000000"
#define USER_ID 6
#define CARD_KEY "M001AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"*/

// Include above defines:
#include "secrets.h"

eQ3* lock1;
eQ3* lock2;
bool do_open = false;
bool do_lock = false;
bool do_unlock = false;
bool wifiActive = true;
int status = 0;
int status_2 =0; 
int lock_number = 0; 

// -----------------------------------------------------------------------------
// ---[MqttCallback]------------------------------------------------------------
// -----------------------------------------------------------------------------
void MqttCallback(char* topic, byte* payload, unsigned int length) {
  
  payload[length] = '\0'; // Null terminator used to terminate the char array
  String msgString   = (char*)payload;
  String topicString = String(topic);

  if (topicString.indexOf(MQTT_TOPIC_SUB)==0){
    lock_number=1;
  }
  if (topicString.indexOf(MQTT_TOPIC_SUB_2)==0){
    lock_number=2;
  }
  
  Serial.print("# Nachricht empfangen: ");
  Serial.println(msgString);

  Serial.print("# Topic: ");
  Serial.println(topicString);

  Serial.print("# Lock Number: ");
  Serial.println(lock_number);



  if (payload[0] == '4' || msgString.indexOf("open") == 0 ) {  //open
    do_open = true;
    //Serial.println("# open ");
    }
  if (payload[0] == '3' || msgString.indexOf("lock") == 0 ) {   //lock
    do_lock = true;
    //Serial.println("# lock ");
    }
  if (payload[0] == '2' || msgString.indexOf("unlock") == 0 ){ //unlock
    do_unlock = true;
    //Serial.println("# unlock ");
    }  
}

WiFiClient wifiClient;
PubSubClient mqttClient(MQTT_HOST, 1883, &MqttCallback, wifiClient);

// -----------------------------------------------------------------------------
// ---[Wifi Signalqualität]-----------------------------------------------------
// -----------------------------------------------------------------------------
int GetWifiSignalQuality() {
float signal = 2 * (WiFi.RSSI() + 100);
if (signal > 100)
  return 100;
else
  return signal;
}

// -----------------------------------------------------------------------------
// ---[Start WiFi]--------------------------------------------------------------
// -----------------------------------------------------------------------------
void SetupWifi() {
  delay(500);
  Serial.println("# WIFI: verbinde...");
  WiFi.persistent(false);    // Verhindert das neuschreiben des Flashs
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("# WIFI: SSID and Password set");
  delay(2000);
  if (WiFi.waitForConnectResult() == WL_CONNECTED)
    Serial.println("# WIFI: Verbindung wiederhergestellt: SSiD: " + WiFi.SSID());
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("# WIFI: prüfe SSiD: " + String(WIFI_SSID));
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    delay(500);
      if (WiFi.waitForConnectResult() == WL_CONNECTED) {
      Serial.println("# WIFI: verbunden!");
      break;
    }
  }
  Serial.println("# WIFI: Signalqualität: "+ String(GetWifiSignalQuality()) + "%");
  Serial.print("# WIFI: IP-Adresse: "); Serial.println(WiFi.localIP());
}

// -----------------------------------------------------------------------------
// ---[MQTT-Setup]--------------------------------------------------------------
// -----------------------------------------------------------------------------
void SetupMqtt() {
  while (!mqttClient.connected()) { // Loop until we're reconnected to the MQTT server
  	Serial.print("# Mit MQTT-Broker verbinden... ");
  	if (mqttClient.connect("SmartLock", MQTT_USER, MQTT_PASSWORD)) {
  		Serial.println("\tverbunden!");
  		mqttClient.subscribe(MQTT_TOPIC_SUB);
      mqttClient.subscribe(MQTT_TOPIC_SUB_2); 
  	}
  	else {
      Serial.print("\t Fehler, rc=");
      Serial.println(mqttClient.state());
      //abort();
    }
  }
}

// -----------------------------------------------------------------------------
// ---[Setup]-------------------------------------------------------------------
// -----------------------------------------------------------------------------
void setup() {
    Serial.begin(115200);
    Serial.println("Starte...");
    Serial.setDebugOutput(true);

    SetupWifi();

    //Bluetooth
    BLEDevice::init("");

    lock1= new eQ3(ADDRESS, USER_KEY, USER_ID);
    lock2= new eQ3(ADDRESS_2, USER_KEY_2, USER_ID_2);

    //MQTT
    SetupMqtt();
}


// -----------------------------------------------------------------------------
// ---[SetWifi]-----------------------------------------------------------------
// -----------------------------------------------------------------------------
void SetWifi(bool active) {
  wifiActive = active;
  if (active) {
    WiFi.mode(WIFI_STA);
    Serial.println("# WiFi aktiviert");
  }
  else {
    WiFi.mode(WIFI_OFF);
    Serial.println("# WiFi deaktiviert");
  }
}

// -----------------------------------------------------------------------------
// ---[loop]--------------------------------------------------------------------
// -----------------------------------------------------------------------------
void loop() {
    // Wifi reconnect
    if (WiFi.status() != WL_CONNECTED && wifiActive) {
      Serial.println("# WiFi getrennt, wiederverbinden...");
      SetupWifi();
    }

    // MQTT connected?
    if (WiFi.status() == WL_CONNECTED && !mqttClient.connected() && wifiActive) {
      Serial.println("# MQTT getrennt, wiederverbinden...");
      SetupMqtt();
    }
    else
      mqttClient.loop();
    
    /*
    keyble = new eQ3(ADDRESS, USER_KEY, USER_ID);

    if (keyble->_LockStatus != status){
      status = keyble->_LockStatus;
      // convert status for publish
      char tmp[1];
      String tmp_str = String(status);
      tmp_str.toCharArray(tmp, tmp_str.length() + 1);
      // publish
      mqttClient.publish(MQTT_TOPIC_PUB, tmp);
    }
    delete keyble;
    keyble=NULL;
    
    keyble = new eQ3(ADDRESS_2, USER_KEY_2, USER_ID_2);
    if (keyble->_LockStatus != status){
      status = keyble->_LockStatus;
      // convert status for publish
      char tmp[1];
      String tmp_str = String(status);
      tmp_str.toCharArray(tmp, tmp_str.length() + 1);
      // publish
      mqttClient.publish(MQTT_TOPIC_PUB_2, tmp);
    }
    */
    
    if ((do_open||do_lock||do_unlock) && (lock_number==1 || lock_number==2)) {
      SetWifi(false);

      if (do_open) {
        Serial.println("Öffnen + Schlossfalle");
        if (lock_number==1) {
            lock1->open();
        }
        if (lock_number==2) {
            lock2->open();
        }

        do_open = false;
      }

      if (do_lock) {
        Serial.println("Schließen");
        if (lock_number==1) {
            lock1->lock();
        }
        if (lock_number==2) {
            lock2->lock();
        }
        do_lock = false;
      }

      if (do_unlock) {
        Serial.println("Öffnen");
        if (lock_number==1) {
            lock1->unlock();
        }
        if (lock_number==2) {
            lock2->unlock();
        }
        do_unlock = false;
      }
      Serial.println("# Delay");
      
      delay(1000);
      Serial.println("# Activate Wifi");
      
      SetWifi(true);

      //Serial.println("# Delete keyble Object");
      //delete keyble;
      //Serial.println("# keyble=NULL");
      //keyble=NULL;
    }
}
