#include <SD.h>                      // need to include the SD library
//#define SD_ChipSelectPin 53  //example uses hardware SS pin 53 on Mega2560
#define SD_ChipSelectPin 4  //using digital pin 4 on arduino nano 328, can use other pins
#include <TMRpcm.h>           //  also need to include this library...
#include <SPI.h>

/* LED stuff */
#define REDPIN 6
#define GREENPIN 5
#define BLUEPIN 3
int red = 0, grn = 0, blu = 0;
boolean redup = true, grnup = true, bluup = true;
unsigned long LEDdelay = 30; // milliseconds between changing LED color
unsigned long lastLEDchange = millis();
int LEDcoherence = 200; // number of times to change the same color in swirlLEDs()
int LEDchangeCnt = 0; // counts number of times changed same LED color
int color = random(3); // for choosing which color to switch

TMRpcm tmrpcm;   // create an object for use in this sketch

unsigned long time = 0;

int PIRpin = 7;
int PIRread;
bool PIRhigh;
unsigned long PIRdelay = 60000;
unsigned long lastTrigger = millis();

/* microphone stuff */
int sensorPin = A2;    // select the input pin for the potentiometer
int sensorValue = 0;  // variable to store the value coming from the sensor
float maxAnalog = 1023; // max analogRead value, for normalizing
float maxAnWr = 255; // max analogWrite value for scaling
float normSensorVal;
float EMA_NSV = 0; // exp mov avg of normalized sensor value
float period = 8; // exponential moving average period
float alpha = 1/period;
float EMA_NSV_long = 80; // exp mov avg of normalized sensor value
float longperiod = 200; // exponential moving average period
float longalpha = 1/longperiod;

String soundFiles[4] = {"laugh.wav", "laugh2.wav", "scream.wav", "cackle.wav"};
char filename[30];

void setup(){
  pinMode(REDPIN, OUTPUT);
  pinMode(GREENPIN, OUTPUT);
  pinMode(BLUEPIN, OUTPUT);
  pinMode(PIRpin, INPUT);
  tmrpcm.speakerPin = 9; //5,6,11 or 46 on Mega, 9 on Uno, Nano, etc
  //Complimentary Output or Dual Speakers:
  //pinMode(10,OUTPUT); Pin pairs: 9,10 Mega: 5-2,6-7,11-12,46-45 
  // get background reading for normalizing mic signal
  int cnt = 0;
  for (cnt; cnt<250; cnt++) {
    sensorValue = analogRead(sensorPin);
    EMA_NSV_long += abs(float(sensorValue) * maxAnWr / maxAnalog);
  }
  EMA_NSV_long = EMA_NSV_long / float(cnt);
  
  Serial.begin(9600);
  Serial.println("");
  Serial.println(EMA_NSV_long);
  if (!SD.begin(SD_ChipSelectPin)) {  // see if the card is present and can be initialized:
    Serial.println("SD fail");  
    return;   // don't do anything more if not

  }
  else{   
    Serial.println("SD ok");   
  }
  tmrpcm.play("cackle.wav"); //the sound file "music" will play each time the arduino powers up, or is reset
}

void loop(){
  if (millis() - lastTrigger > PIRdelay) {
    PIRread = digitalRead(PIRpin);
    if (PIRread == HIGH) {
    Serial.println("PIR triggered");
    Serial.println(millis() - lastTrigger);
    soundFiles[random(sizeof(soundFiles)/sizeof(String))].toCharArray(filename, 30);
    tmrpcm.play(filename);
    lastTrigger = millis();
    PIRhigh = true;
    }
  }
  
  sensorValue = analogRead(sensorPin);
  normSensorVal = abs((float(sensorValue)  * maxAnWr / maxAnalog));
  EMA_NSV = alpha*(abs(normSensorVal - EMA_NSV_long)) + (1-alpha) * EMA_NSV;
  if (tmrpcm.isPlaying()) {
    sensorValue = analogRead(sensorPin);
    normSensorVal = abs((float(sensorValue)  * maxAnWr / maxAnalog));
    EMA_NSV = alpha*(abs(normSensorVal - EMA_NSV_long)) + (1-alpha) * EMA_NSV;
    analogWrite(GREENPIN, 0);
    analogWrite(BLUEPIN, 0);
    analogWrite(REDPIN, min(255, pow(int(EMA_NSV),4))); // todo: make this power tunable by a variable resistor
    Serial.println(pow(int(EMA_NSV),4)); // or better yet, auto adjust based on max volume of EMA_NSV
    Serial.println(EMA_NSV);
  } else {
    Serial.println("sensor val");
    Serial.println(sensorValue);
    Serial.println(normSensorVal);
    Serial.println(EMA_NSV_long);
    Serial.println(EMA_NSV);
    swirlLEDs();
    EMA_NSV_long = longalpha*normSensorVal + (1-longalpha) * EMA_NSV_long;
  }

  if(Serial.available()){    
    switch(Serial.read()){
    case 'd': tmrpcm.play("laugh2.wav"); break;
    case 'P': tmrpcm.play("cackle.wav"); break;
    case 't': soundFiles[random(sizeof(soundFiles)/sizeof(String))].toCharArray(filename, 30); tmrpcm.play(filename); break;
    case 'p': tmrpcm.pause(); break;
    case '?': if(tmrpcm.isPlaying()){ Serial.println("A wav file is being played");} break;
    case 'S': tmrpcm.stopPlayback(); break;
    case '+': tmrpcm.volume(1); break;
    case '-': tmrpcm.volume(0); break;
    case '0': tmrpcm.quality(0); break;
    case '1': tmrpcm.quality(1); break;
    default: break;
    }
  }

}

void swirlLEDs() {
  if (millis() - lastLEDchange > LEDdelay) {
  lastLEDchange = millis();
  if (LEDchangeCnt>LEDcoherence) {
    color = random(3);
    LEDchangeCnt = 0;
  } else {
    LEDchangeCnt++;
  }
  switch (color) {
    case 0: {
      if (redup) {
        red++;
        if (red == 255) {
          redup = false;
        }
      } else {
        red--;
        if (red == 0) {
          redup = true;
        }
      }
    }
    case 1: {
      if (grnup) {
        grn++;
        if (grn == 255) {
          grnup = false;
        }
      } else {
        grn--;
        if (grn == 0) {
          grnup = true;
        }
      }
    }
    case 2: {
      if (bluup) {
        blu++;
        if (blu == 255) {
          bluup = false;
        }
      } else {
        blu--;
        if (blu == 0) {
          bluup = true;
        }
      }
    }
    analogWrite(BLUEPIN, blu);
    analogWrite(GREENPIN, grn);
    analogWrite(REDPIN, red);
  }
  }
}
