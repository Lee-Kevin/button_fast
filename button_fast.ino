// This is a demo by Jiankai.li

/* Each state switch will be subject to the buzzer alarm */
#include "pitches.h"
#include <Adafruit_NeoPixel.h>
// #include <EEPROM.h>
#include <Servo.h>

/*
  value = EEPROM.read(address);
*/

enum Status 
{
  Standby = 0,               /* Breathing lights flashing */
  Working = 1,               /* Game Go */
  Failed  = 2,               /* Failed to break records */
  Success = 3,               /* Successfully break the record */
};
typedef enum Status SystemStatus;
SystemStatus WorkingStatus;

#define LedBarPin      5
#define NUMPIXELS      5
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, LedBarPin, NEO_GRB + NEO_KHZ800);

#define ServoPin       7       
Servo myservo;

#define ButtonPin      2

#define LEDPin         4

#define BuzzerPin      8

#define TimeInterval   10000  /* 10s Game duration */

#define RefreshTime    150

#define MaxNumber      120
#define Srevoposition  165
/*
    pixels.setPixelColor(1, pixels.Color(0,150,0)); // Moderately bright green color.
    pixels.show();
*/

volatile int num = 0;
volatile int flg = 0;
uint32_t LastRecord = 0;
unsigned long nowtime = 0;
unsigned long ServoTime = 0;
volatile int Ledstate = LOW;

int melody[] = {
  NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4
};

int noteDurations[] = {
  4, 8, 8, 4, 4, 4, 4, 4
};

void setup()
{
    Serial.begin(115200);
    pixels.begin();
    pinMode(ButtonPin, INPUT);
    pinMode(LEDPin,OUTPUT);
    attachInterrupt(0, Buttonclick, FALLING);
    myservo.attach(ServoPin);
    delay(15);
    myservo.write(Srevoposition);
    // EEPROM.write(0,15);
    digitalWrite(LEDPin,Ledstate);
    pinMode(BuzzerPin, OUTPUT);
}


void loop()
{
  switch (WorkingStatus) {
  case Standby:
    if (num != 0) {
      WorkingStatus = Working;
      Serial.println("Standby Next state Working");
      // LastRecord = EEPROM.read(0);
      myservo.attach(ServoPin);                        /* attach the servo pin again */
      nowtime = millis();
      ServoTime = millis();
      num = 1;
    } else {                                           /* Breathing light effect */
      action_rgbled_on(NUMPIXELS);
      myservo.write(Srevoposition);
      myservo.detach();
      Ledstate = LOW;
      digitalWrite(LEDPin,Ledstate);

    }
    break;
  case Working:
    if((millis() - nowtime) >= TimeInterval) {          /* Timeout */
      detachInterrupt(0);                              /* Temporary no interrupt */
      if(num > MaxNumber) {                       /* New record */
        WorkingStatus = Success;
        Serial.println("Working Next state Success");
      } else {
        WorkingStatus = Failed;                        /* Failed     */
        Serial.println("Working Next state Failed");
      }
    } else {                                           /* no time out */
        if((millis() - ServoTime) >= RefreshTime) {
          float angle = (Srevoposition - num*1.25);
          if(angle <= 0) {
             angle = 0;
          }
          myservo.attach(ServoPin);
          delay(15);
          myservo.write(angle);
          delay(15);
          myservo.detach(); 
          
          ServoTime = millis();
        } else {
          
        }

    }
    break;
  case Failed:
    Serial.println("Failed Next state Standby");
    WorkingStatus = Standby;
    FailedMusicPlay();
    myservo.attach(ServoPin); 
    delay(15);
    myservo.write(Srevoposition);
    delay(800);
    attachInterrupt(0, Buttonclick, FALLING);
    num = 0;
    break;
  case Success:
    Serial.println("Success Next state Standby");
    WorkingStatus = Standby;
    num = 0;
    SuccessMusicPlay();
    attachInterrupt(0, Buttonclick, FALLING);
    num = 0;
    break;
  default:
    WorkingStatus = Standby;
    break;
  }
}


void Buttonclick()
{
  int buttonState = digitalRead(ButtonPin);
  if(buttonState == 0) {
    delay(70);
    digitalWrite(LEDPin, Ledstate);
    Ledstate = !Ledstate;
    if(buttonState == 0) {
      num++;
      Serial.print("num: ");
      Serial.println(num);
    } else {
    }
  } else {
    
  }
  Serial.print("buttonState: ");
  Serial.println(buttonState);
}


void action_rgbled_on (int led_num)
{
    int i = 0;
    int j = 0;
    uint32_t time = millis();       
    uint8_t phase = (time >> 4) ;  
    uint8_t lightness = 4;    
    float h = phase / 256.0;
    float s = 1.0;
    float v = 1.0;
    int i_ = floor(h * 6);
    float f = h * 6 - i_;
    float p = v * (1 - s);
    float q = v * (1 - f * s);
    float t = v * (1 - (1 - f) * s);
    float r = 0, g = 0, b = 0;
    switch(i_ % 6){
        case 0: r = v; g = t; b = p; break;
        case 1: r = q; g = v; b = p; break;
        case 2: r = p; g = v; b = t; break;
        case 3: r = p; g = q; b = v; break;
        case 4: r = t; g = p; b = v; break;
        case 5: r = v; g = p; b = q; break;
                    }                    
    uint8_t  rgb_color[3] ;
    rgb_color[0] = r * 255;
    rgb_color[1] = g * 255;    
    rgb_color[2] = b * 255;    
    rgb_color[0] = rgb_color[0] >>lightness;
    rgb_color[1] = rgb_color[1] >>lightness;    
    rgb_color[2] = rgb_color[2] >>lightness;        
        for(int i=0;i<led_num;i++) {
            pixels.setPixelColor(i, pixels.Color(rgb_color[0],rgb_color[1],rgb_color[2])); // Moderately bright green color.
            pixels.show(); // This sends the updated pixel color to the hardware.
            delay(5); // Delay for a period of time (in milliseconds).
        }
}

void SuccessMusicPlay(void) 
{
  for(int i = 0;i<2; i++) {
    for (int thisNote = 0; thisNote < 8; thisNote++) {
    digitalWrite(LEDPin,Ledstate);
    // to calculate the note duration, take one second
    // divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int noteDuration = 1000 / noteDurations[thisNote];
    tone(8, melody[thisNote], noteDuration);

    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    // stop the tone playing:
    noTone(8);
    Ledstate = !Ledstate;
   }
  }
}

void FailedMusicPlay(void) 
{
  for(int i = 0;i<3; i++) {
      digitalWrite(BuzzerPin, HIGH);   // turn the LED on (HIGH is the voltage level)
      digitalWrite(LEDPin,HIGH);
      delay(300);              // wait for a second
      digitalWrite(BuzzerPin, LOW);    // turn the LED off by making the voltage LOW
      digitalWrite(LEDPin,LOW);
      delay(300);              // wait for a second
  }
}







