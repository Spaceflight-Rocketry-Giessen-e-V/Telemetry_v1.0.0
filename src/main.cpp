#define cfgpin PIN_PD6
#define rstpin PIN_PD7
#define modepin PIN_PD5

#include <avr/io.h>

int inByte;
int outByte;
char inBuffer[8]={0};
char OutBuffer[4]={0};

int serial0_wait(int delay_microsec);
int serial2_wait(int delay_microsec);

//Serial.print() used for Strings and more than one byte at a time
//Serial.write() used for single bytes

//Objectives for testing: Basic function of the radio modules, Selecting baud rate on serial montitor interface

void setup() {
  //set PD5 (mode), PD6 (cfg), PD7 (rst) as output
  PORTD.DIRSET = PIN5_bm | PIN6_bm | PIN7_bm;  

  //Pin standard mode
  PORTD.OUTSET = PIN7_bm | PIN6_bm; //set PD7 and PD6 to HIGH

  Serial2.begin(19200); //referring to serial monitor
  Serial0.begin(19200); //referring to radio module
}

void loop() {
  //do nothing until something is available on RX0
  while(serial0_wait(600) == 0)
    Serial2.print("Nothing available");

  //copy received String into inBuffer
  for(int i=0; serial0_wait(600) != 0; i++) {
    inBuffer[i]=Serial0.read();
    Serial2.print("Succesfully copied Byte " + i);
  }  
  //transmit received string back to ground station
  for(int i=0; i<4; i++) {
    Serial0.write(inBuffer[i]);
    Serial2.print("Succesfully transmitted byte " + i);
  }

  Serial2.print("String transmitted");
  delay(1000);
}


//Waiting whether serial.available() == true in given time
int serial0_wait(int delay_microsec)
{
  for(int i = 0; i < (delay_microsec / 10) && Serial0.available() == 0; i++)
  {
    delayMicroseconds(10);
  }
  return Serial0.available();
}

//Waiting whether serial2.available() == true in given time
int serial2_wait(int delay_microsec)
{
  for(int i = 0; i < (delay_microsec / 10) && Serial2.available() == 0; i++)
  {
    delayMicroseconds(10);
  }
  return Serial2.available();
}
