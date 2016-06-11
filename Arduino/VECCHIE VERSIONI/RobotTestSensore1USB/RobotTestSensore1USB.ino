/*
  Sends sensor data to Arduino
  (needs SensorGraph and Amarino app installed and running on Android)

  The formula to translate SensorValue into Distance (the formula is only valid for a SensorValue between 80 to 530) is:
  Distance (cm) = 2076/(SensorValue - 11)

*/
 
#include <SoftwareSerial.h>


//MeetAndroidSS meetAndroid(9600, 1, 0);

int sensor = A5;
int dist;
int sensVal;

void setup()  
{
  // we initialize analog pin 5 as an input pin
  pinMode(sensor, INPUT);
  Serial.begin(9600);
}

void loop()
{
  
  // read input pin and send result to Android
  sensVal=analogRead(sensor);
  dist=2076/(sensVal - 11);
  Serial.println(dist); 
  
  // add a little delay otherwise the phone is pretty busy
  delay(100);
}


