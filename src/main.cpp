#include <SPI.h>
#include <SD>

FILE myFile;

void setup() {

  // Set MOSI (PA4) and SCK (PA6) as output
  PORTA.DIRSET = PIN4_bm | PIN6_bm;
  // Set MISO (PA5) as input
  PORTA.DIRCLR = PIN5_bm;
  
  Serial.begin(19500)
  SPI.begin();
  SPI.beginTransaction(SPISettings(25000000, MSBFIRST, SPI_MODE0));

  int SS1 = PA3; 

  
   // see if the card is present and can be initialized:
  if (!SD.begin(SS1)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    while(1);
  }
  Serial.println("card initialized.");

  SD.mkdir("test.txt");

myFile = SD.open("test.txt", FILE_WRTIE);

if(myFile) { //returns 1 if myFile is available
  Serial.print("Writing text to test.txt...");
  myFile.println("test 1,2,3. Is this text visible?");
  myFile.close();
  Serial.println("Done writing to test.text");
}
else 
  Serial.println("Wasn't able to open 'test.txt'");

myFile = SD.open("test.txt");
if(myFile){
  Serial.println("test.txt:");

  while(myFile.available()) 
  Serial.write(myFile.read());
}

myFile.close();
else 
  // if the file didn't open, print an error:
  Serial.println("error opening test.txt");
}
