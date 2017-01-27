/*
A simple clock based on the Adafruit RTC, Adafruit Thermal Recepit Printer Guts and an Arduino UNO. 

This softare uses the Adafruit Thermal Printer Library and the Adafruit RTC library.

Hardware Description:
Setup for the printer is based on SofwareSerial and uses pins 9 and 10 for RX and TX respectively.
Pin 11 is wired to a momentary contact switch and ground initialized to use internal pullup.

The printer will print the time every five minutes and when the button is pushed. A long push of the
button will also print the current date. The time is set on the RTC module according to the system time 
when the code is compiled. There is currently no method to set the time manually.
*/

// Date and time functions using a DS3231 RTC connected via I2C and Wire lib
#include "Adafruit_Thermal.h"
#include <Wire.h>
#include "RTClib.h"
#include "shrug.h"

RTC_DS1307 rtc;

char* months[13] = {"\0", "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"}; 
char* dayNames[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
char* hours[24] = {"Midnight", "One", "Two", "Three", "Four", "Five", "Six", "Seven", "Eight", "Nine", "Ten", "Eleven", "Noon", "One", "Two", "Three", "Four", "Five", "Six", "Seven", "Eight", "Nine", "Ten", "Eleven"};
char* dayNums[32] = {"\0", "1st", "2nd", "3rd", "4th", "5th", "6th", "7th", "8th", "9th", "10th", "11th", "12th", "13th", "14th", "15th", "16th", "17th", "18th", "19th", "20th", "21st", "22nd", "23rd", "24th", "25th", "26th", "27th", "28th", "29th", "30th", "31st"};
char* minutes[60] = {"\0", "One", "Two", "Three", "Four", "Five", "Six", "Seven", "Eight", "Nine", 
"Ten", "Eleven", "Twelve", "Thirteen", "Fourteen", "Quarter", "Sixteen", "Seventeen", "Eighteen", "Nineteen", 
"Twenty", "Twenty-one", "Twenty-two", "Twenty-three", "Twenty-four", "Twenty-five", "Twenty-six", "Twenty-seven", "Twenty-eight", "Twenty-nine", 
"Half", "Thirty-one", "Thirty-two", "Thirty-three", "Thirty-four", "Thirty-five", "Thirty-six", "Thirty-seven", "Thirty-eight", "Thirty-nine",
"Twenty", "Nineteen", "Eighteen", "Seventeen", "Sixteen", "Quarter", "Fourteen", "Thirteen", "Twelve", "Eleven",
"Ten", "Nine", "Eight", "Seven," "Six", "Five", "Four", "Three", "Two", "One"};

// Here's the new syntax when using SoftwareSerial (e.g. Arduino Uno) ----
// If using hardware serial instead, comment out or remove these lines:

#include "SoftwareSerial.h"
#define TX_PIN 10 // Arduino transmit  YELLOW WIRE  labeled RX on printer
#define RX_PIN 9 // Arduino receive   GREEN WIRE   labeled TX on printer

SoftwareSerial mySerial(RX_PIN, TX_PIN); // Declare SoftwareSerial obj first
Adafruit_Thermal printer(&mySerial);     // Pass addr to printer constructor
// Then see setup() function regarding serial & printer begin() calls.

//THE FOLLOWING CODE IMPLEMENTS BUTTONS AND DEBOUNCE VARIABLES

  // constants won't change. They're used here to
  // set pin numbers:
  const int button1 = 11;    // the number of the pushbutton1 pin

  // DEBOUNCE Variables will change:
  int button1State;             // the current reading from the input pin
  int lastButton1State = HIGH;   // the previous reading from the input pin

  // DEBOUNCE the following variables are long's because the time, measured in miliseconds,
  long lastDebounceTime = 0;  // the last time the output pin was toggled
  long debounceDelay = 50;    // the debounce time; increase if the output flickers

//comparison variables to prevent constant rewriting of the display
int curTime = 0;
int lastWriteTime = 0;

bool writeTime = false;
bool writeDate = false;

void setup () {
  pinMode(button1, INPUT_PULLUP);

  //hw serial for DEBUG
  //Serial.begin(9600);

  // NOTE: SOME PRINTERS NEED 9600 BAUD instead of 19200, check test page.
  mySerial.begin(9600);  // Initialize SoftwareSerial
  printer.begin();        // Init printer (same regardless of serial type)

  delay(3000); // wait for console opening

  if (! rtc.begin()) {
    printer.println("Couldn't find RTC");
    while (1);
  }

  if (! rtc.isrunning()) {
    printer.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }

  //printer.upsideDownOn();
  printer.justify('C');
  printer.setSize('L');
  printer.println(F("Ticker-Tape"));
  printer.println(F("Timepiece"));
  printer.justify('L');
  printer.setSize('M');
  printer.feed(1);
  printer.println(F("by Arnold Martin"));
  printer.feed(3);
  /*
  printer.println(F("This device automatically"));
  printer.println(F("prints the time each hour"));
  printer.println(F("and each half hour"));
  printer.println(F("like this:"));
  printer.feed(3);
  displayTime();
  delay(100);
  printer.justify('L');
  printer.setSize('M');
  printer.println(F("A short button press will"));
  printer.println(F("print the current time"));
  printer.println(F("A long button press will"));
  printer.println(F("print todays's date"));
  printer.println(F("like this:"));
  displayDate();
  printer.feed(2);
  */
  printer.justify('C');
  printer.setSize('L');
  printer.println(F("Useless?"));
  printer.feed(1);
  printer.println(F("Wasteful?"));
  printer.feed(1);
  printer.printBitmap(shrug_width, shrug_height, shrug);
  printer.feed(2);
  printer.println(F("ENJOY!"));
  printer.feed(3);
  delay(50);  
  displayTime();
  delay(50);
}

//This runs forever, checks the button for pushes, and checks the time for automated display
//on the hour and ever half hour
void loop () {
  DateTime now = rtc.now();

  if(now.minute() == 0 || now.minute() == 30){
    if(lastWriteTime == now.minute()){
      writeTime = false;
    }
    else if(lastWriteTime != now.minute()){
      writeTime = true;
    }
  }
  
  doubleDebouncyRead();

  if(writeTime == true){
    if(now.minute() != lastWriteTime){
      lastWriteTime = now.minute();
      delay(50);
      displayTime();
      delay(50);
      writeTime = false;
    }
  }
  if(writeDate == true){
    delay(50);
    displayDate();
    delay(50);
    writeDate = false;
  }
  writeTime = false;
  writeDate = false;
}


void displayTime(){
  
  printer.wake();
  DateTime now = rtc.now();
  
  //always print this line
  printer.justify('L');
  printer.setSize('M');
  printer.boldOn();
  printer.println("The Current Time is: ");
  delay(50);
  //If the minute value is greater or equal to 40 we will say the time as "minutes to <the next hour>" 
  //so here we adjust the call from the hours[] array
  //Otherwise we will set the hour text equal to the current hour
  int adjustedHour;
  if (now.minute() >= 40){
    adjustedHour = (now.hour()+1);
    if(adjustedHour == 24){
      adjustedHour = 0;
    }
  }
  else if(now.minute() < 40){
    adjustedHour = now.hour();
  }
  else{
  }

  //create convenient hour and minute strings for writing to the printer
  String hourString = hours[adjustedHour];  
  String minuteString = minutes[now.minute()];

  //If the time is on the hour display this format:
  //There are two basic formats with variations built in dependant on the hour and minute values at runtime

  //if the current time is on the hour do these things
  if ((now.minute() == 0)){
    //Based on the actual value of now.hour() do different things
    printer.justify('C');
    printer.setSize('L');
    printer.boldOn();
    switch (now.hour()) {
      //if it is midnight or noon
      case 0:
      case 12:
        printer.println(hourString);            //print the hour from the the hour[array value]
        break;                                       //and leave
      //if it is on the hour in the morning
      case 1:
      case 2:
      case 3:
      case 4:
      case 5:
      case 6:
      case 7:
      case 8:
      case 9:
      case 10:
      case 11:
        printer.println(hourString+F(" O'clock"));   //print the hour from the the hour[array value]
        printer.justify('R');
        printer.setSize('M');
        printer.println(F("...in the Morning"));             //print the phase of the day
        break;                                          //and leave
      //if it is on the hour in the afternoon
      case 13:
      case 14:
      case 15:
      case 16:
      case 17:
        printer.println(hourString+F(" O'clock"));   //print the hour from the the hour[array value]
        printer.justify('R');
        printer.setSize('M');
        printer.println(F("...in the Afternoon"));           //print the phase of the day
        break;                                          //and leave
      //if it is on the hour in the evening
      case 18:
      case 19:
      case 20:
        printer.println(hourString+F(" O'clock"));   //print the hour from the the hour[array value]
        printer.justify('R');
        printer.setSize('M');
        printer.println(F("...in the Evening"));             //print the phase of the day
        break;                                          //and leave
      //if it is on the hour at night
      case 21:
      case 22:
      case 23:
        printer.println(hourString+F(" O'clock"));   //print the hour from the the hour[array value]
        printer.justify('R');
        printer.setSize('M');
        printer.println(F("...at Night"));                   //print the phase of the day
        break;                                          //and leave
    }
  }

  //if the time is not on the hour display do these things
  else if ((now.minute() != 0)){
    //if the minute value is less than forty display PAST (corresponds to the minutes[] char array)
    if (now.minute() < 40){
      if(now.minute() == 1){
        printer.justify('C');
        printer.setSize('L');
        printer.println(minuteString);
        printer.justify('L');
        printer.setSize('M');
        printer.println(F("minute past..."));
      }
      else if(now.minute() == 15 || now.minute() == 30){
        printer.justify('C');
        printer.setSize('L');
        printer.println(minuteString);
        printer.justify('L');
        printer.setSize('M');
        printer.println(F("past..."));
      }
      else{
        printer.justify('C');
        printer.setSize('L');
        printer.println(minuteString);
        printer.justify('L');
        printer.setSize('M');
        printer.println(F(" minutes past..."));
      }
      if(adjustedHour == 0 || adjustedHour == 12){
        printer.justify('C');
        printer.setSize('L');
        printer.println(hourString);
      }
      else{
        printer.justify('C');
        printer.setSize('L');
        printer.println(hourString+F(" O'clock"));
      }
    }
    //if the minute value is 40 or more display TO the next hour (adjustedhour)
    else if (now.minute() >= 40){
      if(now.minute() == 59){
        printer.justify('C');
        printer.setSize('L');
        printer.println(minuteString);
        printer.justify('L');
        printer.setSize('M');
        printer.println(F("minute to..."));
      }
      else if(now.minute() == 45){
        printer.justify('C');
        printer.setSize('L');
        printer.println(minuteString);
        printer.justify('L');
        printer.setSize('M');
        printer.println(F("to..."));
      }
      else{
        printer.justify('C');
        printer.setSize('L');
        printer.println(minuteString);
        printer.justify('L');
        printer.setSize('M');
        printer.println(F("minutes to..."));
      }
      if(adjustedHour == 0 || adjustedHour == 12){
        printer.justify('C');
        printer.setSize('L');
        printer.println(hourString);
      }
      else{
        printer.justify('C');
        printer.setSize('L');
        printer.println(hourString+F(" O'clock"));
      }
    }
  }
  else{
  }
  //after all is said and done advance the printer paper and put the device to sleep
  printer.feed(3);
  printer.sleep();
}

void displayDate(){
  printer.wake();
  DateTime now = rtc.now();
  String dayNameString = dayNames[now.dayOfTheWeek()];
  String monthString = months[now.month()];
  String dayNumString = dayNums[now.day()];

  printer.justify('L');
  printer.setSize('M');
  printer.println(F("Today's date is:"));
  printer.justify('C');
  printer.setSize('L');
  printer.println(dayNameString);
  printer.println(monthString+F(" ")+dayNumString);
  printer.println(now.year(), DEC);
    
  printer.feed(3);
  printer.sleep();
}

void doubleDebouncyRead(){
    // read the state of the switch into a local variable:
  int reading1 = digitalRead(button1);
/*SERIAL DEBUG*///Serial.println(reading1);
  int reading2;
//  int butNum;
  
  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH),  and you've waited
  // long enough since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (reading1 != lastButton1State) {
    // reset the debouncing timer
    lastDebounceTime = millis();
    }
    unsigned long start = millis();
    unsigned long butTime = 0; 
  if ((millis() - lastDebounceTime) > debounceDelay) {
  // whatever the reading is at, it's been there for longer
  // than the debounce delay, so take it as the actual current state:
    // if the button state has changed:
    if (reading1 != button1State) {
      button1State = reading1;

      //now that the button is debounced and we know noise is not setting it off, let's start a timer
      //to meeasure how long the button is pressed. A long press can have a different action than a short press
      while (button1State == LOW && digitalRead(button1) == LOW && butTime < 2500){
        butTime=millis()-start;
/*SERIAL DEBUG*///        Serial.println(butTime);
      }
      if (button1State == LOW && butTime < 1000) {
        writeTime = true;
      }
      else if (button1State == LOW && butTime > 1000) {
        writeDate = true;
      }
      else{
      }
    }
  }
 lastButton1State = reading1;
}


// OLD WAY OF GETTING TIME
/*
   if (mi != now.minute()) {
    getcurrentHM();
    timeChange = true;
  }

  if (mi != prvMi && timeChange == true) {
    prvMi = mi;
    if (mi == 0 || (mi % 5) == 0) {
      displayTime();
    }
    timeChange == false;
  }

  void getcurrentHM() {
  DateTime now = rtc.now();
  prvMi = mi;
  prvHr = hr;
  mi = now.minute();
  hr = now.hour();
}
*/

