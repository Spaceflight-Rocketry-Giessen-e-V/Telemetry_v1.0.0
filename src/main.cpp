#include <Arduino.h>

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
void reference_coordinates();
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

char flightmode_var = 0;

char inByte;

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

void non_volatile_memory_reading()
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
      char memoryBuffer[129] = {0};
      for(int i = 0; serial2_wait(5000) != 0; i++)
      {
        memoryBuffer[i] = Serial2.read();
      }
      Serial.print(" (0x00) RF Channel [4,0x04]:");
      Serial.write(memoryBuffer[0x00]);
      Serial.print(" (0x01) RF Power [5,0x05]:");
      Serial.write(memoryBuffer[0x01]);
      Serial.print(" (0x02) RF Data rate [3,0x03]:");
      Serial.write(memoryBuffer[0x02]);
      Serial.print(" (0x04) SLEEP Mode [0,0x00]:");
      Serial.write(memoryBuffer[0x04]);
      Serial.print(" (0x05) RSSI Mode [0,0x00]:");
      Serial.write(memoryBuffer[0x05]);
      Serial.print(" (0x0E) Packet length high [0,0x00]:");
      Serial.write(memoryBuffer[0x0E]);
      Serial.print(" (0x0F) Packet length low [128,0x80]:");      
      Serial.write(memoryBuffer[0x0F]);
      Serial.print(" (0x10) Packet timeout [124,0x7C]:");
      Serial.write(memoryBuffer[0x10]);
      Serial.print(" (0x11) Packet end character [0,0x00]:");
      Serial.write(memoryBuffer[0x11]);
      Serial.print(" (0x14) Address mode [2,0x02]:");
      Serial.write(memoryBuffer[0x14]);
      Serial.print(" (0x15) CRC mode [2,0x02]:");
      Serial.write(memoryBuffer[0x15]);
      Serial.print(" (0x19) Unique ID 1 [1,0x01]:");
      Serial.write(memoryBuffer[0x19]);
      Serial.print(" (0x1B) Unique ID 2 [1,0x01]:");
      Serial.write(memoryBuffer[0x1B]);
      Serial.print(" (0x1D) Unique ID 3 [1,0x01]:");
      Serial.write(memoryBuffer[0x1D]);
      Serial.print(" (0x1F) Unique ID 4 [1,0x01]:");
      Serial.write(memoryBuffer[0x1F]);
      Serial.print(" (0x1A) System ID 1 [1,0x01]:");
      Serial.write(memoryBuffer[0x1A]);
      Serial.print(" (0x1C) System ID 2 [1,0x01]:");
      Serial.write(memoryBuffer[0x1C]);
      Serial.print(" (0x1E) System ID 3 [1,0x01]:");
      Serial.write(memoryBuffer[0x1E]);
      Serial.print(" (0x20) System ID 4 [1,0x01]:");
      Serial.write(memoryBuffer[0x20]);
      Serial.print(" (0x21) Destination ID 1 [1,0x01]:");
      Serial.write(memoryBuffer[0x21]);
      Serial.print(" (0x22) Destination ID 2 [1,0x01]:");
      Serial.write(memoryBuffer[0x22]);
      Serial.print(" (0x23) Destination ID 3 [1,0x01]:");
      Serial.write(memoryBuffer[0x23]);
      Serial.print(" (0x24) Destination ID 4 [1,0x01]:");
      Serial.write(memoryBuffer[0x24]);
      Serial.print(" (0x28) Broadcast address [255,0xFF]:");
      Serial.write(memoryBuffer[0x28]);
      Serial.print(" (0x30) UART baud rate [5,0x05]:");
      Serial.write(memoryBuffer[0x30]);
      Serial.print(" (0x31) UART number of bits [8,0x08]:");
      Serial.write(memoryBuffer[0x31]);
      Serial.print(" (0x32) UART parity [0,0x00]:");
      Serial.write(memoryBuffer[0x32]);
      Serial.print(" (0x33) UART stop bits [1,0x01]:");
      Serial.write(memoryBuffer[0x33]);
      Serial.print(" (0x35) UART flow control [0,0x00]:");
      Serial.write(memoryBuffer[0x35]);
      Serial.print(" (0x3C - 0x49) Part number:");
      for(int i = 0x3C; i <= 0x49; i++)
      {
        Serial.write(memoryBuffer[i]);
      }
      Serial.print(" (0x4B - 0x4E) Hardware revision number:");
      for(int i = 0x4B; i <= 0x4E; i++)
      {
        Serial.write(memoryBuffer[i]);
      }
      Serial.print(" (0x50 - 0x53) Software revision number:");
      for(int i = 0x50; i <= 0x53; i++)
      {
        Serial.write(memoryBuffer[i]);
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
    flush_serial();
    flush_serial2();
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
    if(flightmode_var == 0 && Serial2.available() != 0)
    {
      Serial.write(Serial2.read());
    }

    //Checking for data package over radio in flightmode
    else if(flightmode_var == 1 && Serial2.available() >= 5)
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
    flightmode_var = 0;
    Serial.print("Flightmode FALSE | ");
  }
  else
  {
    Serial.print("Abort command cancelled. | ");
  }
}

void flightmode()
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
      flightmode_var = 1;
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

void poweron()
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
    Serial.print("Power on command cancelled. | ");
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
  }
  else
  {
    Serial.print("Parachute deployment cancelled. | ");
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
  int i;
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
  Serial.print("Successfully transmitted");
  Serial.print(sendingBuffer);
  Serial.print(". | ");
  flush_serial();
}

void data_handling()
{
  //Buffering the input
  char inBuffer[5] = {0};
  for(int i = 0; i < 5; i++)
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
  String s_rssi = String((int)(-0.5*(float)inBuffer[4]));

  //Output
  Serial.print('\n' + ';' + s_parity + ';' + s_status + ';' + s_height + ';' + s_latitude + ';' + s_longitude + ';' + s_rssi + ';');
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
  flush_serial();
  flush_serial2();
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
