#include<FastLED.h>
#define LED_PIN 2
#define NUM_LEDS 81

CRGB leds[NUM_LEDS];

void setup() {
  // put your setup code here, to run once:
  FastLED.addLeds<WS2812, LED_PIN, GRB> (leds, NUM_LEDS) ;   //true order is GRB not RGB
  FastLED.setMaxPowerInVoltsAndMilliamps(5,500);
  FastLED.clear();
  FastLED.show();
}

void loop() {
  // Glow green

  for(int i = 0 ; i< NUM_LEDS ; i++)
  {
    leds[i] = CRGB (0,255,0);
    FastLED.setBrightness(15*i);
    FastLED.show();
    delay(20);
  }
  for (int i = NUM_LEDS ; i>0 ; i--)
  {
    leds[i] = CRGB (0,255,0);
    FastLED.setBrightness(30-2*i);
    FastLED.show();
    delay(20);
  }

}
