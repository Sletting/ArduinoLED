#include <FastLED.h>

#define NUM_LEDS  28
#define LED_PIN   2

CRGB leds[NUM_LEDS];

//Button controls
const int buttonSwitchPin = 6;     // the number of the pushbutton pin
const int buttonPourPin = 7;     // the number of the pushbutton pin

// variables will change:
byte buttonSwitchState = 0;         // variable for reading the pushbutton status
boolean buttonSwitchHasBeenPressed = false;
byte buttonPourState = 0;         // variable for reading the pushbutton status
boolean buttonPourHasBeenPressed = false;

// General variables
byte maxMode = 4;
byte pastMode = 0;
byte mode = 0;
byte pourMode = 0;
unsigned long startMillis;  //some global variables available anywhere in the program
unsigned long currentMillis;
const unsigned long period = 2000;  //the value is a number of milliseconds


// Variables for Rainbow effect
uint8_t hue = 0;

// These three custom blue-green color palettes were inspired by the colors found in
// the waters off the southern coast of California, https://goo.gl/maps/QQgd97jjHesHZVxQ7
// Variables for Pacifica
CRGBPalette16 pacifica_palette_1 = 
    { 0x000507, 0x000409, 0x00030B, 0x00030D, 0x000210, 0x000212, 0x000114, 0x000117, 
      0x000019, 0x00001C, 0x000026, 0x000031, 0x00003B, 0x000046, 0x14554B, 0x28AA50 };
CRGBPalette16 pacifica_palette_2 = 
    { 0x000507, 0x000409, 0x00030B, 0x00030D, 0x000210, 0x000212, 0x000114, 0x000117, 
      0x000019, 0x00001C, 0x000026, 0x000031, 0x00003B, 0x000046, 0x0C5F52, 0x19BE5F };
CRGBPalette16 pacifica_palette_3 = 
    { 0x000208, 0x00030E, 0x000514, 0x00061A, 0x000820, 0x000927, 0x000B2D, 0x000C33, 
      0x000E39, 0x001040, 0x001450, 0x001860, 0x001C70, 0x002080, 0x1040BF, 0x2060FF };

// Variables for Mix1
uint8_t colorIndex[NUM_LEDS];
DEFINE_GRADIENT_PALETTE( bluepuke_gp ) {
  0, 206, 217, 24,
  106, 0, 212, 255,
  136, 0, 212, 255, 
  255, 206, 217, 24,
};
CRGBPalette16 bluepuke = bluepuke_gp;

// Gradient palette "christmas_candy_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/ocal/tn/christmas-candy.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 44 bytes of program space.
uint8_t paletteIndex = 0;
DEFINE_GRADIENT_PALETTE( christmas_candy_gp ) {
    0,   0,255,  0,
   93,  42,255, 45,
  127, 255,255,255,
  161, 255, 55, 45,
  255, 255,  0,  0};
CRGBPalette16 christmasCandy = christmas_candy_gp;


// Gradient palette "es_emerald_dragon_12_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/es/emerald_dragon/tn/es_emerald_dragon_12.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 16 bytes of program space.
uint8_t emeraldPaletteIndex = 0;
DEFINE_GRADIENT_PALETTE( es_emerald_dragon_12_gp ) {
    0,   1, 10,  1,
  153, 104,156,  7,
  229, 206,237,138,
  255, 213,233,158};
CRGBPalette16 emerald = es_emerald_dragon_12_gp;



// Variables for pourComplete
uint8_t hue2 = 0;
unsigned long startPourMillis;  //some global variables available anywhere in the program
unsigned long currentPourMillis;
const unsigned long pourPeriod = 1200;  //the value is a number of milliseconds


void setup() {
  // initialize the pushbutton pin as an input:
  pinMode(buttonSwitchPin, INPUT);
  pinMode(buttonPourPin, INPUT);

  delay(2000); // 3 second delay for recovery
  
  // put your setup code here, to run once:
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(200);

  for (int i = 0; i < NUM_LEDS; i++) {
    colorIndex[i] = random8();
  }
}

void loop() {
  // read the state of the pushbutton value:
  buttonSwitchState = digitalRead(buttonSwitchPin);
  buttonPourState = digitalRead(buttonPourPin);

  //Change light mode
  if(buttonSwitchState == HIGH) {    
    if(buttonSwitchHasBeenPressed == false) { //Makes sure that the button call will only be triggered once per click
      mode++;
      if (mode > maxMode) {
        mode = 0;
      }
    }
    buttonSwitchHasBeenPressed = true;
  } else {
    buttonSwitchHasBeenPressed = false;
  }
  
  //Changes state to pouring
  if(buttonPourState == HIGH) {   
    if(buttonPourHasBeenPressed == false) { //Makes sure that the button call will only be triggered once per click
      if(mode != 100){ //Makes sure that it is not already at mode 100 (Makes sure that double taps does not make it stuck with "done" light)
        pastMode = mode;  
      }      
      mode = 100;   
    }        
    staticColor();
    buttonPourHasBeenPressed = true;    
  } else {    
    if(buttonPourHasBeenPressed){
      startMillis = millis();
    }              
    buttonPourHasBeenPressed = false;    
    
    switch (mode){ //The switch that controls all the possible modes. More can easily be added
      case 0:
        rainbow();
        break;
      case 1:        
        pacifica_loop();
        break;
      case 2:
        mix1();
        break;
      case 3:
        christmasLights();     
        break;
      case 4:
        emeraldMix();
        break;
      case 100:        
        currentMillis = millis();  //get the current "time" (actually the number of milliseconds since the program started)        
        if (currentMillis - startMillis >= period)  //test whether the period has elapsed
        {
          mode = pastMode;
        }      
        pourComplete();                   
        break;
    }
  }
  
  FastLED.show(); //Show is called at the end of the method to display whatever has been decided by the switch case
}

// Static color for when pouring
void staticColor() {
  for(int i = 0; i < NUM_LEDS; i++ ) {
    if(i % 2){
      leds[i].r = 0xFF;  // Red
      leds[i].g = 0xFF;  // Green
      leds[i].b = 0xFF;  // Blue
    } else {
      leds[i].r = 0x00;  // Red
      leds[i].g = 0x00;// Green
      leds[i].b = 0x00;  // Blue
    }
  }    
}

//Displays a small animation and in the end goes to a static color for a specified time
void pourComplete(){
  currentPourMillis = millis();  //get the current "time" (actually the number of milliseconds since the program started)
  if (currentPourMillis - startMillis >= pourPeriod) { //test whether the period has elapse 
    for(int i = 0; i < NUM_LEDS; i++ ) {
      leds[i].r = 0x00;  // Red
      leds[i].g = 0xFF;  // Green
      leds[i].b = 0x00;  // Blue

    }     
  } else {
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CHSV(hue2 + (i * 2), 255, 255);    
   }
  }  

  EVERY_N_MILLISECONDS(1) {
    hue2++;
  }
}


//Displays a rainbow effect
void rainbow() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CHSV(hue + (i * 2), 255, 255);
  }

  EVERY_N_MILLISECONDS(15) {
    hue++;
  }
}

//////////////////////////////////////////////////////////////////////////
//
// The code for this animation is more complicated than other examples, and 
// while it is "ready to run", and documented in general, it is probably not 
// the best starting point for learning.  Nevertheless, it does illustrate some
// useful techniques.
//
//////////////////////////////////////////////////////////////////////////
//
// In this animation, there are four "layers" of waves of light.  
//
// Each layer moves independently, and each is scaled separately.
//
// All four wave layers are added together on top of each other, and then 
// another filter is applied that adds "whitecaps" of brightness where the 
// waves line up with each other more.  Finally, another pass is taken
// over the led array to 'deepen' (dim) the blues and greens.
//
// The speed and scale and motion each layer varies slowly within independent 
// hand-chosen ranges, which is why the code has a lot of low-speed 'beatsin8' functions
// with a lot of oddly specific numeric ranges.
//
void pacifica_loop()
{
  EVERY_N_MILLISECONDS( 20) {
    // Increment the four "color index start" counters, one for each wave layer.
    // Each is incremented at a different speed, and the speeds vary over time.
    static uint16_t sCIStart1, sCIStart2, sCIStart3, sCIStart4;
    static uint32_t sLastms = 0;
    uint32_t ms = GET_MILLIS();
    uint32_t deltams = ms - sLastms;
    sLastms = ms;
    uint16_t speedfactor1 = beatsin16(3, 179, 269);
    uint16_t speedfactor2 = beatsin16(4, 179, 269);
    uint32_t deltams1 = (deltams * speedfactor1) / 256;
    uint32_t deltams2 = (deltams * speedfactor2) / 256;
    uint32_t deltams21 = (deltams1 + deltams2) / 2;
    sCIStart1 += (deltams1 * beatsin88(1011,10,13));
    sCIStart2 -= (deltams21 * beatsin88(777,8,11));
    sCIStart3 -= (deltams1 * beatsin88(501,5,7));
    sCIStart4 -= (deltams2 * beatsin88(257,4,6));
  
    // Clear out the LED array to a dim background blue-green
    fill_solid( leds, NUM_LEDS, CRGB( 2, 6, 10));
  
    // Render each of four layers, with different scales and speeds, that vary over time
    pacifica_one_layer( pacifica_palette_1, sCIStart1, beatsin16( 3, 11 * 256, 14 * 256), beatsin8( 10, 70, 130), 0-beat16( 301) );
    pacifica_one_layer( pacifica_palette_2, sCIStart2, beatsin16( 4,  6 * 256,  9 * 256), beatsin8( 17, 40,  80), beat16( 401) );
    pacifica_one_layer( pacifica_palette_3, sCIStart3, 6 * 256, beatsin8( 9, 10,38), 0-beat16(503));
    pacifica_one_layer( pacifica_palette_3, sCIStart4, 5 * 256, beatsin8( 8, 10,28), beat16(601));
  
    // Add brighter 'whitecaps' where the waves lines up more
    pacifica_add_whitecaps();
  
    // Deepen the blues and greens a bit
    pacifica_deepen_colors();
  }
}

// Add one layer of waves into the led array
void pacifica_one_layer( CRGBPalette16& p, uint16_t cistart, uint16_t wavescale, uint8_t bri, uint16_t ioff)
{
  uint16_t ci = cistart;
  uint16_t waveangle = ioff;
  uint16_t wavescale_half = (wavescale / 2) + 20;
  for( uint16_t i = 0; i < NUM_LEDS; i++) {
    waveangle += 250;
    uint16_t s16 = sin16( waveangle ) + 32768;
    uint16_t cs = scale16( s16 , wavescale_half ) + wavescale_half;
    ci += cs;
    uint16_t sindex16 = sin16( ci) + 32768;
    uint8_t sindex8 = scale16( sindex16, 240);
    CRGB c = ColorFromPalette( p, sindex8, bri, LINEARBLEND);
    leds[i] += c;
  }
}

// Add extra 'white' to areas where the four layers of light have lined up brightly
void pacifica_add_whitecaps()
{
  uint8_t basethreshold = beatsin8( 9, 55, 65);
  uint8_t wave = beat8( 7 );
  
  for( uint16_t i = 0; i < NUM_LEDS; i++) {
    uint8_t threshold = scale8( sin8( wave), 20) + basethreshold;
    wave += 7;
    uint8_t l = leds[i].getAverageLight();
    if( l > threshold) {
      uint8_t overage = l - threshold;
      uint8_t overage2 = qadd8( overage, overage);
      leds[i] += CRGB( overage, overage2, qadd8( overage2, overage2));
    }
  }
}

// Deepen the blues and greens
void pacifica_deepen_colors()
{
  for( uint16_t i = 0; i < NUM_LEDS; i++) {
    leds[i].blue = scale8( leds[i].blue,  145); 
    leds[i].green= scale8( leds[i].green, 200); 
    leds[i] |= CRGB( 2, 5, 7);
  }
}

void mix1(){
  for (int i = 0; i < NUM_LEDS; i++){
    leds[i] = ColorFromPalette(bluepuke, colorIndex[i]);   
  }

  EVERY_N_MILLISECONDS(5){
    for (int i = 0; i < NUM_LEDS; i++){
      colorIndex[i]++;
    }
  }
}

void emeraldMix(){
  fill_palette(leds, NUM_LEDS, emeraldPaletteIndex, 255 / NUM_LEDS, emerald, 255, LINEARBLEND);

  EVERY_N_MILLISECONDS(10){
    emeraldPaletteIndex++;
  }
}

void christmasLights(){
  fill_palette(leds, NUM_LEDS, paletteIndex, 255 / NUM_LEDS, christmasCandy, 255, LINEARBLEND);

  EVERY_N_MILLISECONDS(10){
    paletteIndex++;
  }
}
