#include <NanoESP.h>
#include <NanoESP_MQTT.h>
#include <SoftwareSerial.h>

/*
  Change SSID and PASSWORD.
  Change Thingsspeak Access Data
*/

#define SSID "[Your SSID]"
#define PASSWORD "[Your Password]"

#define THINGSPEAK_CHANNEL_ID ""
#define THINGSPEAK_WRITE_API_KEY ""
#define THINGSPEAK_MQTT_API_KEY ""

#define LED_WLAN 13

#define TEMP A7
#define LIGHT A6

NanoESP nanoesp = NanoESP();
NanoESP_MQTT mqtt = NanoESP_MQTT(nanoesp);

//topic,         value,    QOS, retain
mqtt_msg newValues = {"channels/" + String(THINGSPEAK_CHANNEL_ID) + "/publish/" + THINGSPEAK_WRITE_API_KEY, "0", 0, false};
mqtt_msg lastWill = {"", "", 0, false};

String ip, mqttId;
int ticks = 0;

void setup()
{
  Serial.begin(19200);

  nanoesp.init();
  nanoesp.configWifiMode(1);
  nanoesp.configWifiStation(SSID, PASSWORD);

  if (nanoesp.wifiConnected())
  {
    Serial.println(F("Wifi Connected"));
    digitalWrite(LED_WLAN, HIGH);
  }
  else
  {
    Serial.println(F("Wifi not Connected"));
  }

  nanoesp.getIpMac(ip, mqttId);
  Serial.println("My IP: " + ip);

  if (mqtt.connect(0, "mqtt.thingspeak.com", 1883, mqttId, true, 120, &lastWill, "randomname", THINGSPEAK_MQTT_API_KEY))
  {
    Serial.println(F("Connected to Broker"));
  }
  else
  {
    Serial.println(F("error connecting"));
    nanoesp.serialDebug();
  }
}

void loop()
{
  int id, len;

  if (nanoesp.recvData(id, len))
  {
    mqtt.recvMQTT(id, len); 
  }
  mqtt.stayConnected(0);

  sendValues();
  delay(100);
}

void sendValues()
{
  if (ticks > 0)
  {
    ticks--;
  }
  else
  {
    newValues.value = String("field1=" + String(getTemp(), DEC) + "&field2=" + String(getLight(), DEC));
    Serial.println("Update Values: " + newValues.value);
    mqtt.publish(0, &newValues);

    ticks = 150;
  }
}

//--------------------------------Sensors----------------------------------------------------
double getTemp()
{
  //Source: http://playground.arduino.cc/ComponentLib/Thermistor2
  double Temp;
  int RawADC = analogRead(TEMP);
  Temp = log(10000.0 * ((1024.0 / RawADC - 1)));
  Temp = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * Temp * Temp)) * Temp);
  Temp = Temp - 273.15; 
  return Temp;
}

int getLight()
{
  return analogRead(LIGHT);
}