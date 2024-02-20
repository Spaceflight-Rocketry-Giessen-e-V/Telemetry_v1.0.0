#include <Arduino.h>

//Teensy 4.0 ist 32 bit Plattform!
//RSSI in data handling weil rssi byte angehängt
//test 0 empfangen und verarbeiten
//Sending mode

unsigned const int delta_r_max = 3000;	    //Max. darstellbare Entfernung vom Startort in m
unsigned const char delta_d_min = 6;	      //Min. darstellbare Auflösung des Standorts in m

unsigned const char height_resolution = 5;  //Min. darstellbare Auflösung der Höhe in m

signed int lat_zero = 0;
signed int long_zero = 0;

char inBuffer[5] = {0};

int serial_wait(int delay_microsec);
int serial2_wait(int delay_microsec);

void data_handling();

void array_to_coord(char* input, signed long int lat_zero, signed long int long_zero, signed long int *lat_out, signed long int *long_out);
int array_to_height(char* input);
char array_to_status(char* input);
char array_parity_check(char* input);
char array_to_rssi(char* input);

//Pin allocation
int ledpin1 = 0;
int ledpin2 = 1;
int ledpin3 = 2;
int ledpin4 = 13;
int rstpin = 3;
int cfgpin = 4;
int rtspin = 5;

void configuration();
void memory_reset();
void memory_configuration();
void reset();
void rssi_reading();
void temperature_reading();
void voltage_reading();
void non_volatile_memory_reading();

void 


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
      if(setup_user_input('H') == 1)
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
    char powerup = 0;
    while(powerup == 0)
    {
      Serial.print("Enter P to initiate power up command. | ");
      if(setup_user_input('P') == 1)
      {
        Serial2.print("CMDP");
        if(serial2_wait(5000000)!=0)
        {
          if(Serial2.read() == 'P')
          {
            Serial.print("Power up succeeded. Waiting for incoming data package... | ");
            powerup = 1;
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

  //Waiting for incoming zero coordinates

  //Incoming data package
  {
    if(setup_user_input() == 0)
    {
      delay_microsec(2000); // 3 * 600 + puffer
      Serial.write('\n');
      if(Serial2.available() >= 5)
      {
        data_handling();
      }
    }
  }

  //Flight mode command


  
 
 
}

void loop() 
{
  
  if(Serial.available() != 0)             //Check for user input
  {
    if(Serial.read() == 'A')              //Abort command
    {
      Serial.print("Abort command detected. Please enter A to verify. | ");
      while(Serial.available() == 0);
      if(Serial.read() == 'A')
      {
        Serial.print("Abort command verified. Trying to abort. | ");
        for(char i = 0; i < 255; i++)
        {
          Serial2.print("CMDA");
          delay(5);
        }
      }
      else
      {
        Serial.print("Abort command cancelled. | ")
      }
    }
    else if(Serial.read() == 'F')         //Flush Serial
    {
      while(Serial2.available() != 0)
      {
        Serial2.read();
      }
      Serial.print("Serial2 flushed. | ");
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

char setup_user_input(char condition = 0)
{
  while(true)
  {
    while(Serial.available() == 0 && Serial2.available() == 0);
    if(Serial2.available())
    {
      return 0;
    }
    char input = Serial.read();
    if(input == condition)
    {
      return 1;
    }
    else if(input == 'C')
    {
      configuration();
    }
    else if(input == 'M')
    {
      memory_reset();
    }
    else if(input == 'N')
    {
      memory_configuration();
    }
    else if(input == 'R')
    {
      reset();
    }
    else if(input == 'S')
    {
      rssi_reading();
    }
    else if(input == 'U')
    {
      temperature_reading();
    }
    else if(input == 'V')
    {
      voltage_reading();
    }
    else if(input == '0')
    {
      non_volatile_memory_reading();
    }
    else
    {
      Serial.print("Error 2001 (irregular input). | ");
    }
    Serial.print("Enter " + condition + " to continue the setup process. | ");
  }
}

void configuration()
{
  digitalWrite(cfgpin, LOW);
  serial2_wait(5000);
  digitalWrite(cfgpin, HIGH);
  //Check for regular feedback from module
  if(Serial2.available() == 1 && Serial2.read() == '>')
  {
    Serial.print("Entered configuration mode. Enter the desired command. Enter X to exit. ");
    Serial.write('>');
    //Entering communication loop
    while(true)
    {
      //Waiting for user input
      while(Serial.available() == 0);
      Serial2.write(Serial.read());
      //Waiting for module response
      serial2_wait(5000);
      if(Serial2.available() == 0)
      {
        break;
      }
      else
      {
        while(serial2_wait(5000) != 0)
        {
          Serial.write(Serial2.read());
        }
      }
    }
    Serial.print("Configuration finished. Entering normal operation mode. | ");
  }
  else
  {
    Serial2.write('X');
    Serial.print("Error 2201 (received either no or irregular response from module). Entering normal operation mode. | ");
  }
}

void memory_reset()
{
  digitalWrite(cfgpin, LOW);
  serial2_wait(5000);
  digitalWrite(cfgpin, HIGH);
  //Check for feedback from module
  if(Serial2.available() == 1 && Serial2.read() == '>')
  {
    Serial2.print("@RC");
    serial2_wait(5000);
    if(Serial2.available() == 1 && Serial2.read() == '>')
    {
      Serial.print("Memory reset complete. Entering normal operation mode. | ");
    }
    else
    {
      Serial.print("Error 2322 (received either no or irregular response from module). Entering normal operation mode. | ");
    }
  }
  else
  {
    Serial.print("Error 2321 (received either no or irregular response from module). Entering normal operation mode. | ");
  }
  Serial2.write('X');
}

void memory_configuration()
{
  digitalWrite(cfgpin, LOW);
  serial2_wait(5000);
  digitalWrite(cfgpin, HIGH);
  //Check for feedback from module
  if(Serial2.available() == 1 && Serial2.read() == '>')
  {
    Serial2.write('M');
    serial2_wait(5000);
    if(Serial2.available() == 1 && Serial2.read() == '>')
    {
      Serial.print("Entered memory configuration. Enter the desired address. Enter X to exit. ");
      if(serial_wait(20000000) != 0)
      {
        inByte = Serial.read();
        if(inByte == 'X')
        {
          Serial.print("Exiting memory configuration. Entering normal operation mode. | ");
        }
        else if(inByte <= 0x35)
        {
          Serial2.write(inByte);
          Serial.print("Enter the desired value. ");
          if(serial_wait(20000000) != 0)
          {
            inByte = Serial.read();
            if(inByte <= 0x35)
            {
              Serial2.write(inByte);
              Serial.print("Finished memory configuration. Entering normal operation mode. | ");
            }
            else 
            {
              Serial.print("Error 2334 (received irregular input from user). Entering normal operation mode. | ");
            }
          }
        }
        else
        {
          Serial.print("Error 2333 (received irregular input from user). Entering normal operation mode. | ");
        }
      }
      Serial2.write(0xFF);
      while(serial2_wait(5000))
      {
        Serial2.read();
      }
    }
    else
    {
      Serial.print("Error 2332 (received either no or irregular response from module). Entering normal operation mode. | ");
    }
  }
  else
  {
    Serial.print("Error 2331 (received either no or irregular response from module). Entering normal operation mode. | ");
  }
  Serial2.write('X');
}

void reset()
{
  digitalWrite(rstpin, LOW);
  delay(50);
  digitalWrite(rstpin, HIGH);
  delay(50);
  Serial.print("Reset complete. Entering normal operation mode. | ");
}

void rssi_reading()
{
  digitalWrite(cfgpin, LOW);
  serial2_wait(5000);
  digitalWrite(cfgpin, HIGH);
  //Check for feedback from module
  if(Serial2.available() == 1 && Serial2.read() == '>')
  {
    Serial2.print('S');
    serial2_wait(5000);
    if(Serial2.available() != 0)
    { 
      inByte = Serial2.read();
      Serial.print("RSSI-Reading:");
      Serial.write(inByte);
      Serial.print(" Signal Strength:");
      Serial.print(-0.5 * (float)inByte, DEC);
      Serial.print(" RSSI-Reading finished. Entering normal operation mode. | ");
    }
    else
    {
      Serial.print("\nError 2352 (received no response from module). Entering normal operation mode. | ");
    }
  }
}

void temperature_reading()
{
  digitalWrite(cfgpin, LOW);
  serial2_wait(5000);
  digitalWrite(cfgpin, HIGH);
  //Check for feedback from module
  if(Serial2.available() == 1 && Serial2.read() == '>')
  {
    Serial2.print('U');
    serial2_wait(5000);
    if(Serial2.available() != 0)
    {
      inByte = Serial2.read();
      Serial.print("TEMP-Reading:");
      Serial.write(inByte);
      Serial.print(" Temperature:");
      Serial.write(inByte - 128);
      while(Serial2.available())
      {
        Serial2.read();
      }
      Serial.print(" Temperature-Reading finished. Entering normal operation mode. | ");
    }
    else
    {
      Serial.print("Error 2362 (received no response from module). Entering normal operation mode. | ");
    }
  }
  else
  {
      Serial.print("Error 2361 (received either no or irregular response from module). Entering normal operation mode. | ");
  }
  Serial2.write('X');
}

void voltage_reading()
{
  digitalWrite(cfgpin, LOW);
  serial2_wait(5000);
  digitalWrite(cfgpin, HIGH);
  //Check for feedback from module
  if(Serial2.available() == 1 && Serial2.read() == '>')
  {
    Serial2.print('V');
    serial2_wait(5000);
    if(Serial2.available() != 0)
    {
      inByte = Serial2.read();
      Serial.print("VCC-Reading:");
      Serial.print(inByte);
      Serial.print(" Voltage:");
      Serial.print((double)inByte * 0.03, DEC);
      while(Serial2.available())
      {
        Serial2.read();
      }
      Serial.print(" Voltage-Reading finished. Entering normal operation mode. | ");
    }
    else
    {
      Serial.print("Error 2372 (received no response from module). Entering normal operation mode. | ");
    }
  }
  else
  {
    Serial.print("Error 2371 (received either no or irregular response from module). Entering normal operation mode. | ");
  }
  Serial2.write('X');
}

void non_volatile_memory_reading()^
{
  digitalWrite(cfgpin, LOW);
  serial2_wait(5000);
  digitalWrite(cfgpin, HIGH);
  //Check for feedback from module
  Serial.write(Serial2.available());
  if(Serial2.available() == 1 && Serial2.read() == '>')
  {
    Serial2.write('0');
    serial2_wait(5000);
    if(Serial2.available() != 0)
    {
      char inBuffer[129] = {0};
      for(int i = 0; serial2_wait(5000) != 0; i++)
      {
        inBuffer[i] = Serial2.read();
      }
      Serial.print(" (0x00) RF Channel [4,0x04]:");
      Serial.write(inBuffer[0x00]);
      Serial.print(" (0x01) RF Power [5,0x05]:");
      Serial.write(inBuffer[0x01]);
      Serial.print(" (0x02) RF Data rate [3,0x03]:");
      Serial.write(inBuffer[0x02]);
      Serial.print(" (0x04) SLEEP Mode [0,0x00]:");
      Serial.write(inBuffer[0x04]);
      Serial.print(" (0x05) RSSI Mode [0,0x00]:");
      Serial.write(inBuffer[0x05]);
      Serial.print(" (0x0E) Packet length high [0,0x00]:");
      Serial.write(inBuffer[0x0E]);
      Serial.print(" (0x0F) Packet length low [128,0x80]:");      
      Serial.write(inBuffer[0x0F]);
      Serial.print(" (0x10) Packet timeout [124,0x7C]:");
      Serial.write(inBuffer[0x10]);
      Serial.print(" (0x11) Packet end character [0,0x00]:");
      Serial.write(inBuffer[0x11]);
      Serial.print(" (0x14) Address mode [2,0x02]:");
      Serial.write(inBuffer[0x14]);
      Serial.print(" (0x15) CRC mode [2,0x02]:");
      Serial.write(inBuffer[0x15]);
      Serial.print(" (0x19) Unique ID 1 [1,0x01]:");
      Serial.write(inBuffer[0x19]);
      Serial.print(" (0x1B) Unique ID 2 [1,0x01]:");
      Serial.write(inBuffer[0x1B]);
      Serial.print(" (0x1D) Unique ID 3 [1,0x01]:");
      Serial.write(inBuffer[0x1D]);
      Serial.print(" (0x1F) Unique ID 4 [1,0x01]:");
      Serial.write(inBuffer[0x1F]);
      Serial.print(" (0x1A) System ID 1 [1,0x01]:");
      Serial.write(inBuffer[0x1A]);
      Serial.print(" (0x1C) System ID 2 [1,0x01]:");
      Serial.write(inBuffer[0x1C]);
      Serial.print(" (0x1E) System ID 3 [1,0x01]:");
      Serial.write(inBuffer[0x1E]);
      Serial.print(" (0x20) System ID 4 [1,0x01]:");
      Serial.write(inBuffer[0x20]);
      Serial.print(" (0x21) Destination ID 1 [1,0x01]:");
      Serial.write(inBuffer[0x21]);
      Serial.print(" (0x22) Destination ID 2 [1,0x01]:");
      Serial.write(inBuffer[0x22]);
      Serial.print(" (0x23) Destination ID 3 [1,0x01]:");
      Serial.write(inBuffer[0x23]);
      Serial.print(" (0x24) Destination ID 4 [1,0x01]:");
      Serial.write(inBuffer[0x24]);
      Serial.print(" (0x28) Broadcast address [255,0xFF]:");
      Serial.write(inBuffer[0x28]);
      Serial.print(" (0x30) UART baud rate [5,0x05]:");
      Serial.write(inBuffer[0x30]);
      Serial.print(" (0x31) UART number of bits [8,0x08]:");
      Serial.write(inBuffer[0x31]);
      Serial.print(" (0x32) UART parity [0,0x00]:");
      Serial.write(inBuffer[0x32]);
      Serial.print(" (0x33) UART stop bits [1,0x01]:");
      Serial.write(inBuffer[0x33]);
      Serial.print(" (0x35) UART flow control [0,0x00]:");
      Serial.write(inBuffer[0x35]);
      Serial.print(" (0x3C - 0x49) Part number:");
      for(int i = 0x3C; i <= 0x49; i++)
      {
        Serial.write(inBuffer[i]);
      }
      Serial.print(" (0x4B - 0x4E) Hardware revision number:");
      for(int i = 0x4B; i <= 0x4E; i++)
      {
        Serial.write(inBuffer[i]);
      }
      Serial.print(" (0x50 - 0x53) Software revision number:");
      for(int i = 0x50; i <= 0x53; i++)
      {
        Serial.write(inBuffer[i]);
      }
      Serial.print("Finished. Returning to normal operation mode. | ");
    }
    else
    {
      Serial.print("Error 2392 (received no response from module). Entering normal operation mode. | ");
    }
  }
  else
  {
    Serial.print("Error 2391 (received either no or irregular response from module). Entering normal operation mode. | ");
  }
  Serial2.write('X');
}

void data_handling()
{
  for(char i = 0; i < 5; i++)
  {
    inBuffer[i] = Serial2.read();
  }

  signed int latitude = 0;
  signed int longitude = 0;

  array_to_coord(inBuffer, lat_zero, long_zero, &latitude, &longitude);

  String s_parity = String(array_parity_check());
  String s_status = String(array_to_status());
  String s_height = String(array_to_height());
  String s_latitude = String(latitude);
  String s_longitude = String(longitude);
  String s_rssi = String(array_to_rssi());

  Serial.print(s_parity + ';' + s_status + ';' + s_height + ';' + s_latitude + ';' + s_longitude + ';' + s_rssi + '\n');
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

char array_to_status(char* input)
{
  return input[4];
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