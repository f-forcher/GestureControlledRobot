
//COME USARE AMARINO: PER OGNI TRASMISSIONE BISOGNA INVIARE PRIMA UN BYTE DI INIZIO TRASMISSIONE, POI IL MESSAGGIO POI IL BYTE DI ACKNOWLEDGEMENT
/*
	startFlag = 18;
	ack = 19;
	delimiter = 59; //';' Delimita i dati inviati, tipo "A12.2;9.4;3.3"

  Questo programma legge un sensore sharp distance sul pin A5 e lo comunica via bluetooth
  TEST Robot

*/

//#include <MeetAndroidSS.h>   //Amarino
#include <SoftwareSerial.h>   //Software Serial Port
#define RxD 4
#define TxD 7

#define DEBUG_ENABLED  1

SoftwareSerial BlueToothSerial(RxD,TxD);

long sleeptime=0;
long currtime=0;
int pinSensore=A5;
int sensoreVal=0;
int sensoreDist=0;

void setup() {
  BlueToothSerial.begin(57600);  //Set BluetoothBee BaudRate to default baud rate 38400
  pinMode(13,OUTPUT);
  
  setupBlueToothConnection();
  //analogWrite(redLed, 129);
  digitalWrite(13,HIGH);
  delay(2000);

}

void loop() {
  /////////////////////////////////////////////
  
  //meetAndroidSS.send(analogRead(sensor));
  sensoreVal=analogRead(pinSensore);
  sensoreDist=2076/(sensoreVal - 11);
  
  BTSerialPrint(sensoreDist);
  
  /////////////////////////////////////////////

  delay(500);
}

void setupBlueToothConnection()
{
  
  BlueToothSerial.print("\r\n+STWMOD=0\r\n"); //set the bluetooth work in slave mode
  BlueToothSerial.print("\r\n+STNA=SeeedBTSlave\r\n"); //set the bluetooth name as "SeeedBTSlave"
  BlueToothSerial.print("\r\n+STOAUT=1\r\n"); // Permit Paired device to connect me
  BlueToothSerial.print("\r\n+STAUTO=0\r\n"); // Auto-connection should be forbidden here
  delay(2000); // This delay is required.
  BlueToothSerial.print("\r\n+INQ=1\r\n"); //make the slave bluetooth inquirable 
  BlueToothSerial.println("The slave bluetooth is inquirable!");
  delay(2000); // This delay is required.
  BlueToothSerial.flush();
  
}

//Print secondo il protocollo di Amarino/MeetAndroid
void BTSerialPrint(String string){
  BlueToothSerial.write(18);//startFlag = 18;
  BlueToothSerial.print(string);//Dati
  BlueToothSerial.write(19);//ack = 19;
}

void BTSerialPrint(int integer){
  BlueToothSerial.write(18);//startFlag = 18;
  BlueToothSerial.print(integer);//Dati
  BlueToothSerial.write(19);//ack = 19;
}

void BTSerialPrint(unsigned int uinteger){
  BlueToothSerial.write(18);//startFlag = 18;
  BlueToothSerial.print(uinteger);//Dati
  BlueToothSerial.write(19);//ack = 19;
}

void BTSerialPrint(long longint){
  BlueToothSerial.write(18);//startFlag = 18;
  BlueToothSerial.print(longint);//Dati
  BlueToothSerial.write(19);//ack = 19;
}

void BTSerialPrint(unsigned long ulongint){
  BlueToothSerial.write(18);//startFlag = 18;
  BlueToothSerial.print(ulongint);//Dati
  BlueToothSerial.write(19);//ack = 19;
}






