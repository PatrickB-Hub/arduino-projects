

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <NanoESP.h>
#include <NanoESP_MQTT.h>

/* 
  Change SSID and PASSWORD.
*/

#define SSID "[Your SSID]"
#define PASSWORD "[Your Password]"

NanoESP nanoesp = NanoESP();
NanoESP_MQTT mqtt = NanoESP_MQTT(nanoesp);

String ip, mqttId;

#define SWITCH_ROOM_1 5
#define SWITCH_ROOM_2 10

#define LIGHT_ROOM_1 9
#define LIGHT_ROOM_2 13

unsigned long previousMillisAlarm = 0;
unsigned long intervalAlarm = 1000;
int durationAlarm;
bool alarmActive = false;
bool alarmToggle;

bool alarm_status_v;
bool room1_light_status_v;
bool room2_light_status_v;

void setup() {
  pinMode(SWITCH_ROOM_1, INPUT_PULLUP);
  pinMode(SWITCH_ROOM_2, INPUT_PULLUP);
  pinMode(LIGHT_ROOM_1, OUTPUT);
  pinMode(LIGHT_ROOM_2, OUTPUT);

  Serial.begin(19200);
  nanoesp.init();

  // Only if you use a new Network:
  // nanoesp.configWifi(STATION, SSID, PASSWORD);

  //Station Mode
  nanoesp.configWifiMode(STATION);

  // Wait till Connected to WiFi
  if (nanoesp.wifiConnected())Serial.println(F("WLAN Connected")); else Serial.println(F("Error WLAN Connect"));

  //Print IP to Terminal
  nanoesp.getIpMac( ip,  mqttId);
  Serial.println("My IP: " + ip);

  //Print MQTT_ID to Terminal
  mqttId.replace(":", "");
  Serial.println("My MQTT ID: " + mqttId + "\n");

  //if (mqtt.connect(0, "broker.hivemq.com", 1883, mqttId)) {  //connect to alternative broker
  if (mqtt.connect(0, "broker.fkainka.de", 1883, mqttId)) {  //connect to broker
    Serial.println(F("Connected to Broker"));

    Serial.println(F("My personal Topic is:"));
    Serial.println("NanoESP/" + mqttId + "/\n");

    //subcribe: Connection ID, Topic, Max Qualitiy of Service, Function to call when msg recieved
    if (mqtt.subscribe(0, "NanoESP/" + mqttId + "/room1/light", 0, room1_light_status)) Serial.println(F("Subscribed to /room1/light"));
    if (mqtt.subscribe(0, "NanoESP/" + mqttId + "/room1/light", 0,  room1_light)) Serial.println(F("Subscribed to /room1/light"));
    if (mqtt.subscribe(0, "NanoESP/" + mqttId + "/room2/light", 0,  room2_light_status)) Serial.println(F("Subscribed to /room2/light"));
    if (mqtt.subscribe(0, "NanoESP/" + mqttId + "/room2/light", 0,  room2_light)) Serial.println(F("Subscribed to /room2/light"));

  }
  else {
    Serial.println(F("error connecting"));
  }
}


void loop() {
  int id, len;

  if (nanoesp.recvData(id, len)) {
    mqtt.recvMQTT(id, len); //auto handle recieved data by defined functions
  }

  checkConditions();
  mqtt.stayConnected(0);
}

void checkConditions() {
  room1_button_check();
  room2_button_check();
}


// ---------------------------- Room1 -----------------------------------------
// ----------------------------- Switch -----------------------------------------

//Switch knows about the light status so it can toggle the light
void room1_light_status (String value) {
  Serial.println("Light status Room 1 :" + value);
  if (value == "1") {
    room1_light_status_v = true;
  }
  else if (value == "0") {
    room1_light_status_v = false;
  }

}

void room1_button_check () {
  if (!digitalRead(SWITCH_ROOM_1)) {
    if (mqtt.publish(0, "NanoESP/" + mqttId + "/room1/light", String(!room1_light_status_v))) Serial.println(F("Button 1 pressed\n"));
    while (!digitalRead(SWITCH_ROOM_1)) {};
  }
}

// ------------------------------------ Light ---------------------------------------
void room1_light (String value) {
  if (value == "1") {
    Serial.println(F("Light on"));
    digitalWrite(LIGHT_ROOM_1, HIGH);
  }
  else if (value == "0") {
    Serial.println(F("Light off"));
    digitalWrite(LIGHT_ROOM_1, LOW);
  }
}


// ---------------------------- Room2 ---------------------------------------------
// ---------------------------- Switch ---------------------------------------------

//Switch knows about the light status so it can toggle the light
void room2_light_status (String value) {
  Serial.println("Light status Room 2 :" + value);
  if (value == "1") {
    room2_light_status_v = true;
  }
  else if (value == "0") {
    room2_light_status_v = false;
  }
}

void room2_button_check () {
  if (!digitalRead(SWITCH_ROOM_2)) {
    if (mqtt.publish(0, "NanoESP/" + mqttId + "/room2/light", String(!room2_light_status_v))) Serial.println(F("Button 2 pressed\n"));
    while (!digitalRead(SWITCH_ROOM_2)) {};
  }
}


//------------------------------------------- Light --------------------------------------
void room2_light (String value) {
  if (value == "1") {
    Serial.println(F("Light on"));
    digitalWrite(LIGHT_ROOM_2, HIGH);
  }
  else if (value == "0") {
    Serial.println(F("Light off"));
    digitalWrite(LIGHT_ROOM_2, LOW);
  }

}









