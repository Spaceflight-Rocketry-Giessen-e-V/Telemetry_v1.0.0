#include <Arduino.h>
#include <stdio.h>

//Pin allocation
int ledpin1 = PIN_PD3;
int ledpin2 = PIN_PD4;
int modepin = PIN_PD5;
int cfgpin = PIN_PD6;
int rstpin = PIN_PD7;


//UART0 = Radiomodul

void setup() {
 //Pin initialisation
  {
    pinMode(ledpin1, OUTPUT);
    pinMode(ledpin2, OUTPUT);
    pinMode(modepin, OUTPUT);
    pinMode(cfgpin, OUTPUT);
    pinMode(rstpin, OUTPUT);
  }
    //Pin standard mode
  {
    digitalWrite(modepin, HIGH);
    digitalWrite(cfgpin, HIGH);
    digitalWrite(rstpin, HIGH);
    
    digitalWrite(ledpin1, LOW);
    digitalWrite(ledpin2, LOW);

  }
  //Serial configuration
  {
    Serial.begin(19200);
    Serial0.begin(19200);
  }
  
  //Waiting for first user input
  {
    while(Serial.available() == 0);
    Serial.read();
    Serial.print("Setup complete. Entering normal operation mode. | ");
  }
}

void loop() {
  //Test programm which indicates a fully functioning UPDI-programmer
  for(int i=0; i<5; i++)
  {
    digitalWrite(ledpin1, HIGH);
    delay(500);
    digitalWrite(ledpin2, HIGH);
    delay(1000);
    digitalWrite(ledpin1, LOW);
    digitalWrite(ledpin2, LOW);
  }
}
