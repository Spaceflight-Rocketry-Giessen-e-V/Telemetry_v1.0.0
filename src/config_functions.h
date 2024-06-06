#include <Arduino.h>

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