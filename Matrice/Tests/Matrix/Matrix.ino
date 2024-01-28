#include <FastLED.h>

#define LED_DATA_PIN  2     // Pin de données pour la matrice

#define COLOR_ORDER GRB
#define CHIPSET     WS2812
#define BRIGHTNESS 10       // luminosité réglable

/* Fonction ciblage de LED
  XY(x, y) prend les coordonnées x et y et renvoie un numéro d'indice de LED, 
    à utiliser comme ceci : 
          leds[XY(x, y)] == CRGB::Red;
    
  Une vérification d'erreur EST effectuée sur les plages de x et y, une coordonnée inexistante ne sera pas prise en compte

*/

// Dimensions de la matrice 9x9
const uint8_t kMatrixWidth = 9;
const uint8_t kMatrixHeight = 9;

/* Matrice disposée en serpentin
    0 >  1 >  2 >  3 >  4
                        =
    9 <  8 <  7 <  6 <  5
    =
    10 > 11 > 12 > 13 > 14
                         =
    19 < 18 < 17 < 16 < 15
*/

uint16_t calcul_coordonnee( uint8_t x, uint8_t y)
{
  uint16_t i;

  if( y & 0x01) 
  {
    // lignes impaires : ordre inversé
    uint8_t reverseX = (kMatrixWidth - 1) - x;
    i = (y * kMatrixWidth) + reverseX;
  } 
  else 
  {
    // lignes paires : ordre normal
    i = (y * kMatrixWidth) + x;
  }
    
  return i;
}

#define NUM_LEDS (kMatrixWidth * kMatrixHeight)
CRGB leds_plus_safe[ NUM_LEDS + 1];     // +1 led inexistante en cas de coordonnée hors de la matrice
CRGB* const leds( leds_plus_safe + 1);

uint16_t XY( uint8_t x, uint8_t y)
{
  if( x >= kMatrixWidth) return -1;
  if( y >= kMatrixHeight) return -1;
  return calcul_coordonnee(x,y);
}

void setup() 
{
  FastLED.addLeds<CHIPSET, LED_DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalSMD5050);
  FastLED.setMaxPowerInVoltsAndMilliamps(5,500);
  FastLED.clear();
  FastLED.show();
}

void loop()
{ 
  FastLED.setBrightness(BRIGHTNESS);
  leds [XY(0,2)] = CRGB::Red;
  leds [XY(0,0)] = CRGB::Blue;
  leds [XY(0,1)] = CRGB::White;

  leds [XY(6,8)] = CRGB::Red;
  leds [XY(8,8)] = CRGB::Blue;
  leds [XY(7,8)] = CRGB::White;
  FastLED.show();

  delay(500);
  FastLED.setBrightness(0);  FastLED.show() ;
  delay(500);

}




