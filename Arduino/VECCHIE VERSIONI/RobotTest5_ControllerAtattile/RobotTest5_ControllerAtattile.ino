
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
 Formula per trasformare in centimetri i valori del sensore sharp
 sensoreDist=2076/(sensoreVal - 11);
 
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

#define CORRENTE_MOTORE_DESTRO A0
#define CORRENTE_MOTORE_SINISTRO A1

#define AVANTI HIGH
#define INDIETRO LOW

#define DEBUG_ENABLED  1



SoftwareSerial BlueToothSerial(RxD,TxD);

//AutoBrake si ferma da solo, autoManovra si gira anche
unsigned long controltime=500;
unsigned long currtime=0;
unsigned long oldtime=0;
int pinSensore=A5;
int sensoreVal=0;
int sensoreDist=0;
int currSensDestro=0;
int currSensSinistro=0;
boolean isInManovraFlag = false;
boolean isAutoBrakeActive=true;
boolean isAutoManovraActive=true;

void setup() {
  BlueToothSerial.begin(57600);  //Set Bluetooth BaudRate to default baud rate 57600
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

  //HIGH = AVANTI, LOW = INDIETRO
  digitalWrite(DIREZIONE_MOTORE_DESTRO,AVANTI);
  digitalWrite(DIREZIONE_MOTORE_SINISTRO,AVANTI);
  //analogWrite(redLed, 129);
  digitalWrite(13,HIGH);
  delay(10000);
  digitalWrite(13,LOW);

}

void loop() {

  //Leggi il sensore e verifica cosa fare
  currtime=millis();
  if(currtime-oldtime>=controltime){
    oldtime=currtime;
    sensoreVal=analogRead(pinSensore);
    currSensDestro=analogRead(CORRENTE_MOTORE_DESTRO);
    currSensSinistro=analogRead(CORRENTE_MOTORE_SINISTRO);
    sensoreDist=20;//constrain(2076/(sensoreVal - 11),1,30);//SI E' ROTTO IL SENSORE!!!
    //Invia al tablet i dati
    BTSerialPrint("S"+String(sensoreDist)+";"+String(currSensDestro)+";"+String(currSensSinistro));
    
    if(sensoreDist<13){
      if(isAutoBrakeActive){
        muoviMotori(0,0,true);
        frenaMotori(true,true,400);
      }
      //Gira su se stesso e riparti lentamente
      if(isAutoManovraActive){
        isInManovraFlag=true;
        //imposta overrideFlag true per manovrare i motori anche se isInManovraFlag=true
        muoviMotori(192,-192,true);
        delay(1200);
        muoviMotori(0,0,true);
        frenaMotori(true,true,20);
        muoviMotori(192,192,true);
      } 
    }
  }


  char rCh;
  if(BlueToothSerial.available()){//check if there's any data sent from the remote bluetooth shield
    rCh = (char)BlueToothSerial.read();
    int rval1=0;
    int rval2=0;
    
    switch (rCh){
      //Primo valore, motore SINISTRO, secondo valore motore DESTRO!! ordine di lettura
      case 'm':
      rval1=BlueToothSerial.parseInt();
      rval2=BlueToothSerial.parseInt();
      muoviMotori(rval2, rval1, false);
      break;
      

   /* case 'p':
      rval=BlueToothSerial.parseInt();
      analogWrite(PWM_MOTORE_SINISTRO,rval);
      break;*/
    }
  }
}


////////////////////////////////////////////////////////////////////////////////////////////
/*
 *Cotrolli motori
 */

//Mettere overrideFlag come true se si vuole saltare il controllo che impedisce di muoverlo se è in manovra (per esempio per usare comandi per manovrare...)
void muoviMotori(int velDestro, int velSinistro, boolean overrideFlag){
  if(!isInManovraFlag || overrideFlag){
    if(velDestro<0){
      digitalWrite(DIREZIONE_MOTORE_DESTRO,INDIETRO);  
      analogWrite(PWM_MOTORE_DESTRO,abs(velDestro));
    } 
    else {
      digitalWrite(DIREZIONE_MOTORE_DESTRO,AVANTI);
      analogWrite(PWM_MOTORE_DESTRO,velDestro);
    }
    if(velSinistro<0){
      digitalWrite(DIREZIONE_MOTORE_SINISTRO,INDIETRO);  
      analogWrite(PWM_MOTORE_SINISTRO,abs(velSinistro));
    } 
    else {
      digitalWrite(DIREZIONE_MOTORE_SINISTRO,AVANTI);
      analogWrite(PWM_MOTORE_SINISTRO,velSinistro);
    }
  }
}
//Frena i motori per
void frenaMotori(boolean frenaDestro, boolean frenaSinistro, int durata){
  if(frenaDestro){
    digitalWrite(BRAKE_MOTORE_DESTRO,HIGH);
  }

  if(frenaSinistro){
    digitalWrite(BRAKE_MOTORE_SINISTRO,HIGH);
  }

  delay(durata);

  digitalWrite(BRAKE_MOTORE_DESTRO,LOW);
  digitalWrite(BRAKE_MOTORE_SINISTRO,LOW);
}

//////////////////////////////////////////////////////////////////////////////////////////////
/*
 *Connessioni varie
 */
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

void BTSerialPrint(char character){
  BlueToothSerial.write(18);//startFlag = 18;
  BlueToothSerial.print(character);//Dati
  BlueToothSerial.write(19);//ack = 19;
}





