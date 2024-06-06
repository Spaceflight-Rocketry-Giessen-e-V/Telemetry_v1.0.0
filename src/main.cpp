#include <Arduino.h>
#include "config_functions.h"

unsigned const int delta_r_max = 3000;	    //Max. darstellbare Entfernung vom Startort in m
unsigned const char delta_d_min = 6;	      //Min. darstellbare Auflösung des Standorts in m
unsigned const char delta_h_min = 5;        //Min. darstellbare Auflösung der Höhe in m

signed int lat_ref = 0;
signed int long_ref = 0;

void abort();
void flightmode();
void handshake();
void poweron();
void parachute();
void reference_coordinates()
void sending_mode();

void data_handling();
char user_commands();
void flush_serial();
void flush_serial2();
int serial_wait(int delay_microsec);
int serial2_wait(int delay_microsec);

//Pin allocation
int ledpin1 = 0;
int ledpin2 = 1;
int ledpin3 = 2;
int ledpin4 = 13;
int rstpin = 3;
int cfgpin = 4;
int rtspin = 5;

char flightmode = 0;

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

  //Standard operation loop
  while(true)
  {
    //Checking for user input
    if(Serial.available() != 0)
    {
      switch(user_commands())
      {
        //Abort
        case 'A':
          abort();
          flush_serial();
          flush_serial2();
          break;

        //Flush serial
        case 'B':
          flush_serial2();
          Serial.print("Serial2 successfully flushed. | ");
          break;

        //Flight mode
        case 'F':
          flightmode();
          break;  

        //Handshake
        case 'H':
          handshake();
          break;

        //Power on
        case 'O':
          poweron();
          flush_serial();
          flush_serial2();
          break;

        //Parachute deployment
        case 'P':
          parachute();
          flush_serial();
          flush_serial2();
          break;

        //Reference coordinates
        case 'R':
          reference_coordinates();
          break;

        //Sending mode
        case 'S':
          sending_mode();
          break;

        //Invalid input
        default:
          Serial.print("Error 4321 (invalid input). | ");
      }
    }

    //Checking for data package over radio in standby
    if(flightmode == 0 && Serial2.available() != 0)
    {
      Serial.write(Serial2.read());
    }

    //Checking for data package over radio in flightmode
    else if(flightmode == 1 && Serial2.available() >= 5)
    {
      data_handling();
    }
  }
}

void loop() {}

void abort()
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
    Serial.print("CMDA | ");
    flightmode = 0;
    Serial.print("Flightmode FALSE | ");
    break;
  }
  else
  {
    Serial.print("Abort command cancelled. | ")
  }
}

char flightmode()
{
  flush_serial2();
  Serial2.print("CMDF");
  Serial.print("CMDF | ");
  if(serial2_wait(1000000) != 0)
  {
    char input = Serial2.read();
    Serial.print(Serial2.read() + " | ");
    if(input == '#')
    {
      flightmode = 1;
      Serial.print("Flightmode TRUE | ");
    }
  }
  else
  {
    Serial.print("No answer received. | ");
  }
  flush_serial();
  flush_serial2();
}

void handshake()
{
  flush_serial2();
  Serial2.print("CMDH");
  Serial.print("CMDH | ");
  if(serial2_wait(1000000) != 0)
  {
    Serial.print(Serial2.read() + " | ");
  }
  else
  {
    Serial.print("No answer received. | ");
  }
  flush_serial();
  flush_serial2();
}

char poweron()
{
  Serial.print("Power on command detected. Please enter O to verify. | ");
  while(Serial.available() == 0);
  if(Serial.read() == 'O')
  {
    flush_serial2();
    Serial2.print("CMDO");
    Serial.print("CMDO | ");
    if(serial2_wait(1000000) != 0)
    {
      Serial.print(Serial2.read() + " | ");
    }
    else
    {
      Serial.print("No answer received. | ");
    }
  }
  else
  {
    Serial.print("Power on command cancelled. | ")
  }
}

void parachute()
{
  Serial.print("Parachute command detected. Please enter P to verify. | ");
  while(Serial.available() == 0);
  if(Serial.read() == 'B')
  {
    Serial.print("Parachute command verified. Parachute deployment in progress. | ");
    for(char i = 0; i < 255; i++)
    {
      Serial2.print("CMDP");
      delay(5);
    }
    Serial.print("CMDP | ");
    break;
  }
  else
  {
    Serial.print("Parachute deployment cancelled. | ")
  }
}

void reference_coordinates() //Incoming GPS without dot (e.g. 50111111 statt 50.111111)
{
  flush_serial2();
  Serial2.print("CMDR");
  Serial.print("CMDR | ");
  if(serial2_wait(2000000) == 0)
  {
    Serial.print("Error 1234 (No answer received). | ");
    return;
  }
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
  Serial.print(String(lat_ref) + " | " + String(long_ref) + " | ");
}

void sending_mode()
{
  Serial.print("Entered sending mode. Enter the string to be transmitted (max. 4 bytes). | ");
  char sendingBuffer[5] = {0};
  while(Serial.available() == 0);
  for(int i = 0; i < 4 && serial_wait(600); i++)
  {
    sendingBuffer[i] = Serial.read();
  }
  Serial2.print(sendingBuffer);
  Serial.print("Successfully transmitted " + sendingBuffer + ". | ");
  flush_serial();
}

void data_handling()
{
  //Buffering the input
  char inBuffer[5] = {0};
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
  Serial.print('\n' + s_parity + ';' + s_status + ';' + s_height + ';' + s_latitude + ';' + s_longitude + ';' + s_rssi + ';');
}

char user_commands()
{
  char cmd_buffer = 0;
  if(Serial.available() != 0)
  {
    cmd_buffer = Serial.read();
  }
  switch(cmd_buffer)
  {
    case 'C':
      configuration();
      break;
    case 'M':
      memory_reset();
      break;
    case 'N':
      memory_configuration();
      break;
    case 'D':
      reset();
      break;
    case 'T':
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
      return cmd_buffer;
  }
  flush_serial_ports();
  return 0;
}

void flush_serial()
{
  while(Serial.available() != 0)
  {
    Serial.read();
  }
}

void flush_serial2()
{
  while(Serial2.available() != 0)
  {
    Serial2.read();
  }
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
