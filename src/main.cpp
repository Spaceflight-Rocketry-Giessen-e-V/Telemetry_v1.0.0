#include <Arduino.h>
#include "config_functions.h"

//Zeit schicken vor Testpaket

unsigned const int delta_r_max = 3000;	    //Max. darstellbare Entfernung vom Startort in m
unsigned const char delta_d_min = 6;	      //Min. darstellbare Auflösung des Standorts in m
unsigned const char delta_h_min = 5;        //Min. darstellbare Auflösung der Höhe in m

signed int lat_ref = 0;
signed int long_ref = 0;

char inBuffer[5] = {0};
char inByte = 0;

char handshake();
char powerup();
void reference_coordinates();
void flightmode();

int serial_wait(int delay_microsec);
int serial2_wait(int delay_microsec);
char check_serial_ports(char condition = 0);
void flush_serial_ports();
void data_handling();

//Pin allocation
int ledpin1 = 0;
int ledpin2 = 1;
int ledpin3 = 2;
int ledpin4 = 13;
int rstpin = 3;
int cfgpin = 4;
int rtspin = 5;

void setup() 
{
  //Pin initialisation
  {
    pinMode(ledpin1, OUTPUT);
    pinMode(ledpin2, OUTPUT);
    pinMode(ledpin3, OUTPUT);
    pinMode(ledpin4, OUTPUT);
    pinMode(rstpin, OUTPUT);
    pinMode(cfgpin, OUTPUT);
    pinMode(rtspin, OUTPUT);
  }

  //Pin standard mode
  {
    digitalWrite(rstpin, HIGH);
    digitalWrite(cfgpin, HIGH);
    digitalWrite(rtspin, HIGH);
    
    digitalWrite(ledpin1, LOW);
    digitalWrite(ledpin2, LOW);
    digitalWrite(ledpin3, LOW);
    digitalWrite(ledpin4, HIGH);
  }

  //Serial configuration
  {
    Serial.begin(19200);
    Serial2.setRX(7);
    Serial2.begin(19200);
  }
  
  //Waiting for first user input
  {
    Serial.print("Enter any character to continue. | ");
    while(Serial.available() == 0);
    flush_serial_ports();
  }
  
  //Handshake protocol
  {
    while(true)
    {
      Serial.print("Enter H to initiate handshake protocol. | ");
      if(check_serial_ports('H') == 1)
      {
        if(handshake() == 1)
        {
          break;
        }
        else
        {
          continue;
        }
      }
    }
  }

  //Clock synchronization (Zeit an Boden schicken)

  //Abort Cycle
  while(true)
  {
    //Power up command
    {
      while(true)
      {
        Serial.print("Enter P to initiate powerup procedure. | ");
        if(check_serial_ports('P') == 1)
        {
          if(powerup() == 1)
          {
            break;
          }
          else
          {
            continue;
          }
        }
      }
    }

    //Waiting for incoming reference coordinates
    {
      if(check_serial_ports() == 2)
      {
        reference_coordinates();
      }
      flush_serial_ports();
    }
    
    //Incoming data package
    {
      if(check_serial_ports() == 2)
      {
        delay_microsec(2500); // 4 * 600 + puffer
        if(Serial2.available() >= 5)
        {
          data_handling();
        }
        Serial.print(" | ");
      }
      flush_serial_ports();
    }

    //Data Package ok?

    //Flight mode command
    {
      while(true)
      {
        Serial.print("Enter D to initiate flight mode initialisation procedure. | ");
        if(check_serial_ports('D') == 1)
        {
          if(flightmode() == 1)
          {
            break;
          }
          else
          {
            continue;
          }
        }
      }
    }

    //Abort Command Confirmation

    //Loop
    while(true)
    {
      if(Serial.available() != 0)             //Check for user input
      {
        if(Serial.read() == 'A')              //Abort command
        {
          Serial.print("Abort command detected. Please enter A to verify. | ");
          while(Serial.available() == 0);
          if(Serial.read() == 'A')
          {
            Serial.print("Abort command verified. Abort in progress. | ");
            for(char i = 0; i < 255; i++)
            {
              Serial2.print("CMDA");
              delay(5);
            }
            break;
          }
          else
          {
            Serial.print("Abort command cancelled. | ")
          }
        }
        else if(Serial.read() == 'F')         //Flush Serial
        {
          flush_serial_ports();
          Serial.print("Serial ports flushed. | ");
        }
        else
        {
          Serial.print("Error 1001 (irregular input). | ")
        }
      }
      else if(Serial2.available() >= 5)       //Check for data package from module
      {
        data_handling();
      }
    }
  }
}

void loop() {}

char handshake() //Returning 1 if successful, 0 else
{
  flush_serial_ports();
  Serial.print("Initialising handshake procedure. | ");
  for(char i = 1; i <= 20; i++)
  {
    //Transmitting handshake package
    Serial2.print("CMDH");
    //Waiting for handshake acknowledgement
    if(serial2_wait(5000000 != 0))
    {
      while(serial2_wait(600) != 0)
      {
        inByte = Serial2.read();
        if(inByte == '#')
        {
          Serial.print("Handshake succeeded in try " + String(i) + ". | ");
          flush_serial_ports();
          return 1;
        }
        else
        {
          Serial.print("Error H1 (Wrong acknowledgement received (" + inByte + ")). Trying again... | ");
        }
      }
    }
    else
    {
      Serial.print("Error H2 (No acknowledgement received). Trying again... | ");
    }
  }
  Serial.print("Error H3 (Handshake failed finally). | ");
  flush_serial_ports();
  return 0;
}

char powerup() //Returning 1 if successful, 0 else
{
  flush_serial_ports();
  Serial.print("Initialising powerup procedure. | ");
  for(char i = 1; i <= 20; i++)
  {
    //Transmitting powerup package
    Serial2.print("CMDP");
    //Waiting for powerup acknowledgement
    if(serial2_wait(5000000 != 0))
    {
      while(serial2_wait(600) != 0)
      {
        inByte = Serial2.read();
        if(inByte == '#')
        {
          Serial.print("Powerup succeeded in try " + String(i) + ". Waiting for incoming data package... | ");
          flush_serial_ports();
          return 1;
        }
        else
        {
          Serial.print("Error P1 (Wrong acknowledgement received (" + inByte + ")). Trying again... | ");
        }
      }
    }
    else
    {
      Serial.print("Error P2 (No acknowledgement received). Trying again... | ");
    }
  }
  Serial.print("Error P3 (Powerup failed finally). | ");
  flush_serial_ports();
  return 0;
}

void reference_coordinates() //Incoming GPS without (e.g. 50111111 statt 50.111111)
{
  lat_ref = 0;
  long_ref = 0;
  //Buffer for coordinate input. 21 bytes, because: max. 2 x 9 bytes coordinates (1 byte for sign, 2 bytes for integer places, 6 bytes for decimal places) + 1 byte ';' + RSSI + '\0' at the end
  char coordBuffer[21] = {0};
  char i;
  //Buffering coordinate input
  for(i = 0; i < 21 && serial2_wait(600) != 0; i++)
  {
    coordBuffer[i] = Serial2.read();
  }
  //Delete RSSI (last element in coordBuffer[])
  coordBuffer[i] = 0;
  char sign = 1;
  //Converting first part into latitude
  for(i = 0; i < 9 && coordBuffer[i] != ';'; i++)
  {
    if(coordBuffer[i] == '-')
    {
      sign = -1;
    }
    else
    {
      lat_ref *= 10;
      lat_ref += sign * (coordBuffer[i] - '0');
    }
  }
  sign = 1;
  //Converting second part into longitude
  for(i = i + 1; i < 19 && coordBuffer[i] != 0; i++)
  {
    if(coordBuffer[i] == '-')
    {
      sign = -1;
    }
    else
    {
      long_ref *= 10;
      long_ref += sign * (coordBuffer[i] - '0');
    }
  }
}

char flightmode()
{
  flush_serial_ports();
  Serial.print("Initialising flightmode procedure. | ");
  for(char i = 1; i <= 20; i++)
  {
    //Transmitting flightmode package
    Serial2.print("CMDD");
    //Waiting for flightmode acknowledgement
    if(serial2_wait(5000000 != 0))
    {
      while(serial2_wait(600) != 0)
      {
        inByte = Serial2.read();
        if(inByte == '#')
        {
          Serial.print("Flightmode initialisation succeeded in try " + String(i) + ". | ");
          flush_serial_ports();
          return 1;
        }
        else
        {
          Serial.print("Error D1 (Wrong acknowledgement received (" + inByte + ")). Trying again... | ");
        }
      }
    }
    else
    {
      Serial.print("Error D2 (No acknowledgement received). Trying again... | ");
    }
  }
  Serial.print("Error D3 (Flightmode initialisation failed finally). | ");
  flush_serial_ports();
  return 0;
}

//Serial functions

char check_serial_ports(char condition = 0)
{
  while(true)
  {
    while(Serial.available() == 0 && Serial2.available() == 0);
    if(Serial2.available())
    {
      return 2;
    }
    char input = Serial.read();
    switch (input)
    case condition:
      return 1;
    case 'E':
      sending_mode();
      break;
    case 'C':
      configuration();
      break;
    case 'M':
      memory_reset();
      break;
    case 'N':
      memory_configuration();
      break;
    case 'R':
      reset();
      break;
    case 'S':
      rssi_reading();
      break;
    case 'U':
      temperature_reading();
      break;
    case 'V':
      voltage_reading();
      break;
    case '0':
      non_volatile_memory_reading();
      break;
    default:
      Serial.print("Error 2001 (irregular input). | ");
      break;
    Serial.print("Enter " + condition + " to continue the setup process. | ");
  }
}

void flush_serial_ports()
{
  while(Serial.available() != 0)
  {
    Serial.read();
  }
  while(Serial2.available() != 0)
  {
    Serial2.read();
  }
}

void data_handling()
{
  //Buffering the input
  for(char i = 0; i < 5; i++)
  {
    inBuffer[i] = Serial2.read();
  }

  //Parity
  unsigned int parity = inBuffer[0] + (inBuffer[1] << 8) + (inBuffer[2] << 16) + (inBuffer[3] << 24);
  parity ^= parity >> 16;
  parity ^= parity >> 8;
  parity ^= parity >> 4;
  parity ^= parity >> 2;
  parity ^= parity >> 1;
  String s_parity = String(parity & 1);

  //Status
  String s_status = String((inBuffer[1] & 0x0E) >> 1);

  //Height
  String s_height = String(inBuffer[0] * delta_h_min);

  //GNSS/Coordinates
  unsigned int lat_in = ((inBuffer[1] & 0xF0) >> 4) + ((inBuffer[2] & 0x3F) << 4);
	unsigned int long_in = ((inBuffer[2] & 0xC0) >> 6) + ((inBuffer[3] & 0xFF) << 2);
	String s_latitude = String((lat_in * delta_d_min - (float)delta_r_max) / 0.1111949266 + lat_ref);
	String s_longitude = String((long_in * delta_d_min - (float)delta_r_max) / 0.1111949266 + long_ref);

  //RSSI
  String s_rssi = String((int)(-0.5*(float)inBufferf[4]));

  //Output
  Serial.print('\n' + s_parity + ';' + s_status + ';' + s_height + ';' + s_latitude + ';' + s_longitude + ';' + s_rssi);
}

//Waiting whether serial.available() == true in given time
int serial_wait(int delay_microsec)
{
  for(int i = 0; i < (delay_microsec / 10) && Serial.available() == 0; i++)
  {
    delayMicroseconds(10);
  }
  return Serial.available();
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