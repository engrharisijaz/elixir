#include <ThingSpeak.h>
#include <FirebaseArduino.h>
#include <ESP8266WiFi.h>

#define FIREBASE_HOST "**************"
#define FIREBASE_AUTH "**************"
#define WIFI_SSID "DELL"                                               //your WiFi SSID for which yout NodeMCU connects
#define WIFI_PASSWORD "12345678"  

#define Relay1 D7
#define ANALOG_INPUT A0

int rel1;


float nVPP1;
float nCurrThruResistorPP1;
float nCurrThruResistorRMS1;
float nCurrentThruWire1;

String apiWritekey = "**************";
const char* server = "api.thingspeak.com";
WiFiClient client;


void setup()
{
  Serial.begin(115200);
  pinMode(Relay1, OUTPUT);
  pinMode(D1, OUTPUT);
  digitalWrite(Relay1, HIGH);
  digitalWrite(D1, LOW);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("connected:");
  Serial.println(WiFi.localIP());

  pinMode(ANALOG_INPUT, INPUT);

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  
}


float getVPP()
{
  float result;
  int readValue;
  int maxValue = 0;
  uint32_t start_time = millis();
  while ((millis() - start_time) < 1000)
  {
    readValue = analogRead(ANALOG_INPUT);
    if (readValue > maxValue)
    {
      maxValue = readValue;
    }
  }
  result = (maxValue * 3.3) / 1024.0;
  return result;
}
void loop()
{

  float value;

  rel1 = Firebase.getString("FB7").toInt();

  if (rel1 == 1)
  {
    digitalWrite(Relay1, HIGH);
    Serial.println("Relay 1 ON");
    nVPP1 = getVPP();
    nCurrThruResistorPP1 = nVPP1 / 330.0;
    nCurrThruResistorRMS1 = nCurrThruResistorPP1 * 0.707;
    nCurrentThruWire1 = nCurrThruResistorRMS1 * 1000;
    Serial.println (nCurrentThruWire1, 3);
    delay(500);
  }
  if (rel1 == 0)
  {
    digitalWrite(Relay1, LOW);
    Serial.println("Relay 1 OFF");
    nCurrentThruWire1 = 0;
    Serial.println (nCurrentThruWire1);
  }

  if (client.connect(server, 80))
  {
    String tsData = apiWritekey;
    tsData += "&field1=";
    tsData += String(nCurrentThruWire1, 3);
    tsData += "\r\n\r\n";

    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + apiWritekey + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(tsData.length());
    client.print("\n\n"); 
    client.print(tsData);
    delay(1000);
  }
  
  client.stop();
}
