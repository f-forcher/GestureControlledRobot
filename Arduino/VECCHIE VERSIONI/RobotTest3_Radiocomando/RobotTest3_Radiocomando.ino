
//COME USARE AMARINO: PER OGNI TRASMISSIONE BISOGNA INVIARE PRIMA UN BYTE DI INIZIO TRASMISSIONE, POI IL MESSAGGIO POI IL BYTE DI ACKNOWLEDGEMENT
/*
	startFlag = 18;
 	ack = 19;
 	delimiter = 59; //';' Delimita i dati inviati, tipo "A12.2;9.4;3.3"
 
 Questo programma legge un sensore sharp distance sul pin A5 e lo comunica via bluetooth
 TEST Robot
 
 /////////////////////////////////////////////////////////////////////////////////////////////
 Function	pins per Ch. A	pins per Ch. B

Direction	D12	        D13
PWM	        D3	        D11
Brake	        D9	        D8
Current Sensing	A0	        A1


 MOTORE A=MOTORE DI DESTRA!!
 MOTORE B=MOTORE DI SINISTRA!!

 NOTA: L'ALTOPARLANE NON ANDRA' CON I MOTORI ACCESI! PWM 3 e 11 occupate dal generatore di onde quadre.
//////////////////////////////////////////////////////////////////////////////////////////////
 
 
 */


#include <SoftwareSerial.h>   //Software Serial Port
#define RxD 4
#define TxD 7

#define DIREZIONE_MOTORE_DESTRO 12
#define DIREZIONE_MOTORE_SINISTRO 13

#define PWM_MOTORE_DESTRO 3
#define PWM_MOTORE_SINISTRO 11

#define BRAKE_MOTORE_DESTRO 9
#define BRAKE_MOTORE_SINISTRO 8

#define DEBUG_ENABLED  1

SoftwareSerial BlueToothSerial(RxD,TxD);

long sleeptime=0;
long currtime=0;
int pinSensore=A5;
int sensoreVal=0;
int sensoreDist=0;

void setup() {
  BlueToothSerial.begin(57600);  //Set BluetoothBee BaudRate to default baud rate 57600
  pinMode(13,OUTPUT);
  
  //Setta i pin dei motori come output
  /////////////////////////////////////////////////////////////////////////////
  pinMode(DIREZIONE_MOTORE_DESTRO,OUTPUT);
  pinMode(DIREZIONE_MOTORE_SINISTRO,OUTPUT);
  
  pinMode(PWM_MOTORE_DESTRO,OUTPUT);
  pinMode(PWM_MOTORE_SINISTRO,OUTPUT);
  
  pinMode(BRAKE_MOTORE_DESTRO,OUTPUT);
  pinMode(BRAKE_MOTORE_SINISTRO,OUTPUT);
  /////////////////////////////////////////////////////////////////////////////
  
  setupBlueToothConnection();
  digitalWrite(DIREZIONE_MOTORE_DESTRO,HIGH);
  digitalWrite(DIREZIONE_MOTORE_SINISTRO,HIGH);
  //analogWrite(redLed, 129);
  digitalWrite(13,HIGH);
  delay(2000);

}

void loop() {
  char rCh;
  if(BlueToothSerial.available()){//check if there's any data sent from the remote bluetooth shield
    rCh = (char)BlueToothSerial.read();
    int rval=0;

    switch (rCh){
      //Aggiorna i colori dei led  CAMBIO: Led Bianco, solo 9 è collegato
    case 'o':
      rval=BlueToothSerial.parseInt();
      analogWrite(PWM_MOTORE_DESTRO,rval);
      break;

    case 'p':
      rval=BlueToothSerial.parseInt();
      analogWrite(PWM_MOTORE_SINISTRO,rval);
      break;

    /*case 'q':
      rval=BlueToothSerial.parseInt();
      red(rval);
      break;*/
    }
  }
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

