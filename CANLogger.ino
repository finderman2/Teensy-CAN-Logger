// Teensy CAN Logger ---------------------------------------------
// Based on CANtest for Teensy 3.1 example by teachop
// Further Modifications by Liam O'Brien
//


#include <Metro.h>
#include <FlexCAN.h>
#include <SPI.h>
#include <SD.h>       /* Library from Adafruit.com */

Metro pauseLed = Metro(1000);// milliseconds
unsigned long time;  //used for time stamp
boolean toggleP = LOW;
FlexCAN CANbus(500000);
static CAN_message_t rxmsg;

// SD Card Setup ---------------------------------------------------

Sd2Card card;
SdVolume volume;
SdFile root;
SdFile file;

#define SCK_PIN   13  //Clock pin
#define MISO_PIN  12  //Mater in Slave output
#define MOSI_PIN  11  //Master out Slave input
#define SD_PIN    10  //pin for SD card control

// Setup Loop -------------------------------------------------------------
void setup() {

  Serial.begin(115200);
  CANbus.begin();
  
  pinMode(SD_PIN, OUTPUT);
  pinMode(MISO_PIN, OUTPUT);
  pinMode(MOSI_PIN, INPUT);
  

  Serial.println("Setup Finished");
  
 
// SD Card Setup -----------------------------------------------
  
  // initialize the SD card at SPI_HALF_SPEED to avoid bus errors with
  // breadboards.  use SPI_FULL_SPEED for better performance.
  if (!card.init(SPI_FULL_SPEED,SD_PIN)) {
    Serial.println("ERROR: card.init failed");
  }

  // initialize a FAT volume
  if (!volume.init(&card)) {
    Serial.println("ERROR: volume.init failed");
  }
  
  // open the root directory
  if (!root.openRoot(&volume)) {
    Serial.println("ERROR: openRoot failed");
  }
}


// Main Loop -------------------------------------------------------------

void loop() {
  if (!CANbus.available()) {
    Serial.println("CAN bus not available. Check connections");
  }
  
  while(!CANbus.available()) {
       if (pauseLed.check() ==1){
         //digitalWrite(led, toggleP); 
         toggleP = !toggleP; 
         Serial.println("CAN not available");
       }
  } //waiting for Canbus to be connected
  
  Serial.println("CAN Bus Connected and available");   //only comes out of the above loop if CAN is available
  
  // create a new file
  time = millis();
  char name[] = "CANlog-00.csv";
  for (uint8_t i = 0; i < 100; i++) {
    name[6] = i/10 + '0';
    name[7] = i%10 + '0';
    if (file.open(&root, name, O_CREAT | O_EXCL | O_WRITE)) break;
  }
  
  if (!file.isOpen()) {
    Serial.println("file.created");
  }
  Serial.print("Writing to: ");
  Serial.println(name);
  file.println("seconds,milliSec, ID, B0, B1, B2, B3, B4, B5, B6, B7"); //, B0(DEC), B1(DEC), B2(DEC), B3(DEC), B4(DEC), B5(DEC), B6(DEC), B7(DEC)");
  //file.println(buffer1);
  //file.println();  
  Serial.println("Header written to SD Card");  
 
  CAN_Capture();

}//end loop 
 
 
void CAN_Capture(){
  Serial.println("CAN capture has started.");  
   while(true){                 //stay within this loop unless forced out
     if (CANbus.read(rxmsg)){    //while messages are available perform the following
        String CANStr(""); 
        time = millis();  //capture time when message was recieved 
        CANStr +=String(time); //Time in milliseconds
        CANStr += (",");
        CANStr += String(rxmsg.id,HEX); // CAN ID
        CANStr += String(",");
        CANStr += String(rxmsg.ext); // Extended ID or not
        CANStr += String(",");
        CANStr += String("0"); //CAN Bus, setting it to zero in this instance.
        CANStr += String(",");
        CANStr += String(rxmsg.len,DEC); //Message Length
        for (int i=0; i < 8; i++) {     
          CANStr += (",") ;
          CANStr += String(rxmsg.buf[i],HEX);
        }

        file.println(CANStr);    //print the CAN message to the file that should already be open
        Serial.println(CANStr); // print CAN messages to the 
     }
    }
}
