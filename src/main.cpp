#include <Arduino.h>

//Teensy 4.0 ist 32 bit Plattform!

unsigned const int delta_r_max = 3000;	    //Max. darstellbare Entfernung vom Startort in m
unsigned const char delta_d_min = 6;	      //Min. darstellbare Auflösung des Standorts in m

unsigned const char height_resolution = 5;  //Min. darstellbare Auflösung der Höhe in m

signed int lat_zero = 0;
signed int long_zero = 0;

char inBuffer[4] = {0};

int serial_wait(int delay_microsec);
int serial2_wait(int delay_microsec);

void data_handling();

void array_to_coord(char* input, signed long int lat_zero, signed long int long_zero, signed long int *lat_out, signed long int *long_out);
int array_to_height(char* input);
char array_to_status(char* input);
char array_parity_check(char* input);


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
    while(serial_wait(5000000) == 0)
    {
      Serial.print("Enter any character to continue. | ");
    }
    while(Serial.available() != 0)
    {
      Serial.read();
    }
    while(Serial2.available() != 0)
    {
      Serial2.read();
    }
    Serial.print("Setup complete. Entering normal operation mode. | ");
  }
  
  //Handshake protocol
  {
    char handshake = 0;
    while(handshake == 0)
    {
      Serial.print("Enter H to initiate handshake protocol. | ");
      while(Serial.available() == 0);
      if(Serial.read() == 'H')
      {
        char[5] handshake_array = {'H', 0, 'H', 0, 0};
        for(char i = 1; i < 255; i++)
        {
          handshake_array[1] = i;
          handshake_array[3] = i;
          Serial2.print(handshake_array);
          if(serial2_wait(5000000 == 0))
          {
            Serial.print("Handshake failed. Trying again... | ");
          }
          else
          {
            char[5] inBuffer = {0};
            for(char j = 0; j < 4 && serial2_wait(600); j++)
            {
              inBuffer[i] = Serial2.read();
            }
            if(inBuffer[0] == 'H' && inBuffer[0] == 'H')
            {
              Serial.print("Handshake succeeded. Own handshake code: ");
              Serial.print({i,i});
              Serial.print(" Received handshake code: ");
              Serial.print({inBuffer[1], inBuffer[3]});
              Serial.print(" | ");
              handshake = 1;
              break;
            }
            else
            {
              Serial.print("Handshake failed. Trying again... | ");
            }
          }
        }
      }
    }
    while(Serial.available() != 0)
    {
      Serial.read();
    }
    while(Serial2.available() != 0)
    {
      Serial2.read();
    }
  }

  //Clock synchronization

  //Power up command
  {
    while(true)
    {
      Serial.print("Enter P to initiate power up command. | ");
      while(Serial.available() == 0);
      if(Serial.read() == 'P')
      {
        Serial2.print("CMDP");
        Serial.print("Power up command sent. Waiting for incoming data package... | ");
        break;
      }
    }
    while(Serial.available() != 0)
    {
      Serial.read();
    }
    while(Serial2.available() != 0)
    {
      Serial2.read();
    }
  }

  //Waiting for incoming zero coordinates

  //Incoming data package
  {
    while(Serial2.available() == 0);
    delay_microsec(2000); // 3 * 600 + puffer
    if(Serial2.available() >= 4)
    {
      data_handling();
    }
  }

  //Flight mode command


  
 
 
}

void loop() 
{
  
}


void data_handling()
{
  for(char i = 0; i < 4; i++)
  {
    inBuffer[i] = Serial2.read();
  }

  signed int latitude = 0;
  signed int longitude = 0;

  array_to_coord(inBuffer, lat_zero, long_zero, &latitude, &longitude);

  String s_parity = String(array_parity_check);
  String s_status = String(array_to_status);
  String s_height = String(height);
  String s_latitude = String(latitude);
  String s_longitude = String(longitude);

  Serial.print(s_parity + ';' + s_status + ';' + s_height + ';' + s_latitude + ';' + s_longitude + '\n');
}

void array_to_coord(char* input, signed int lat_zero, signed int long_zero, signed int *lat_out, signed int *long_out)
{
	unsigned int lat_in = ((input[1] & 0xF0) >> 4) + ((input[2] & 0x3F) << 4);
	unsigned int long_in = ((input[2] & 0xC0) >> 6) + ((input[3] & 0xFF) << 2);

	*lat_out = (lat_in * delta_d_min - (float)delta_r_max) / 0.1111949266 + lat_zero;
	*long_out = (long_in * delta_d_min - (float)delta_r_max) / 0.1111949266 + long_zero;
}

int array_to_height(char* input);
{
  return input[0] * height_resolution;
}

char array_to_status(char* input)
{
  return ((input[1] & 0x0E) >> 1);
}

char array_parity_check(char* input)
{
  unsigned int array = input[0] + (input[1] << 8) + (input[2] << 16) + (input[3] << 24);

  array ^= array >> 16;    // XOR der oberen und unteren 16 Bits
  array ^= array >> 8;  // XOR der oberen und unteren 8 Bits
  array ^= array >> 4;  // XOR der oberen und unteren 4 Bits
  array ^= array >> 2;  // XOR der oberen und unteren 2 Bits
  array ^= array >> 1;  // XOR des MSB und LSB

  // Das letzte Bit enthält die Parität (1 für ungerade, 0 für gerade)
  return array & 1;
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