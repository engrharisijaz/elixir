#include <Firebase.h>
#include <FirebaseArduino.h>
#include <FirebaseCloudMessaging.h>
#include <FirebaseError.h>
#include <FirebaseHttpClient.h>
#include <FirebaseObject.h>

#include <ThingSpeak.h>
#include <ESP8266WiFi.h>

#define FIREBASE_HOST "**************"
#define FIREBASE_AUTH "**************"

#define WIFI_SSID "DELL"
#define WIFI_PASSWORD "12345678"

#define Relay1 D7
#define Relay2 D8


#define MUX_A D4
#define MUX_B D3
#define MUX_C D2

#define ANALOG_INPUT A0

int rel1, rel2;


float nVPP1;
float nCurrThruResistorPP1;
float nCurrThruResistorRMS1;
float nCurrentThruWire1;
float nVPP2;
float nCurrThruResistorPP2;
float nCurrThruResistorRMS2;
float nCurrentThruWire2;




String apiWritekey = "**************";
const char* server = "api.thingspeak.com";
WiFiClient client;


void setup()
{
  Serial.begin(115200);
  pinMode(Relay1, OUTPUT);
  pinMode(Relay2, OUTPUT);


  pinMode(MUX_A, OUTPUT);
  pinMode(MUX_B, OUTPUT);
  pinMode(MUX_C, OUTPUT);
  pinMode(ANALOG_INPUT, INPUT);

  digitalWrite(Relay1, HIGH);
  digitalWrite(Relay2, HIGH);

  pinMode(D1, OUTPUT);
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
  delay(5000);
//  Firebase.setInt("FB1", 0);
//  Firebase.setInt("FB2", 0);
}

void changeMux(int c, int b, int a) {
  digitalWrite(MUX_A, a);
  digitalWrite(MUX_B, b);
  digitalWrite(MUX_C, c);
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

  rel1 = Firebase.getString("FB5").toInt();

  rel2 = Firebase.getString("FB6").toInt();

  if (rel1 == 1)
  {
    digitalWrite(Relay1, HIGH);
    Serial.println("Relay 1 ON");
    changeMux(LOW, LOW, LOW);
    nVPP1 = getVPP();
    nCurrThruResistorPP1 = nVPP1 / 380.0;
    nCurrThruResistorRMS1 = nCurrThruResistorPP1 * 0.707;
    nCurrentThruWire1 = nCurrThruResistorRMS1 * 1000;
    Serial.println (nCurrentThruWire1, 3);
    delay(200);
  }
  if (rel1 == 0)
  {
    digitalWrite(Relay1, LOW);
    Serial.println("Relay 1 OFF");
    nCurrentThruWire1 = 0;
    Serial.println (nCurrentThruWire1);
  }
  if (rel2 == 1)                                                          // If, the Status is 1, turn on the Relay2
  {
    digitalWrite(Relay2, HIGH);
    Serial.println("Relay 2 ON");
    changeMux(LOW, LOW, HIGH);
    nVPP2 = getVPP();
    nCurrThruResistorPP2 = nVPP2 / 380.0;
    nCurrThruResistorRMS2 = nCurrThruResistorPP2 * 0.707;
    nCurrentThruWire2 = nCurrThruResistorRMS2 * 1000;
    Serial.println (nCurrentThruWire2, 3);
    delay(200);
  }
  if (rel2 == 0)                                                   // If, the Status is 0, turn Off the Relay2
  {
    digitalWrite(Relay2, LOW);
    Serial.println("Relay 2 OFF");
    nCurrentThruWire2 = 0;
    Serial.println (nCurrentThruWire2);
  }


  if (client.connect(server, 80))
  {
    String tsData = apiWritekey;
    tsData += "&field1=";
    tsData += String(nCurrentThruWire1, 3);
    tsData += "\r\n\r\n";
    tsData += "&field2=";
    tsData += String(nCurrentThruWire2, 3);
    tsData += "\r\n\r\n";

    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + apiWritekey + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(tsData.length());
    client.print("\n\n");  // the 2 carriage returns indicate closing of Header fields & starting of data
    client.print(tsData);
    delay(1000);
  }
  client.stop();

}
