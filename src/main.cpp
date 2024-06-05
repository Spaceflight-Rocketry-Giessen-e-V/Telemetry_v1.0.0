#include <avr/io.h>

int serial0_wait(int delay_microsec);
int serial1_wait(int delay_microsec);
int serial2_wait(int delay_microsec);

void flush_serial_ports();
char radio_commands();
void data_handing();
void reference_coordinates();

//Start byte: 'X', End byte: 'Y'
const char package_length = 60;
char data_package[package_length] = {0};

char flightmode = 0;

void setup() 
{
  //Pin initialisation (A7 (1): powerpin, D2 (8): parachutepin, D3 (9): ledpin1, D4 (10): ledpin2, D5 (11): modepin, D6 (12): cfgpin, D7 (13): rstpin)
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
  if(PORTD.IN & 0x10) //Reading 5th bit in PORTD (modepin)
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
            Serial0.write('#');
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

      //Checking for x incoming bytes from Serial1 (all data packages have length x)
      if(Serial1.available() >= package_length)
      {
        data_handing();
      }
    }
  }
}

void loop() {}

//Processing data coming from sensory subsystem
void data_handing()
{
  //Write input in global array
  for(int i = 0; i < package_length && Serial0.available() >= package_length - i; i++)
  {
    data_package[i] = Serial0.read();
  }
  //Check for start and end byte
  if(data_package[0] != 'X' || data_package[package_length-1] != 'Y')
  {
    //For wrong start or end byte: flush serial input
    //MISSING: Write in log file
    while(serial1_wait(600) != 0)
    {
      flush_serial_ports();
    }
    return;
  }
  //Only when in flightmode, data is stored and send via radio
  if(flightmode == 1)
  {
    String status = String(10 * PORTC_IN & 0x03 + PORTC_IN & 0x02); //Result: 00/01/10/11
    //MISSING: Splitting data_package array in individual variables
    //MISSING: Checking parity
    //MISSING: Storing data onboard
    //MISSING: Sending data via radio
  }
}

//Sending reference coordinates to ground station
void reference_coordinates()
{
  //Functions need regular data package array
  while(data_package[0] != 'X' || data_package[package_length - 1] != 'Y')
  {
    if(Serial1.available() >= package_length)
    {
      data_handing();
    }
  }
  //MISSING: lat_ref and long_ref calculation
  String lat_ref = ;
  String long_ref = ;
  //MISSING: removing dots from strings
  Serial0.print(lat_ref + ';' + long_ref);
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
  for(int i = 0; i < 4 && Serial0.available() >= 4 - i; i++)
  {
    cmd_buffer[i] = Serial0.read();
  }
  if(cmd_buffer[0] == 'C' && cmd_buffer[1] == 'M' && cmd_buffer[2] == 'D')
  {
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
