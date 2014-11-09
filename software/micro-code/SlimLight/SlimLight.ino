

//###############################################################################
//# SilhuetteLight - controller
//#
//# Based on a ATTiny84 controller with USB (D- on PortA.1 D+ on PortA.0 Interupt on PortB.2
//#
//# Button on D8, LED on D7 and RGB-out on D3
//# Pixelstrip can be attached in segments of 4.
//#
//###############################################################################

#include <Adafruit_NeoPixel.h>
#include <Bounce2.h>

#define DEBOUNCE_TIME 5    //Debounce time in ms
#define LONG_PRESS    400  //Time in ms for long-press
#define SHORT_PRESS   100   //Time in ms for short-press (to avoid any non-pressess)

#define PIXEL_PIN     3      //Pixel strio in pin D3 (attiny84: PA7, pin 6)
#define LED_PIN       7      //Led on pin D7 (attiny84: PA3, pin 10)
#define BUTTON_PIN    8      //Button on pin D8 (attiny84: PA2, pin 11)

#define STRIP_LENGTH  4

#define OFF           0
#define ON            1

#define M_OFF         0
#define M_RED         1
#define M_GREEN       2
#define M_BLUE        3
#define M_COLOR       4
#define M_CYCLE       5
#define M_CYCLE_STOP  6
#define M_WHITE       7
#define M_MODES       8

#define COLOR_STEP    10

#define CYCLE_TIME    100

Adafruit_NeoPixel strip = Adafruit_NeoPixel(STRIP_LENGTH, PIXEL_PIN, NEO_GRB + NEO_KHZ800);
Bounce debouncePin = Bounce(); 

uint8_t  dispMode;
uint8_t  ledState;
uint8_t  buttonState;
uint8_t  colorValue;
uint8_t  blinkedLed;
byte dispColor;
//uint16_t pressLength;
long     startTime, pressTime, cycleTime;
//long debounceTime = DEBOUNCE;

void setup() {
  pinMode(BUTTON_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  
  dispMode = M_OFF;
  ledState = 1;
  buttonState = 0;
  
  dispColor = 0;
  
  debouncePin.attach(BUTTON_PIN);
  debouncePin.interval(DEBOUNCE_TIME);
  
  digitalWrite(LED_PIN, HIGH);
}

uint8_t oldMode = 99;
void loop() {
  boolean stateChanged = debouncePin.update();
  int pinState = debouncePin.read();
  if(stateChanged && (pinState == LOW))
  {
    //starta Timer...
    startTime = millis();
    blinkedLed = 0; 
  }
  else if(stateChanged && (pinState == HIGH))
  {
    pressTime = millis() - startTime;
    if(pressTime >= LONG_PRESS)
    {
      LongPress();
    }
    else if(pressTime >= SHORT_PRESS)
    {
      ShortPress();
    }
  }
  else if((blinkedLed == 0) && !stateChanged && (pinState == LOW) && (millis() > startTime + LONG_PRESS))
  {
     digitalWrite(LED_PIN, LOW);
     delay(15);
     digitalWrite(LED_PIN, HIGH);
     blinkedLed = 1;
  }
  if((oldMode != dispMode) || (dispMode > M_COLOR))
  { 
    switch(dispMode)
    {
      case M_OFF:
        setAllToColor(0,0,0);
        break;
      case M_RED:
        setAllToColor(colorValue,0,0);
        break;
      case M_GREEN:
        setAllToColor(0,colorValue,0);
        break;
      case M_BLUE:
        setAllToColor(0,0,colorValue);
        break;
      case M_COLOR:
        setAllToColor32(Wheel(dispColor));
        break;
      case M_CYCLE:
        if(millis() > cycleTime + CYCLE_TIME)
        {
          dispColor++;
          if(dispColor > 255) dispColor = 0;
          setAllToColor32(Wheel(dispColor));
          cycleTime = millis();
        }
      case M_CYCLE_STOP:
        break;
      default:
        setAllToColor(50,50,50);
        break;
    }
  }
}

void LongPress()
{

  dispMode++;
  if(dispMode >= M_MODES) dispMode = M_OFF;
  colorValue = 240;
  delay(10);
  digitalWrite(LED_PIN, HIGH );
}

void ShortPress()
{
  if(dispMode == M_CYCLE)
  {
    dispMode = M_CYCLE_STOP;
  }
  else if( dispMode == M_CYCLE_STOP)
  {
     dispMode = M_CYCLE;
  }
  else
  {
    colorValue -= 10;
    if(colorValue < 20) colorValue = 240;
    dispColor += COLOR_STEP;
    if(dispColor >= 255) dispColor = 0;
  }
}

void setAllToColor(uint8_t R, uint8_t G, uint8_t B)
{
  uint32_t color = strip.Color(R, G, B);
  for(uint16_t i=0; i < STRIP_LENGTH; i++) {
    strip.setPixelColor(i,color);
  }
  strip.show();
}

void setAllToColor32(uint32_t color)
{
//  uint32_t color = strip.Color(R, G, B);
  for(uint16_t i=0; i < STRIP_LENGTH; i++) {
    strip.setPixelColor(i,color);
  }
  strip.show();
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
      strip.show();
      delay(wait);
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, c);    //turn every third pixel on
      }
      strip.show();
     
      delay(wait);
     
      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
        for (int i=0; i < strip.numPixels(); i=i+3) {
          strip.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
        }
        strip.show();
       
        delay(wait);
       
        for (int i=0; i < strip.numPixels(); i=i+3) {
          strip.setPixelColor(i+q, 0);        //turn every third pixel off
        }
    }
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}

