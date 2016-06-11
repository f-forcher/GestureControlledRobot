/*Programma di controllo del drone "RoboSoft v1.1.rev002"
 *
 *
 *
 *
 ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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

#define FANALE_DESTRO 5
#define FANALE_SINISTRO 6

#define PIN_ALTOPARLANTE 10
#define PIN_VENTOLA_SENS A3

#define AVANTI HIGH
#define INDIETRO LOW

#define DEBUG_ENABLED  1



SoftwareSerial BlueToothSerial(RxD,TxD);

//Inludi il file con le frequenze delle note
#include "pitches.h"

//AutoBrake si ferma da solo, autoManovra si gira anche
unsigned long controltime=150;
unsigned long synctime=500;
unsigned long currtime=0;
unsigned long oldtime1=0;
unsigned long oldtime2=0;
int pinSensore=A5;
int sensoreVal=0;
int sensoreDist=0;
int currSensDestro=0;
int currSensSinistro=0;
boolean isInManovraFlag = false;
boolean isAutoBrakeActive=true;
boolean isAutoManovraActive=false;
boolean giaRilevato=false;
boolean usaAltoparlante=false;
int ventolaPrevSstate=0;
int ventolaNewState=0;
int ventVal=0;

void setup() {
  BlueToothSerial.begin(57600);  //Set Bluetooth BaudRate to default baud rate 57600
  pinMode(13,OUTPUT);

  //Setta i pin
  /////////////////////////////////////////////////////////////////////////////
  pinMode(DIREZIONE_MOTORE_DESTRO,OUTPUT);
  pinMode(DIREZIONE_MOTORE_SINISTRO,OUTPUT);

  pinMode(PWM_MOTORE_DESTRO,OUTPUT);
  pinMode(PWM_MOTORE_SINISTRO,OUTPUT);

  pinMode(BRAKE_MOTORE_DESTRO,OUTPUT);
  pinMode(BRAKE_MOTORE_SINISTRO,OUTPUT);

  pinMode(FANALE_DESTRO,OUTPUT);
  pinMode(FANALE_SINISTRO,OUTPUT);

  pinMode(PIN_ALTOPARLANTE,OUTPUT);
  pinMode(PIN_VENTOLA_SENS,INPUT);
  /////////////////////////////////////////////////////////////////////////////

  setupBlueToothConnection();

  //HIGH = AVANTI, LOW = INDIETRO
  digitalWrite(DIREZIONE_MOTORE_DESTRO,AVANTI);
  digitalWrite(DIREZIONE_MOTORE_SINISTRO,AVANTI);
  //Fai la musichetta di accensione
  int musAccNote[] = {
    NOTE_C5,NOTE_E5,NOTE_G5,NOTE_C5,NOTE_D5  };
  int musAccDurate[] = {
    4,8,8,4,2  };

  suonaNote(musAccNote,musAccDurate,PIN_ALTOPARLANTE,5);
  digitalWrite(13,HIGH);
  delay(10000);
  digitalWrite(13,LOW);

}

void loop() {

  //Leggi il sensore e verifica cosa fare ogni (controltime) millisecondi
  currtime=millis();
  if(currtime-oldtime1>=controltime){
    oldtime1=currtime;
    sensoreVal=analogRead(pinSensore);
    currSensDestro=analogRead(CORRENTE_MOTORE_DESTRO);
    currSensSinistro=analogRead(CORRENTE_MOTORE_SINISTRO);
    sensoreDist=constrain(abs(2076/(sensoreVal - 11)),1,30);

    if(sensoreDist<13 && !giaRilevato){
      if(isAutoBrakeActive){
        giaRilevato=true;
        muoviMotori(0,0,true);
        frenaMotori(true,true,400);
      }
      //Gira su se stesso e riparti lentamente
      if(isAutoManovraActive){
        isInManovraFlag=true;
        giaRilevato=true;
        //imposta overrideFlag true per manovrare i motori anche se isInManovraFlag=true
        muoviMotori(192,-192,true);
        delay(1200);
        muoviMotori(0,0,true);
        frenaMotori(true,true,20);
        muoviMotori(192,192,true);
        isInManovraFlag=false;
      } 
    } 
    else {
      giaRilevato=false;
    }
    
    ventVal = analogRead(PIN_VENTOLA_SENS);
    //Controlla se la ventola ha cambiato stato. Se si è accesa invia "v1", altrimenti invia "v0"
    if(ventVal>300){
      ventolaNewState=1;
      if (ventolaNewState!=ventolaPrevSstate) {
        ventolaPrevSstate=1;
        BTSerialPrint("v1");
      }
    } 
    else {
      ventolaNewState=0;
      if (ventolaNewState!=ventolaPrevSstate) {
        ventolaPrevSstate=0;
        BTSerialPrint("v0");
      }
    }
  }

  //Invia i dati ogni (synctime) millisecondi
  currtime=millis();
  if(currtime-oldtime2>=synctime){
    oldtime2=currtime;
    //Invia al tablet i dati. Protocollo: S+Distanza rilevata dal sensore + Num del sensore di corrente destro + Num del sensore di corrente sinistro
    BTSerialPrint("S"+String(sensoreDist)+";"+String(currSensDestro)+";"+String(currSensSinistro));
  }

  char rCh;
  if(BlueToothSerial.available()){//Controlla se ci sono dati inviati dal bluetooth shield
    rCh = (char)BlueToothSerial.read();//Leggi la flag
    int rval1=0;
    int rval2=0;

    switch (rCh){
      //Primo valore, motore SINISTRO, secondo valore motore DESTRO!! ordine di lettura
    case 'm':
      rval1=BlueToothSerial.parseInt();
      rval2=BlueToothSerial.parseInt();
      muoviMotori(rval2, rval1, false);
      break;

      //Accendi fanali se "f1", se f0, spegnili
    case 'f':
      rval1=BlueToothSerial.parseInt();
      if(rval1==1){
        digitalWrite(FANALE_SINISTRO,HIGH);
        digitalWrite(FANALE_DESTRO,HIGH);
      }
      else {
        digitalWrite(FANALE_SINISTRO,LOW);
        digitalWrite(FANALE_DESTRO,LOW);
      }
      break;

      //Altoparlante
    case 'a':
      rval1=BlueToothSerial.parseInt();
      //Se 'a1',accendi altoparlante, stoppalo se arriva un'altra volta a
      if(rval1==1){
        isInManovraFlag=true;
        usaAltoparlante=true;
        muoviMotori(0,0,true);
        frenaMotori(true,true,200);

        char rCh2;
        while (usaAltoparlante){
          //Suona un bel reDiesis, ma di un'ottava più alta del solito
          tone(PIN_ALTOPARLANTE, NOTE_DS5);
          delay(500);
          noTone(PIN_ALTOPARLANTE);
          delay(500);
          if(BlueToothSerial.available()){//Controlla se ci sono dati inviati dal bluetooth shield
            rCh2 = (char)BlueToothSerial.read();    
            if(rCh2=='a'){
              isInManovraFlag=false;
              usaAltoparlante=false;
              noTone(PIN_ALTOPARLANTE);
            }
          }
        }
      }
      break;

    }
  }
}


////////////////////////////////////////////////////////////////////////////////////////////
/*
 *Controlli motori
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
//Frena i motori per (durata) secondi
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
 *Suona l'altoparlante
 *
 */
void suonaNote(int melody[], int noteDurations[], int pin, int numNote){
  for (int thisNote = 0; thisNote < numNote+1; thisNote++) {

    int noteDuration = 1000/noteDurations[thisNote];
    tone(pin, melody[thisNote],noteDuration);

    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);

    noTone(pin);
  }
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











