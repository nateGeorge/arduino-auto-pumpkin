#include <SD.h>                      // need to include the SD library
//#define SD_ChipSelectPin 53  //example uses hardware SS pin 53 on Mega2560
#define SD_ChipSelectPin 4  //using digital pin 4 on arduino nano 328, can use other pins
#include <TMRpcm.h>           //  also need to include this library...
#include <SPI.h>

/* LED stuff */
#define REDPIN 6
#define GREENPIN 5
#define BLUEPIN 3
int r, g, blu;

TMRpcm tmrpcm;   // create an object for use in this sketch

unsigned long time = 0;

int greenLEDpin = 6;
int greenLevel = 10;

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
float background = 80; // background noise level for mic
float normSensorVal;
float EMA_NSV = 0; // exp mov avg of normalized sensor value
float period = 128; // exponential moving average period
float alpha = 1/period;
float EMA_NSV_long = 80; // exp mov avg of normalized sensor value
float longperiod = 6400; // exponential moving average period
float longalpha = 1/longperiod;
int spkNegPin = 2;

String soundFiles[4] = {"laugh.wav", "laugh2.wav", "scream.wav", "cackle.wav"};
char filename[30];

void setup(){
  pinMode(REDPIN, OUTPUT);
  pinMode(GREENPIN, OUTPUT);
  pinMode(BLUEPIN, OUTPUT);
  pinMode(PIRpin, INPUT);
  pinMode(greenLEDpin, OUTPUT);
  pinMode(spkNegPin, OUTPUT);
  tmrpcm.speakerPin = 9; //5,6,11 or 46 on Mega, 9 on Uno, Nano, etc
  //Complimentary Output or Dual Speakers:
  //pinMode(10,OUTPUT); Pin pairs: 9,10 Mega: 5-2,6-7,11-12,46-45 
  
  Serial.begin(9600);
  //pinMode(13,OUTPUT); //LED Connected to analog pin 0
  if (!SD.begin(SD_ChipSelectPin)) {  // see if the card is present and can be initialized:
    Serial.println("SD fail");  
    return;   // don't do anything more if not

  }
  else{   
    Serial.println("SD ok");   
  }
  digitalWrite(spkNegPin, HIGH);
  tmrpcm.play("cackle.wav"); //the sound file "music" will play each time the arduino powers up, or is reset
}

void loop(){
  if (tmrpcm.isPlaying()) {
    digitalWrite(spkNegPin, 1);
  }
  else {
    //digitalWrite(spkNegPin, 0);
  }
  PIRread = digitalRead(PIRpin);
  if (PIRread == HIGH && millis() - lastTrigger > PIRdelay) {
    Serial.println("PIR triggered");
    Serial.println(millis() - lastTrigger);
    soundFiles[random(sizeof(soundFiles)/sizeof(String))].toCharArray(filename, 30);
    tmrpcm.play(filename);
    lastTrigger = millis();
    PIRhigh = true;
  }
  
  sensorValue = analogRead(sensorPin);
  normSensorVal = pow(abs((float(sensorValue)  * maxAnWr / maxAnalog)) - EMA_NSV_long,3);
  //Serial.println((float(sensorValue)  * maxAnWr / maxAnalog));
  //Serial.println(normSensorVal);
  EMA_NSV = alpha*normSensorVal + (1-alpha) * EMA_NSV;
  EMA_NSV_long = longalpha*normSensorVal + (1-longalpha) * EMA_NSV_long;
  
  analogWrite(REDPIN, int(EMA_NSV));

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
    case '2': {
      if (greenLevel > 0) {
        greenLevel -= 1;
        analogWrite(greenLEDpin, greenLevel);
        Serial.println(greenLevel);
        //digitalWrite(greenLEDpin, HIGH);
      }
      break;
    }
    case '8': {
    if (greenLevel < 255) {
        greenLevel += 1;
        analogWrite(greenLEDpin, greenLevel);
        Serial.println(greenLevel);
        //digitalWrite(greenLEDpin, LOW);
      }
      break;
    }
    default: break;
    }
  }

}
