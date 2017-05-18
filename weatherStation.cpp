/*
http://www.openweathermap.com/

Change CITY, SSID and PASSWORD.
*/

#define SSID "[Your SSID]"
#define PASSWORD "[Your Password]"

#define CITY "[Your City]]"

#define DEBUG true

#define LED_WLAN 13

#define RED 3
#define GREEN 5
#define BLUE 6

#define GND 4

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal.h>

// RS, E, D4, D5, D6, D7
LiquidCrystal lcd(A0, A1, A2, A3, A4, A5);
SoftwareSerial esp8266(11, 12); // RX, TX

void setup() {
  Serial.begin(19200);
  esp8266.begin(19200);

  lcd.begin(16, 2);
  lcd.print("TCP - Weather");

  if (!espConfig()) serialDebug();
  else digitalWrite(LED_WLAN, HIGH);

  lcd.setCursor(0, 1);
  lcd.print("WLAN CONNECTED");


  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE, OUTPUT);

  pinMode(GND, OUTPUT);
  digitalWrite(GND, LOW);
}

void loop() {
  int temp  = getWeatherTemp(CITY);

  lcd.clear();
  lcd.print("Aktuelle Temp.:");
  lcd.setCursor(0, 1);
  lcd.print("    "+String(temp)+" C");

  int xtemp = round(temp);
  rgbTemp(xtemp);
  delay(60000);
}

void rgbTemp(int val)
{
  int green, blue , red ;

  if ((val <= 10) & (val >= -20))
    blue = map(val, -20, 10, 255, 0);
  else blue = 0;

  if ((val >= 10) & (val <= 40))
    green = map(val, 10, 40, 255, 0);
  else if ((val >= -20) & (val < 10))  green = map(val, -20, 10, 0, 255);
  else green = 0;

  if ((val > 10) & (val <= 40)) red = map(val, 10, 40, 0, 255);
  else red = 0;

  analogWrite(RED, red);
  analogWrite(GREEN, green);
  analogWrite(BLUE, blue);
}


float getWeatherTemp(String city)
{
  float temp;
  String  Host = "temp.fkainka.de";
  String  Subpage = "/?city="+city;

  if (sendCom("AT+CIPSTART=\"TCP\",\"" + Host + "\",80", "OK"))
  {
    String getRequest = "GET " + Subpage + " HTTP/1.1\r\nHost:" + Host + "\r\n";
    if (sendCom("AT+CIPSEND=" + String(getRequest.length() + 2), ">"))
    {
      esp8266.println(getRequest);
      if (esp8266.find("+IPD"))
      {
        if (esp8266.find("\r\n\r\n"))
        {
          if (esp8266.find("Temp:"))
          {
            int vTemp = esp8266.parseInt();
            debug("Temp: " + String(vTemp) + "C");
            temp = vTemp;
          }
          sendCom("AT+CIPCLOSE", "OK");
          return temp;
        }
      }
    }

  }
  return 0;
}

//-----------------------------------------Config ESP8266------------------------------------

boolean espConfig()
{
  boolean success = true;
  esp8266.setTimeout(5000);
  success &= sendCom("AT+RST", "ready");
  esp8266.setTimeout(1000);
  if (configStation(SSID, PASSWORD)) {
    success &= true;
    debug("WLAN Connected");
    debug("My IP is:");
    debug(sendCom("AT+CIFSR"));
  }
  else
  {
    success &= false;
  }

  success &= sendCom("AT+CIPMODE=0", "OK"); 
  success &= sendCom("AT+CIPMUX=0", "OK");

  return success;
}

boolean configStation(String vSSID, String vPASSWORT)
{
  boolean success = true;
  success &= (sendCom("AT+CWMODE=1", "OK"));
  esp8266.setTimeout(20000);
  success &= (sendCom("AT+CWJAP=\"" + String(vSSID) + "\",\"" + String(vPASSWORT) + "\"", "OK"));
  esp8266.setTimeout(1000);
  return success;
}

//-----------------------------------------------Controll ESP-----------------------------------------------------
boolean sendCom(String command, char respond[])
{
  esp8266.println(command);
  if (esp8266.findUntil(respond, "ERROR"))
  {
    return true;
  }
  else
  {
    debug("ESP SEND ERROR: " + command);
    return false;
  }
}

String sendCom(String command)
{
  esp8266.println(command);
  return esp8266.readString();
}



//-------------------------------------------------Debug Functions------------------------------------------------------
void serialDebug() {
  while (true)
  {
    if (esp8266.available())
      Serial.write(esp8266.read());
    if (Serial.available())
      esp8266.write(Serial.read());
  }
}

void debug(String Msg)
{
  if (DEBUG)
  {
    Serial.println(Msg);
  }
}