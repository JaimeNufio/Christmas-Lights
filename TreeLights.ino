#define FASTLED_ALLOW_INTERRUPTS 0

#include <FastLED.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#define LED_PIN     5
#define COLOR_ORDER RGB
#define CHIPSET     WS2811
#define NUM_LEDS    100
#define BRIGHTNESS  255
#define FRAMES_PER_SECOND 60
#define SECONDS_PER_MODE 60*5

#define NOTIFICATION  0
#define TWINKLE       1
#define MERRY         2
#define LASER         3
#define SOLID         4

ESP8266WebServer server(80);
CRGB leds[NUM_LEDS];
int ledState[NUM_LEDS];
int Mode = TWINKLE;
int step = 0;
int largeStep = 0;
int currentLed = 0;
int timeElapsed = 0;
int timeSince = 0;

const char *ssid = "NufioWifi";
const char *password = "FlapjackAndWaffles";

void handleRandom();
int roundToNearest(int value, int roundTo);

int defaultModes[3] = {TWINKLE, MERRY, LASER};
int twinkleRounding = 10;
CRGB currentColor = CRGB(255,255,255);

void randomSolidColor();

void handleTwinkle();
void initTwinkle();
void updateTwinkle();

void handleNotification();
void initNotification();
void updateNotification();

void handleMerry();
void initMerry();
void updateMerry();

void handleLaser();
void initLaser();
void updateLaser();

void handleSolid();
void initSolid();
void updateSolid();

void setup () {
  delay(3000); // sanity delay
  
  // FastLED Init
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness( BRIGHTNESS );

  Serial.begin(115200);

  // Wifi Connect
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Define server routes
  server.on("/twinkle", HTTP_GET, handleTwinkle);
  server.on("/notification", HTTP_GET, handleNotification);
  server.on("/merry", HTTP_GET, handleMerry);
  server.on("/laser", HTTP_GET, handleLaser);
  server.on("/solid", HTTP_GET, handleSolid);

  // Start server
  server.begin();
  Serial.println("HTTP server started");
  handleTwinkle();
}

void loop () {
  server.handleClient();  
  
  timeElapsed += millis()-timeSince;
  timeSince = millis();

  if (Mode == TWINKLE){
    updateTwinkle();
  } else if (Mode == NOTIFICATION){    
    updateNotification();

    if (timeElapsed > 1000*3){
      handleRandom();
    }

  } else if (Mode == MERRY){
    updateMerry();
  } else if (Mode == LASER){
    updateLaser();
  }else if (Mode == SOLID){
    updateSolid();
  }


  FastLED.show(); // display this frame
  FastLED.delay(1000 / FRAMES_PER_SECOND);

  if (timeElapsed > 1000*SECONDS_PER_MODE && Mode != NOTIFICATION){
    timeElapsed = 0;
    handleRandom();
  }
}

void initNotification () {
  currentLed = 0;
  for (int i = 0; i<NUM_LEDS; i++){
    ledState[i] = 0;
    leds[i] = CRGB(0,0,0);
  }
}

void handleNotification () {
  Mode = NOTIFICATION;
  timeElapsed = 0; // reset and reuse timer.

  initNotification();
  Serial.println("Notification!");
}

void updateNotification () {

  if (currentLed >= NUM_LEDS){
    initNotification();
  }

  // for (int i = currentLed; i<NUM_LEDS; i++){
    int i = currentLed;
    ledState[i] = ledState[i]+255;
    ledState[i+1] = ledState[i+1]+255;

    if (ledState[i] > 255){
      ledState[i] = 255;
    }

    leds[i] = CRGB(0,ledState[i],0);
    leds[i+1] = CRGB(0,ledState[i+1],0);
    
    currentLed = currentLed + 2;

  // }
}


void initTwinkle () {
  for (int i = 0; i<NUM_LEDS; i++){
    ledState[i] = random(255);
    leds[i] = CRGB(ledState[i],ledState[i],ledState[i]);
  }
}

void handleTwinkle () {
  Mode = TWINKLE;
  timeElapsed = 0;
  initTwinkle();
  Serial.println("Twinkle!");
}

void updateTwinkle () {
  
  for (int i = 0; i<NUM_LEDS; i++){
    ledState[i] = (ledState[i]+1) % 255;

    int intensity = roundToNearest((255*sin(ledState[i]*2*3.14/255)),twinkleRounding);

    if (intensity > 255) {intensity = 255;}
    if (intensity <= 25) {intensity = 5;}

    leds[i] = CRGB(
      // 0xFC,0xEE,0xA7
      intensity,intensity,intensity
    );
    
  }
}

void handleMerry () {
  Mode = MERRY;
  timeElapsed = 0;
  initMerry();
  Serial.println("Merry!");
}

void initMerry () {
  currentLed = 0;
}

void updateMerry () {
  int largeLoopSize = FRAMES_PER_SECOND/2;
  step = (step+1);
  
  if (step > largeLoopSize){
    step = 0;

    // Serial.println(largeStep);
    largeStep = (largeStep+1)%3;
  }


  for (int i = 0; i < 100; i++){
    if(i%3==0){
      if (largeStep == 0){
        leds[i] = CRGB(255,0,0); // RED
      } else if (largeStep == 1){
        leds[i] = CRGB(255,255,0); // YELLOW
      } else if (largeStep == 2){
        leds[i] = CRGB(0,255,0); // GREEN
      }
    }
    if(i%3==1){
      if (largeStep == 0){
        leds[i] = CRGB(0,255,0); // GREEN
      } else if (largeStep == 1){
        leds[i] = CRGB(255,0,0); // RED
      } else if (largeStep == 2){
        leds[i] = CRGB(255,255,0); // YELLOW
      }
    }
    if(i%3==2){
      if (largeStep == 0){
        leds[i] = CRGB(255,255,0); // YELLOW
      } else if (largeStep == 1){
        leds[i] = CRGB(0,255,0); // GREEN
      } else if (largeStep == 2){
        leds[i] = CRGB(255,0,0); // RED
      }
    }
  }
}

void handleLaser () {
  Mode = LASER;
  timeElapsed = 0;
  initLaser();
  randomSolidColor();
  Serial.println("Laser!");
}

void initLaser() {
  currentLed = 0;
}

void updateLaser() {
  step += 1;

  if (step > FRAMES_PER_SECOND/50){
    step = 0;
    currentLed+=1;
  }

  if (currentLed >= 80) {
    currentLed = 0;
  }

  for (int i = 0; i<NUM_LEDS; i++){
    if (near(currentLed, i, 3)){
      leds[i] = currentColor;
    }else{
      leds[i] = CRGB(0,0,0);
    }
  }
}


void handleSolid () {
  Mode = SOLID;
  timeElapsed = 0;
  initSolid();
}

void initSolid() {
  randomSolidColor();
}

void updateSolid() {
  for (int i = 0; i<NUM_LEDS; i++){
    leds[i] = currentColor;
  }
}


void handleRandom() {
  int nextMode = random(4)+1; // 1,2,3... notification is 0

  while (nextMode == Mode){ // avoid running same thing twice
    nextMode = random(4)+1;
  }

  switch (nextMode){
    case 1:
      handleTwinkle();
      break;
    case 2:
      handleMerry();
      break;
    case 3:
      handleLaser();
      break;
    case 4:
      handleSolid();
      break;
    default:
      handleTwinkle();
      break;
  }
}

void randomSolidColor(){
  switch (random(5)){
    case 1: //red
      currentColor = CRGB(255,0,0);
      break;
    case 2: //green
      currentColor = CRGB(0,255,0);
      break;
    case 3: //red
      currentColor = CRGB(0,0,255);
      break;
    case 4: //orange
      currentColor = CRGB(255,69,0);
      break;
    default: //white
      currentColor = CRGB(255,255,255);
      break;
  }
}

bool near(int leaderLed, int currentLed, int dst){
  int max = (leaderLed+dst)%NUM_LEDS;
  int min = (leaderLed-dst)%NUM_LEDS;

  if (min > max){
    return currentLed <= max || currentLed >= min;
  }else{
    return currentLed >= min && currentLed <= max;
  }
}

int roundToNearest(int value, int roundTo) {
  int remainder = value % roundTo;
  int halfRoundTo = roundTo / 2;
  int roundedValue = value + (remainder < halfRoundTo ? -remainder : roundTo - remainder);

  return roundedValue;
}