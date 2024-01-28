#include<FastLED.h>
#define LED_PIN 2
#define NUM_LEDS 9

CRGB leds[NUM_LEDS];

void setup() {
  // put your setup code here, to run once:
  FastLED.addLeds<WS2812, LED_PIN, GRB> (leds, NUM_LEDS) ;   //true order is GRB not RGB
  FastLED.setMaxPowerInVoltsAndMilliamps(5,500);
  FastLED.clear();
  FastLED.show();
}

void loop() {
  // put your main code here, to run repeatedly:

  //RGB Calibration 
  leds[0]= CRGB(1,0,0);
  leds[1]= CRGB(0, 1, 0);
  leds[2]= CRGB(0, 0, 1);
  FastLED.show();

}
