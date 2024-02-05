
#include <SoftwareSerial.h>

SoftwareSerial gpsSerial(6, 7); // RX, TX

char buffer[64];
int count=0;

void setup() {
  // put your setup code here, to run once:
    gpsSerial.begin(9600);                 // the gpsSerial baud rate
    Serial.begin(9600);   
}

void loop() {
  // put your main code here, to run repeatedly:
    if (gpsSerial.available()) 
    {                   // if date is coming from software serial port ==> data is coming from gpsSerial shield   
        while(gpsSerial.available()) {             // reading data into char array
              buffer[count++]=gpsSerial.read();    // writing data into array
              if(count == 64)break;
        }

        
    }
    Serial.write(buffer);
    Serial.println(" -received-");

    gpsSerial.print("valid");

    delay(1000);
    //clearBufferArray();
    count=0;
}

void clearBufferArray(){
    for (int i=0; i<count;i++)
      buffer[i]=NULL; // clear all index of array with command NULL
}
