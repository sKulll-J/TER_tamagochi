
#include <SoftwareSerial.h>

#define maxCnt 64
#define RX_PIN 2
#define TX_PIN 7

#define BT_PIN 8

SoftwareSerial gpsSerial(RX_PIN, TX_PIN); // RX, TX

char buffer[maxCnt];
int count=0;


void handleRxInterrupt();

void setup() {
  // put your setup code here, to run once:
    gpsSerial.begin(9600);                 // the gpsSerial baud rate
    Serial.begin(9600);   
    attachInterrupt(digitalPinToInterrupt(RX_PIN), handleRxInterrupt, RISING); //CHANGE - RISING - FALLING
    pinMode(BT_PIN, INPUT_PULLUP);
}

void loop() {
  // put your main code here, to run repeatedly:
    /*if (gpsSerial.available()) 
    {                   // if date is coming from software serial port ==> data is coming from gpsSerial shield      

        Serial.write(gpsSerial.read());
        Serial.println(" -received-");
    }*/


    //delay(10);
    /*noInterrupts();
    interrupts();
    //clearBufferArray();
   */
    //Serial.println("..");
    if(digitalRead(BT_PIN)==LOW)
    {
        gpsSerial.write(0x41);
    }
    else
    {
        gpsSerial.write(NULL);//can't write 0
    }
    delay(100);
    count=0;
}

void clearBufferArray(){
    for (int i=0; i<maxCnt;i++)
      buffer[i]=NULL; // clear all index of array with command NULL
}


void handleRxInterrupt() {
  // Cette fonction sera appelée à chaque fois qu'une interruption est déclenchée sur la pin RX
   // Mettre le drapeau à true pour indiquer que des données ont été reçues
   if (gpsSerial.available() > 0) {
      //Serial.println(gpsSerial.read(), HEX);
      
       Serial.println(gpsSerial.read(), DEC);
   }
}
