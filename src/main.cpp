#define cfgpin PIN_PD6
#define rstpin PIN_PD7
#define modepin PIN_PD5

#include <avr/io.h>

int inByte;
int outByte;
char inBuffer[8]={0};
char OutBuffer[4]={0};

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
  while(Serial0.available() == 0)
    Serial2.write("Nothing available");

  //copy received String into inBuffer
  for(int i=0; Serial0.available() != 0; i++) {
    inBuffer[i]=Serial0.read();
    Serial2.write("Succesfully copied Byte %i", i);
  }  
  //transmit received string back to ground station
  for(int i=0; i<4; i++) {
    Serial0.write(inBuffer[i]);
    Serial2.write("Succesfully transmitted byte %i",i);
  }

  Serial2.write("String transmitted");

}
