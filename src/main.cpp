#include <avr/io.h>

int serial0_wait(int delay_microsec);
int serial1_wait(int delay_microsec);
int serial2_wait(int delay_microsec);

void flush_serial_ports();
char radio_commands();
void data_handling();
void reference_coordinates();

const char package_length_max = 192;        //max package length possible with 38400 baud
const char package_length_min = 64;         //min package length possible
char data_package[package_length_max] = {0};

unsigned const int delta_r_max = 3000;	    //max displayable distance from starting point in m
unsigned const char delta_d_min = 6;	      //min displayable resolution of current location in m
unsigned const char delta_h_min = 5;        //min displayable resolution of current height in m

unsigned long int lat_ref = 0;
unsigned long int long_ref = 0;

char flightmode = 0;

void setup() 
{
  //Pin initialisation (A7 (1): powerpin, C2 (4): statuspin1, C3 (5): statuspin2, D2 (8): parachutepin, D3 (9): ledpin1, D4 (10): ledpin2, D5 (11): modepin, D6 (12): cfgpin, D7 (13): rstpin)
  PORTA.DIRSET = PIN7_bm;
  PORTC.DIRCLR = PIN2_bm | PIN3_bm;
  PORTD.DIRCLR = PIN5_bm;
  PORTD.DIRSET = PIN2_bm | PIN3_bm | PIN4_bm | PIN6_bm | PIN7_bm;

  //Pin standard mode
  PORTA.OUTCLR = PIN7_bm;
  PORTA.OUTCLR = PIN2_bm | PIN3_bm | PIN4_bm;
  PORTD.OUTSET = PIN6_bm | PIN7_bm;

  //MISSING: Storage cell initialisation
  //MISSING: SD card initialisation

  //Data mode
  if(PORTD.IN & 0x10) //Reading 5th bit in PORTD (modepin) (try using bitmaps?)
  {
    PORTD.OUTSET = PIN3_bm; //ledpin1
    Serial2.begin(19200); //External serial connection
    //MISSING: Flash Chip auslesen, Programmieren, Radiomodul konfigurieren
  }

  //Normal operation mode
  else
  {
    PORTD.OUTSET = PIN4_bm; //ledpin2
    //Serial configuration
    Serial0.begin(19200); //Radio module
    Serial1.begin(19200); //Sensory subsystem

    unsigned int standby_counter = 0; //For sending regular standby messages

    //Standard operation loop
    while(true)
    {
      //Checking for 4 incoming bytes over radio (all command from the ground station are 4 bytes ("CMDx"))
      if(Serial0.available() >= 4)
      {
        switch(radio_commands())
        {
          //Handshake
          case 'H':
            Serial0.write('#'); //'#' referring to confirmation
            //MISSING: Writing in log file
            break;

          //Power on
          case 'O':
            PORTA.OUTSET = PIN7_bm //powerpin
            Serial0.write('#');
            //MISSING: Writing in log file
            break;

          //Reference Coordinates
          case 'R':
            reference_coordinates();
            //MISSING: Writing in log file
            break;

          //Flight mode
          case 'F':
            flightmode = 1;
            Serial0.write('#');
            //MISSING: Writing in log file
            break;

          //Abort Command
          case 'A':
            PORTA.OUTCLR = PIN7_bm //powerpin
            flightmode = 0;
            //MISSING: Writing in log file
            break;

          //Parachute deployment
          case 'P':
            if(flightmode == 1)
            {
              PORTD.OUTSET = PIN2_bm //parachutepin
            }
            //MISSING: Writing in log file
            break;
        }
      }

      //Checking for 64 incoming bytes from Serial1 (all data packages have at least length 64)
      if(Serial1.available() >= package_length_min)
      {
        data_handling();
      }

      //Sending standby messages
      if(flightmode == 0)
      {
        standby_counter %= 10000;
        if(standby_counter == 0)
        {
          Serial0.write('^'); //referring to confirmation of stand-by
        }
        standby_counter++;
      }
    }
  }
}

void loop() {} //if functional: Change void setup() to main() and delete void loop()

//Processing data coming from sensory subsystem
void data_handling()
{
  //Write input in global array
  int i;
  for(i = 0; i < package_length_max - 2 && serial0_wait(600) != 0; i++)
  {
    data_package[i] = Serial0.read();
  }
  data_package[i + 1] = ';';
  data_package[i + 2] = 0; //defininug end byte
	
  //Check for start and end byte
  if(data_package[0] != 'X' || data_package[i] != 'Y')
  {
    //MISSING: Write in log file
    while(serial1_wait(600) != 0) //For wrong start or end byte: flush serial input
    {
      flush_serial_ports();
    }
    return;
  }
  //Only when in flightmode, data is stored and send via radio
  if(flightmode == 1)
  {
    unsigned char output_buffer[5] = {0}; //Fifth byte = '\0' for easy Serial.print()

    //status pins are read and written in 2 bits
    unsigned char status = (PORTC.IN & 0x03) << 1 + PORTC.IN & 0x02 //Result: 00/01/10/11 //test: change hex-codes to bitmaps
    
    double data[13] = { 0 }; //13 different values are sent and will be stored in this array
    //Values are seperated and written as double in data array
    long int dot = 0;
    for (int i = 2, j = 0; j < 13; i++)
    {
      char tmp = data_package[i]; //tmp as temporary variable for copying data-package
      if (tmp == ';')
      {
        dot = 0; //0 referring to position in front of komma
        j++;
      }
      else if (j == 1 || j == 2 || j == 11)
      {
        if (tmp == '.')
        {
          dot = 10;
        }
        else if (dot == 0) //conversion of integer digits
        {
          data[j] *= 10;
          data[j] += (double)(tmp - '0');
        }
        else //conversion of decimal digits
        {
          data[j] += (double)(tmp - '0') / dot;
          dot *= 10;
        }
      }
    }
    //Result: data[1]: latitude, data[2]: longitude, data[11]: height

    //height is compressed to 8 bits
    if(data[11] < 0)
    {
      data[11] = 0;
    }
    unsigned char height = data[11] / delta_h_min;

    //latitude and longitude are compressed to 10 bits each
    unsigned long int latidude = 0x3FF & (long int)((delta_r_max + ((int)(data[1] * 1000000) - lat_ref) * 0.1111949266) / delta_d_min);
    unsigned long int longitude = 0x3FF & (long int)((delta_r_max + ((int)(data[2] * 1000000) - long_ref) * 0.1111949266) / delta_d_min);

    //height, status, latitude and longitude are stored in output array
    output_buffer[0] = (0x0FF & height);
    output_buffer[1] |= ((0x00F & latidude) << 4) + ((0x00E & status) << 1);
	  output_buffer[2] |= ((0x3F0 & latidude) >> 4) + ((0x003 & longitude) << 6);
	  output_buffer[3] |= ((0x3FC & longitude) >> 2);

    //Parity is calculated
    unsigned long int parity = output_buffer[0] + (output_buffer[1] << 8) + (output_buffer[2] << 16) + (output_buffer[3] << 24);
    parity ^= parity >> 16;
    parity ^= parity >> 8;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    
    //parity is stored in output array
    output_buffer[1] |= (0x001 & parity);

    //MISSING: Storing data onboard (First send \n !!)

    //Sending 4 bytes of data over radio
    Serial0.print(output_buffer);
  }
}

//Sending reference coordinates to ground station
void reference_coordinates()
{
  //Function needs regular data package array
  while(data_package[0] != 'X')
  {
    if(Serial1.available() >= package_length_min)
    {
      data_handling();
    }
  }

  //Process similar to data handling, only values two and three important (latitude, longitude)
  double data[3] = { 0 };
  long int dot = 0;
  for (int i = 2, j = 0; j < 3; i++)
  {
    char tmp = data_package[i];
    if (tmp == ';')
    {
      dot = 0;
      j++;
    }
    else if (j == 1 || j == 2)
    {
      if (tmp == '.')
      {
        dot = 10;
      }
      else if (dot == 0)
      {
        data[j] *= 10;
        data[j] += (double)(tmp - '0');
      }
      else
      {
        data[j] += (double)(tmp - '0') / dot;
        dot *= 10;
      }
    }
    //Result: data[1]: latitude, data[2]: longitude

  //Conversion double to long int (49.123456 -> 49123456)
  lat_ref = data[1] * 1000000;
  long_ref = data[2] * 1000000;

  Serial0.print(String(lat_ref) + ';' + String(long_ref));
}

//Flushing all serial ports
void flush_serial_ports()
{
  while(Serial0.available() != 0)
  {
    Serial0.read();
  }
  while(Serial1.available() != 0)
  {
    Serial1.read();
  }
  while(Serial2.available() != 0)
  {
    Serial2.read();
  }
}

//Processing commands received by the radio module
char radio_commands()
{
  char cmd_buffer[4] = {0};
  //Storing input in buffer
  for(int i = 0; i < 4 && Serial0.available() >= 4 - i; i++)
  {
    cmd_buffer[i] = Serial0.read();
  }
  //Checking for "CMD" at beginning of input
  if(cmd_buffer[0] == 'C' && cmd_buffer[1] == 'M' && cmd_buffer[2] == 'D')
  {
    //Returning the command byte (last byte)
    return cmd_buffer[3];
  }
  flush_serial_ports();
  return 0;
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
