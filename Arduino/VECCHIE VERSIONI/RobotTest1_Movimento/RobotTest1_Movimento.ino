/*
TEST MOTORI BASE

**
Function	pins per Ch. A	pins per Ch. B

Direction	D12	        D13
PWM	        D3	        D11
Brake	        D9	        D8
Current Sensing	A0	        A1
**

MOTORE A=MOTORE DI DESTRA!!
MOTORE B=MOTORE DI SINISTRA!!

NOTA: L'ALTOPARLANE NON ANDRA' CON I MOTORI ACCESI! PWM 3 e 11 occupate dal generatore di onde quadre.

*/

// Pin 13 has an LED connected on most Arduino boards.
// give it a name:
int speedDestra = 3;
int speedSinistra = 11;
int i=0;

// the setup routine runs once when you press reset:
void setup() {                
  // initialize the digital pin as an output.
  pinMode(speedDestra, OUTPUT);
  pinMode(speedSinistra, OUTPUT);
  delay(2000);
  
  analogWrite(speedDestra,255);
  analogWrite(speedSinistra, 255);
}

// the loop routine runs over and over again forever:
void loop() {

}
