 // Author: Juan Carlos Juárez
 // Sensors & Actuators: Smart Door
 // https://github.com/jc-juarez

#include <Keypad.h>
#include <ESP32Servo.h>

#define BLYNK_PRINT Serial
#define ROW_NUM     4 // four rows
#define COLUMN_NUM  4 // four columns
#define timeInterval 7
#define PIR 4
#define led 16

#include <WiFi.h>
#include <WiFiClient.h>
char ssid[] = "INFINITUM15A2_2.4";
char pass[] = "2f887qyAcd";

#include <BlynkSimpleEsp32.h>
char auth[] = "7vdOwO96eTFCToq5h8fDDLi4dqKzAySi";

char keys[ROW_NUM][COLUMN_NUM] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte pin_rows[ROW_NUM]      = {13, 12, 14, 27}; // GIOP19, GIOP18, GIOP5, GIOP17 connect to the row pins
byte pin_column[COLUMN_NUM] = {26, 25, 33, 32};   // GIOP16, GIOP4, GIOP0, GIOP2 connect to the column pins

Keypad keypad = Keypad( makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM );

Servo myservo;

int pos = 0;    // variable to store the servo position
// Recommended PWM GPIO pins on the ESP32 include 2,4,12-19,21-23,25-27,32-33 
int servoPin = 2;


String password = "A"; // change your password here
String input_password;
String currState = "0";
String notificationOpen = "0"; // 0 for Active, 1 for Non Active
String notificationClosed = "0";
String notificationSensor = "0";
String systemControl = "0";
int firstIteration = 1;


unsigned long now = millis();
unsigned long lastTrigger = 0;

void openDoor(){
  digitalWrite(led, LOW);
  for (pos = 90; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
          myservo.write(pos);    // tell servo to go to position in variable 'pos'
          delay(15);             // waits 15ms for the servo to reach the position
  }
}

void closeDoor(){
  digitalWrite(led, HIGH);
  for (pos = 0; pos <= 90; pos += 1) { // goes from 0 degrees to 180 degrees
          // in steps of 1 degree
          myservo.write(pos);    // tell servo to go to position in variable 'pos'
          delay(15);             // waits 15ms for the servo to reach the position
  }
}

BLYNK_WRITE(V1)
{
  password = param.asStr();
  Blynk.notify("Password has been succesfully changed.");
}

BLYNK_WRITE(V2)
{
  String state = param.asStr();
  //Serial.println(param.asStr());
  if(state == "1" && currState != "1"){
    openDoor();
  }else if(state == "0" && currState != "0"){
    closeDoor();
  }
  currState = state;
}

BLYNK_WRITE(V3)
{
  notificationOpen = param.asStr();
}

BLYNK_WRITE(V4)
{
  notificationClosed = param.asStr();
}

BLYNK_WRITE(V5)
{
  notificationSensor = param.asStr();
}

BLYNK_WRITE(V6)
{
  systemControl = param.asStr();
}

void setup() {
  Serial.begin(9600);
  Blynk.begin(auth, ssid, pass);
  input_password.reserve(32); // maximum input characters is 33, change if needed
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  myservo.setPeriodHertz(50);    // standard 50 hz servo
  myservo.attach(servoPin, 500, 2400);
  pinMode(PIR, INPUT_PULLUP);// define pin as Input  sensor
  pinMode(led, OUTPUT);
  digitalWrite(led, HIGH);
}

void loop() {
  Blynk.run();
  int motion =digitalRead(PIR);
  if(motion){
    now = millis();
    if(notificationSensor != "1" && (firstIteration == 1 || (now - lastTrigger > (timeInterval*1000)))){
      Blynk.notify("Alert: Movement detected near door.");
      Serial.println("MOTION DETECTED!!!");
      lastTrigger = millis();
      firstIteration = 0;
    }
  }
  if(systemControl != "1"){
    //*****
    char key = keypad.getKey();
    if (key) {
      Serial.println(key);
  
      if (key == '*') {
        input_password = ""; // clear input password
        if(currState != "0"){
            closeDoor();
        }
        currState = "0";
      } else if (key == '#') {
        if (password == input_password) {
          Serial.println("The password is correct, ACCESS GRANTED!");
          // DO YOUR WORK HERE
          if(currState != "1"){
            if(notificationOpen != "1") Blynk.notify("Alert: Cupboard Door has been opened.");
            openDoor();
          }
          currState = "1";
        } else {
          digitalWrite(led, LOW);
          delay(200);
          digitalWrite(led, HIGH);
          delay(200);
          digitalWrite(led, LOW);
          delay(200);
          digitalWrite(led, HIGH);
          delay(200);
          digitalWrite(led, LOW);
          delay(200);
          digitalWrite(led, HIGH);
          delay(200);
          if(notificationClosed != "1") Blynk.notify("Alert: Someone has tried to open the door.");
          Serial.println("The password is incorrect, ACCESS DENIED!");
        }
  
        input_password = ""; // clear input password
      } else {
        input_password += key; // append new character to input password string
      }
    }
    //*****
  }
}
