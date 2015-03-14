/*---------------[SCREEN LIFTER ver 2.3 BY BENJAMIN DAVOULT]-----------------
This program has 3 purposes:

-lift up and down my wood framed screen in my home theatre room...whitch is also my bedroom.
-control a LED tape with a 10Kohm trimpot to enlight the room just a bit before bedtime with 4
selectable timers.
-cool the NAS server, DUNE HD player, XBOX etc... with 2 computer fans (I use 2 NOCTUA 60mm
they are fantastics).

note: the system will use the LED tape during the lifting of the screen to act as a safety light
like a "guys, be carefull there is a geek inside this room who make crazy things!"... 
You know what I mean

my screen wheigt about 4kg, and to lift it I use the Hitec HS-805 converted for continuous
rotation (a lot of tuto for this on the internet, its quite easy).
with a 1cm radius spool, it can lift up to 20kg, so it is usually called "the monster".

I started arduino (and programmation in general) about 1 month ago, so I am sure that my program
is terrible and that it is possible to make it FAR more simple...But you know waht? it works
just fine so its good enough for me!
but of course if you have some ideas to make it better, simpler or whatever, please let me know!

The runningMedian library is the one found on the arduino playground at
http://playground.arduino.cc/Main/RunningMedian
the credit goes to robtillaart.
there is no download file on the page, so you have to copy the code of the .h and .cpp,
create files and put it inside.

I dedicate this program to the community, so no copyright.
Do what you want with it.

2014-09-15

---------------------[Benn25 (atmk) gmail.com]-----------------------

Tthe cool stuff starts here:
*/

#include <RunningMedian.h>
//#include <Servo.h>
#include <PWM.h>

RunningMedian samples = RunningMedian(15); //25 samples for the median of the temperatures
//this helps reducing the noise of the sensor if you have a long wire... like me
//use a shilded wire also helps
RunningMedian samples2 = RunningMedian(20);

RunningMedian samplesLED = RunningMedian(8);//25 samples for the median of the pot values
//to reeduce noise and smooth the transitions in the change of light values


//Servo servo; //winch
//Servo Lservo; //servo to lock the winch

#define swlow 3 //lower microswitch
#define swuser 4 //user pushbutton
#define swup 7 //upper microswitch
#define relay 8 //relay for the 2 servos
#define FAN 11 //TIP120 darlington transostor to control the fans
#define led 9 //TIP120 darlington transostor to control the LED tape
#define servo 10
#define Lservo 6

int myAngle = 0;
int pulseWidth = 0;
int pulseWidth2 = 0;
  const long delayTime = 8;
  boolean flag = false;
  int spd = 0; //spd of the fan
  float temp;
  float temp2;
  int tempPin = 0;
  int state = 0; //the state of the screen for the switch case. can be 0, 1 or 2
  int prevstate = 0; //previous state of the screen, to reset the servotime
  unsigned long time =0; //time to reset the user button
  int lockangle = 115; //locked position of the lock servo
  int unlockangle = 60; //unlocked position
  int potar = 0; //trimpot for the light remapped 0-255
  long potar0 = 0; ////trimpot for the light not remapped
  boolean moving = false; //screen moving or not?
  unsigned long time2 = 0; //timer to avoid a delay for the fading of the LED
  unsigned long time3 = 0; //timer to keep the LED on briefely before fading out
  int brightness = 0; //brightness of the LED (no kidding)
  boolean up = true; //true when the LED fade in, false when fading off
  boolean trig = false; //trigger to avoid the repetition of the instructions when the screen starts to move
  boolean templock = false; //temperature threshold for the fans
  unsigned long fantime = 0; //timer to put a delay without delay() between 2 changes of the fanspeed
  unsigned long timetemp = 0;
  unsigned long pottime = 0; //light timer
  int prevpot = 0; //previous value of the potar
  int lightdim = 0; //smooth the shutting off of the light
  boolean flaglow; //debouncer for the upper switch (make the upper sw operable only once)
  int remain = 30;
  int lightzone = 1;
  unsigned long lightoff = 1000;
  int prevzone;
  unsigned long servotime = 0;
  unsigned long servotime2 = 0;
void setup(){
  
   TCCR2B = TCCR2B & 0b11111000 | 0x01;   
   /*
   accellerate the PWM freq of the timer 2 (pin 11 and 3)
   to the maximum (multiplication factor set to 1), so the frequency is close to 25khz
   If you dont do this, the fans will become as loud as a noisy speaker,
   but 25khz cant be heard by the human ear...but I cant tell for your pets!
  */
  
  InitTimersSafe(); 
  
  if(swup == HIGH){
    flaglow = false;}
  if(swup == LOW){
  flaglow = true;
  }
  
 //Serial.begin(115200);//debug
  pinMode(FAN, OUTPUT);
  pinMode(led, OUTPUT);
  pinMode(swlow, INPUT);
  pinMode(swuser, INPUT);
  pinMode(swup, INPUT);  
  pinMode(relay, OUTPUT);
    pinMode(servo, OUTPUT);
    pinMode(Lservo, OUTPUT);
  //servo.attach(10); //winch
  //Lservo.attach(6); //Lock
 
}

void servoPulse(int servoPin, int myAngle, int servoPin2, int myAngle2) {

  pulseWidth = (myAngle * 11) + 500; // converts angle to microseconds.
  pulseWidth2 = (myAngle2 * 11) + 500;
  digitalWrite(servoPin, HIGH);    // set servo high.
  delayMicroseconds(pulseWidth);     // wait a very small amount.
  digitalWrite(servoPin, LOW);       // set servo low.
  digitalWrite(servoPin2, HIGH);    // set servo high.
  delayMicroseconds(pulseWidth2);     // wait a very small amount.
  digitalWrite(servoPin2, LOW);       // set servo low.
  delay(delayTime);                  // refresh cycle of typical servos.
}

/*
void ledFlash(int delin, int delout){//if delin is big, it will take time to light up,
  //same thing for delout with the fadeout time. 0 is OK, its quite close to instantaneous.
 
  if(up==true){
  analogWrite(led, brightness);    
  if(time2 + delin < millis()){
  brightness+=3;
  time2=millis();
   if(brightness>149){//100 is enough for me
     up = false;
     time3 = millis();}}
}

if(time3 + 200 < millis()){//this is to keep the light up a brief time before fading out 

if(up==false){
  analogWrite(led, brightness);    
  if(time2 + delout < millis()){
  brightness-=3;
    time2=millis();
   if(brightness<1){
     up = true;}}}}

}
*/

void fanspeed(){
   if(temp>30)
     templock = true;
   if(temp<20)
     templock = false; //I create a threshold between 20 and 30deg with a trigger
     //to avoid the fans to start and stop repetitively when the temp is just touching the limit  
  
 if(digitalRead(swlow) == HIGH){//screen low, no prob, the fan is always spining
   spd = map(temp, 25, 50, 3, 18);
   spd = constrain(spd, 3, 20);
   analogWrite(FAN,spd);}
 
 if(temp<=30 && templock == false && digitalRead(swlow) == LOW){//screen up but cold
   digitalWrite(FAN,LOW);}
   
 if(templock == true && digitalRead(swlow) == LOW){//screen up and hot
   spd = map(temp, 30, 50, 3, 12);
   spd = constrain(spd, 3, 12);
   analogWrite(FAN,spd);    
 }
}

void light(unsigned long remmin){
 delay(15);
 //Serial.print("light= ");
 //Serial.println(potar);
 remmin*=60000;//convert minutes in millisec
 
   if (potar < prevpot-1 || potar > prevpot +1){//the user has touched the pot
   //I applied a small threshold to prevent noise from cheap pot to reset the timer w/o touching it
    pottime=millis();//reset the timer
    }
    
   if (pottime + remmin > millis()){ //timer active
    pwmWriteHR(led, potar0); //light ON
    lightdim = potar0;  
    }
      
   if (pottime + remmin < millis() && lightdim >= 1 ){ //end of timer, 1800000 is for 30 min
    pwmWriteHR(led, lightdim); //dim the light until 0
//       Serial.print("lightdim= ");
//    Serial.println(lightdim);
//          Serial.print("lightoff= ");
//    Serial.println(lightoff);
    if (lightoff+32000/(lightdim*10+900) < millis()){
    lightdim-=(lightdim/100+1);
    lightoff=millis();

    }
   }
   if(lightdim <= 0){
      digitalWrite(led, LOW);
    }
 // Serial.print("light= ");
 //Serial.println(potar0);
}

void timechoice(){
  
  if(potar < 15){
    lightzone = 1;
   if (lightzone != prevzone){
 remain = 15;
 digitalWrite(led, LOW);
  delay(100);
 analogWrite(led,50);
  delay(200);
  digitalWrite(led, LOW);
  delay(300);
   }
    //signal lumineux pour 15 min
 }
 if(potar >= 15 && potar < 40){
    lightzone = 2;
   if (lightzone != prevzone){
   remain = 25;
   digitalWrite(led, LOW);
  delay(100);
  analogWrite(led,50);
  delay(200);
  digitalWrite(led, LOW);
  delay(150);
  analogWrite(led,50);
  delay(200);
  digitalWrite(led, LOW);
  delay(300);
   }
 //signal lumineux pour 25 min
 }
 if(potar >= 40 && potar < 200){
    lightzone = 3;
   if (lightzone != prevzone){
   remain = 45;
   digitalWrite(led, LOW);
  delay(100);
  analogWrite(led,50);
  delay(400);
  digitalWrite(led, LOW);
  delay(150);
  analogWrite(led,50);
  delay(200);
  digitalWrite(led, LOW);
  delay(150);
  analogWrite(led,50);
  delay(200);
  digitalWrite(led, LOW);
  delay(300);
   }
 //signal lumineux pour 45 min
 }
 if(potar >= 200){
    lightzone = 4;   
    if (lightzone != prevzone){
   remain = 150;
   digitalWrite(led, LOW);
  delay(100);
  analogWrite(led,50);
  delay(400);
  digitalWrite(led, LOW);
  delay(150);
  analogWrite(led,50);
  delay(400);
  digitalWrite(led, LOW);
  delay(150);
  analogWrite(led,50);
  delay(300);
  digitalWrite(led, LOW);
  delay(150);
  analogWrite(led,50);
  delay(200);
  digitalWrite(led, LOW);
  delay(300);
   }
 //signal lumineux pour 150 min
 } 
 //Serial.println(lightzone);
prevzone = lightzone;
}

void loop(){
  
  //Serial.println(flaglow);
  
  if(timetemp + 800 < millis()){
  temp2 = analogRead(tempPin);
  temp2 = temp2 * 0.48828125; //formula for my temp sensor, the LM35: (5.0 * 1000 / 1024) / 10 = 0.48828125
  samples.add(temp2);//record temp samples for the running median library...
  temp2 = samples.getMedian();
  samples2.add(temp2);
  temp = samples2.getMedian();
  
//Serial.print("speed: ");  
//Serial.println(spd);//debug
//Serial.print("templock: ");
//Serial.println(templock);//debug
//Serial.print("temp sonde: ");
//Serial.println(analogRead(tempPin) * 0.48828125);//debug
//Serial.print("temp premier filtre: ");
//Serial.println(temp2);//debug
//Serial.print("temp filtre final: ");
//Serial.println(temp);//debug
//Serial.println("------------");

timetemp = millis();
}
  
  potar0 = analogRead(2);
  samplesLED.add(potar0);//record samples for the pot...
  potar0 = samplesLED.getMedian();//...and get the result here
  potar = map(potar0, 0, 1023, 0, 255);
  //potar0 = map(potar0,0, 1023, 0, 32767);
  //potar0 = pow(potar0,2);
 potar0 = potar0 + potar0*potar0/25;
 potar0 = map(potar0, 0, 42884, 0 ,20200);
// potar0 = map(potar0,0,24346,0,32000);
  //  Serial.println(potar0);
 

if(digitalRead(swuser) == LOW && (digitalRead(swup)!=digitalRead(swlow)) && flag == false){
  if(state == 1 && digitalRead(swup) == HIGH){
    state=1;
  }
  else{
  state = 0; //state 0 for the stable state of the screen, up or low
  }
}

if(digitalRead(swuser) == HIGH && flag == false){ //pushing the user button
//the booleans will automatically shift back, so debounce is useless here
  flag = true; //possible move of the screen for 2500ms
  time = millis(); //record the time
  trig = true;//boolean trigger to avoid the repeat of the starting process
  // of the states 1 and 2 to get rid of the delays at those times
}

if(digitalRead(swup) == HIGH && flag == true){
    moving = true; //the screen is mooving
  state = 1; //state 1 for the lift off of the screen
}


if(digitalRead(swlow) == HIGH && flag == true){
  moving = true; //the screen is mooving
  state = 2; //state 2 for lift up the screen
}

if (time + 2500 < millis()){
 flag = false; //end of possibility to put the screen in move, but stopping is possible
}

if (prevstate != state){
    servotime = millis(); //reset timer for the servo move
    //Serial.println(servotime);
}

switch(state){
  case 0:
  if(fantime + 25000 < millis()){//wait 25 sec between every fanspeed change to avoid too
  //much speed variations
  fanspeed();
  fantime = millis();}
  
  if(moving == true && digitalRead(swup) == HIGH && flaglow == true){ //STOP in UPPER position
  
  if (servotime + 230 > millis()){
    servoPulse(servo, 100, Lservo, lockangle);
  }
  if (servotime + 230 <= millis()){    
  //Lservo.write(lockangle);//lock
  //servo.write(105);//stop mouving servo winch
  //delay(350); //hold briefly the move to keep the switch strongly pushed
  //servo.write(93);//stop mouving servo winch
  //delay(300);//small delay to be sure that everyone is steady in place
    servoPulse(servo, 93, Lservo, lockangle);
    digitalWrite(relay, LOW); //relay OFF
    moving = false;
     // Serial.println("moving false");
      flaglow = false;
  }
    }
  
  if(moving == true && digitalRead(swlow) == HIGH){ //STOP in LOWER position
     if (servotime + 300 > millis()){
       servoPulse(servo, 93, Lservo, unlockangle);
     }
     if (servotime + 300 <= millis()){
       servoPulse(servo, 93, Lservo, lockangle);
     }
     if (servotime + 360 <= millis()){    
  //delay useless here thanks to the gravity pulling the screen down,
  //so the switch will be firmly pushed even if we stop without delay
  //servo.write(93);//stop moving servo winch
  //delay(150);//to be sure that the winch in stopped before the lock
  //Lservo.write(lockangle);//lock
  //delay(300);//small delay to be sure that everyone is steady in place
  digitalWrite(relay, LOW); //relay OFF
  moving = false;
 // Serial.println("moving false");
  }
  }
  
  if(moving == false){
     //Serial.println("go");
   if(digitalRead(swup) != digitalRead(swlow)){
    light(remain);
   }
   if(digitalRead(swup) == HIGH && digitalRead(swlow) == HIGH){
     timechoice();
   }
    
  }
//  Serial.print("up ");
//  Serial.println(digitalRead (swup));
//  Serial.print("low ");
//  Serial.println(digitalRead (swlow));
  
  break;
  
  case 1:   //RELAY ON and lift off
  //if(trig == true)
  digitalWrite(FAN,LOW);
  digitalWrite(led, HIGH);
  if (servotime + 500 <= millis() && servotime + 900 > millis()){
      servoPulse(servo, 120, Lservo, unlockangle);
    
  //delay(500); //get away from the screen !
  //servo.write(120);//return the spool a bit to desengage the lock
  digitalWrite(relay, HIGH);
  }
  //Lservo.write(unlockangle); //unlock
  //delay(400);
    if (servotime + 900 <= millis()){
      //servoPulse(Lservo, unlockangle);
      servoPulse(servo, 74, Lservo, unlockangle);
    }

  //servo.write(79);//unspool the servo winch slowly
  //trig = false;

//if(trig == false)
//ledFlash(0,15);
  break;
  
  case 2:   //RELAY ON and lifting up
  //if(trig == true)
  flaglow = true;
  digitalWrite(FAN,LOW);
  templock = false; //pass the temp treshold to make the fan slower if the temp is between 20 and 30

  if (servotime + 1500 <= millis()){
      servoPulse(servo, 180, Lservo, unlockangle);
  //delay(1500); //get away from the screen !
  //Lservo.write(unlockangle);
  digitalWrite(relay, HIGH);
  }
  
  //delay(200);
  //servo.write(180);//spooling the winch
  //trig = false;
  
  //if (trig == false)
  //ledFlash(10,0);
  digitalWrite(led, HIGH);
  break;  
}
    //  Serial.println(millis()-servotime2);
     // servotime2=millis();
   //   Serial.print("case");
    //  Serial.println(state);
      
      
  prevpot = potar;
prevstate = state;
  }
