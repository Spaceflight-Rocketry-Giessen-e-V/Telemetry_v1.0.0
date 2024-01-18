#include <Arduino.h>
#include <stdio.h>

//Pin allocation
int ledpin1 = 9;
int ledpin2 = 10;
int cfgpin = 12;
int rstpin = 13;
int powpin = 1;
int modepin = 11;
//eventuell SPI und UART Zuweisungen

int serial0_wait(int delay_microsec);
int serial1_wait(int delay_microsec);
int serial2_wait(int delay_microsec);

void setup() 
{
  //Pin initialisation
  {
    pinMode(ledpin1, OUTPUT);
    pinMode(ledpin2, OUTPUT);
    pinMode(cfgpin, OUTPUT);
    pinMode(rstpin, OUTPUT);
    pinMode(powpin, OUTPUT);
    pinMode(modepin, INPUT);
  }

  //Pin standard mode
  {
    digitalWrite(powpin, LOW);
    digitalWrite(rstpin, HIGH);
    digitalWrite(cfgpin, HIGH);
  }

  //Mode pin check
  //Data mode
  if(digitalRead(modepin) == HIGH)
  {
    //eventuell Serialx.setRX(piny); und Serialx.setTX(pinz)
    Serial2.begin(19200); //Reffering to external serial connection
    //...
  }
  //Normal operation mode
  else
  {
    //Serial configuration
    {
      //eventuell Serialx.setRX(piny); und Serialx.setTX(pinz)
      Serial0.begin(19200); //Referring to the radio module
    }

    //Storage cell initialisation
    {
      //...
    }
    
    char inByte;

    //Handshake
    while(true)
    {
      if(Serial0.available())
      {
        inByte = Serial0.read();
        if(inByte == 'H')
        {
          Serial0.write('H'); //To be interpreted in ground systems
          break;
        }
      }      
    }

    //eventuell Uhrensynchronisation

    //POW Pin HIGH
    while(true)
    {
      while(Serial0.available() == 0);
      char InBuffer[4] = {0};
      //Puts received string into 8 byte array
      for(int i = 0; i < 4 && serial0_wait(600); i++)
      {
        InBuffer[i] = Serial2.read();
      }
      int q;
      //Checks for a "C" char within the input
      for(q = 0; InBuffer[q] != 'C' && q <= 4; q++);
      //Checks for the string "CMD" within the Input
      if(InBuffer[q] == 'C' && InBuffer[q+1] == 'M' && InBuffer[q+2] == 'D') 
      {
        //Checks for a char "P" following a "CMD" string to set POW-Pin HIGH
        if(InBuffer[q+3] == 'P') 
        {
          digitalWrite(powpin, LOW);
          delay(5);
          Serial1.begin(19200); //Referring to pin socket (bus)
          break;
        }
      }
    }
    
    //Function tests
    {
      
    }



  }
}

void loop() 
{
  
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

//Waiting whether serial1available() == true in given time
int serial1_wait(int delay_microsec)
{
  for(int i = 0; i < (delay_microsec / 10) && Serial1.available() == 0; i++)
  {
    delayMicroseconds(10);
  }
  return Serial1.available();
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