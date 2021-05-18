#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

#define WLAN_SSID       ""
#define WLAN_PASS       ""

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    ""
#define AIO_KEY         ""

Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

Adafruit_MQTT_Publish te = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/test"); /// сосояние реле
Adafruit_MQTT_Publish temp = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temp"); //// темпиратура 
Adafruit_MQTT_Publish humidity = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/humidity");  /// влажность 
Adafruit_MQTT_Publish TimeFromStart = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/TimeFromStart"); /// время с последней загрузки 
Adafruit_MQTT_Subscribe Light1 = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/button"); /// включение вентилятора 
Adafruit_MQTT_Subscribe Light2 = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/Light"); /// включение света 
Adafruit_MQTT_Subscribe VoSkolVkl = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/StarLamp"); /// время включения ламп 
Adafruit_MQTT_Subscribe DurationLamp = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/DurationLamp"); /// время включения ламп
Adafruit_MQTT_Subscribe TurningTemp = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/TurningTemp"); /// темпиратура включения венилятора 
Adafruit_MQTT_Subscribe ManualControl = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/ManualControl"); /// включение ручного режима 


void MQTT_connect();
int period = 0, insideT= 24, VentMode= 0, svet = 1, TimeFS, NaSkolVkl = 12, TimeVkl = 9, Vikl, Otkl;
uint32_t time1, VentTimer, periodVent, TimeRabotiLamp, TimeRec;
byte sensormode = 0, Manual = 0;
float h, t;
String a, b, c, qw, LastRes;
int adress = 0;


const long utcOffsetInSeconds = 21600;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

void setup() {

  // Setup MQTT subscription for onoff feed.
  mqtt.subscribe(&Light1);
  mqtt.subscribe(&Light2);
  mqtt.subscribe(&VoSkolVkl);
  mqtt.subscribe(&DurationLamp);
  mqtt.subscribe(&TurningTemp);
  mqtt.subscribe(&ManualControl);

    timeClient.begin();

}

void loop() {
if (digitalRead(Relay1) == HIGH )    a = " Fan OFF,  "; else a = " Fan is ON, ";
 if (digitalRead(Relay2) == HIGH)  b = " Light OFF,  " ; else b = "Light is ON, ";
 if (Manual == 1)  qw = " ManualMode ON " ; else qw = "ManualMode OFF ";
 c = a + b + qw;
   if (LastRes != c) {
   LastRes = c;
   char d[255];
   strcpy(d, LastRes.c_str());
    te.publish(d);
    }
///////////////////////////////////////////////////////////    
  ///////////// отправка данных на сервер/////////////////////////
  if (millis() - time1 > period) {
    time1 = millis();
  switch (sensormode) {
  case 0: 
  sensormode = 1;
  period = 10;
  h = dht.readHumidity();
  t = dht.readTemperature();
  TimeFS = millis() / 3600000; 
  humidity.publish(h);
  temp.publish(t);
  TimeFromStart.publish(TimeFS);
  timeClient.update();
  break;
  case 1:
  sensormode = 0;
  period = 55000;
 */
  }
}
//////////////////////////////////////////////////
 ///////////////////////вентилятор///////////////

 if (t > insideT && Manual == 0 || h > 65) {digitalWrite(Relay1, 0);}
 else if (t < insideT - 0.5 ) {
     if (millis() - VentTimer > periodVent && Manual == 0 ) {
       VentTimer = millis();
       switch (VentMode) {
       case 0: 
        VentMode = 1;
        periodVent = 2400000;       /////выключение 
        digitalWrite(Relay1, 1);
        break;
      case 1:   // измеряем
        VentMode = 0;
        digitalWrite(Relay1, 0);
        periodVent = 120000;      /////включение
         break;
    }

     }
       }
//////////////////////////////////////////////////////
///////////////////свет///////////////

     Vikl = TimeVkl + NaSkolVkl;
     if (timeClient.getHours() >= TimeVkl  && timeClient.getHours() < Vikl && Manual == 0 ) {
       digitalWrite(Relay2, 0);
     }
       if (Vikl > 24) {Otkl = Vikl - 24;
       } else {Otkl = Vikl;}
       if (timeClient.getHours() == Otkl) {
      digitalWrite(Relay2, 1);
     }
////////////////////////////////////////////////////////////
////////////// полкчение данных с сервера/////////////////////////////////

  MQTT_connect();
 Adafruit_MQTT_Subscribe *subscription;
 while ((subscription = mqtt.readSubscription(20000))) {
if (subscription == &Light1) {
      Serial.print(F("Got1: "));
      Serial.println((char *)Light1.lastread);
      int Light1_State = atoi((char* )Light1.lastread);
      digitalWrite(Relay1, !Light1_State);
    }

    if (subscription == &Light2) {
      Serial.print(F("Got2: "));
      Serial.println((char *)Light2.lastread);
      int Light2_State = atoi((char* )Light2.lastread);
      digitalWrite(Relay2, !Light2_State);
    }

    if (subscription == &VoSkolVkl) {
      int vrem = atoi((char* )VoSkolVkl.lastread);
      TimeVkl = vrem;  
    }

    if (subscription == &DurationLamp) {
      int Dur = atoi((char* )DurationLamp.lastread);
      NaSkolVkl = Dur;  
    }

    if (subscription == &TurningTemp) {
      int Turn = atoi((char* )TurningTemp.lastread);
      insideT = Turn;  
    }

    if (subscription == &ManualControl) {
      int Man = atoi((char* )ManualControl.lastread);
      Manual = Man;  
    }
    
  }
}

void MQTT_connect() {
  int8_t ret;
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}
