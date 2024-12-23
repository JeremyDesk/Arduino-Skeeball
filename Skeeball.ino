#include <EEPROM.h>             // install EEPROM library from library manager
const int startPin = 2;         // Which pin on the Arduino is connected to the Start pin?
const int hundredPin = 3;       // Which pin on the Arduino is connected to the 100 pin?
const int fiftyPin = 4;         // Which pin on the Arduino is connected to the 50 pin?
const int fortyPin = 5;         // Which pin on the Arduino is connected to the 40 pin?
const int thirtyPin = 6;        // Which pin on the Arduino is connected to the 30 pin?
const int twentyPin = 7;        // Which pin on the Arduino is connected to the 20 pin?
const int tenPin = 8;           // Which pin on the Arduino is connected to the 10 pin?
const int relay = 12;           // Which pin on the Arduino is connected to the Relay? (for the start button LED)
const int relay2 = 13;          // Which pin on the Arduino is connected to the Other Relay? (for the 100 Light)
const byte ledPin = 11;         // Which pin on the Arduino is connected to the NeoPixels?
const byte numDigits = 4;       // How many digits (numbers) are available on your display
const byte pixelPerDigit = 14;  // all Pixel, including decimal point pixels if available at each digit
bool start;                     // Set start at True/False
uint16_t score;                 // Set the score as an unsigned 16 bit integer
uint16_t hi;                    // Set high score as an unsigned 16 bit integer
int run;                        // Set the run variable (explained later) as an integer
unsigned long startTime;        // Set this time-keeping variable as an unsigned long (needs to be an unsigned long for dealing with time)
unsigned long endTime;          // Set this time-keeping variable as an unsigned long
unsigned long notIdle;          // Set this time-keeping variable as an unsigned long

typedef uint16_t segsize_t;  // fit variable size to your needed pixels. uint16_t --> max 16 Pixel per digit
const segsize_t segment[8]{
  0b0000000000000011,  // SEG_A
  0b0000000000001100,  // SEG_B
  0b0000000000110000,  // SEG_C
  0b0000000011000000,  // SEG_D
  0b0000001100000000,  // SEG_E
  0b0000110000000000,  // SEG_F
  0b0011000000000000,  // SEG_G
  0b0000000000000000   // SEG_DP if you don't have a decimal point, just leave it zero
};
const uint16_t ledCount(pixelPerDigit* numDigits);  // keeps track of used pixels

#include <Adafruit_NeoPixel.h>                                    // install Adafruit library from library manager
Adafruit_NeoPixel strip(ledCount, ledPin, NEO_GRB + NEO_KHZ800);  // create neopixel object like you commonly used with Adafruit

#include <Noiasca_NeopixelDisplay.h>                                        // download library from: http://werner.rothschopf.net/202005_arduino_neopixel_display_en.htm
Noiasca_NeopixelDisplay display(strip, segment, numDigits, pixelPerDigit);  // create display object, handover the name of your strip as first parameter!

void setup() {
  strip.begin();                      // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();                       // Turn OFF all pixels ASAP
  strip.setBrightness(50);            // Set BRIGHTNESS to about 1/5 (max = 255)
  display.setColorFont(0xff0000);     // Sets the display color to red
  pinMode(relay, OUTPUT);             // Blah blah defining inputs (idk why this one needs to be pulled up but it does)
  pinMode(hundredPin, INPUT_PULLUP);  // Blah blah defining inputs
  pinMode(fiftyPin, INPUT);           // Blah blah defining inputs
  pinMode(fortyPin, INPUT);           // Blah blah defining inputs
  pinMode(thirtyPin, INPUT);          // Blah blah defining inputs
  pinMode(twentyPin, INPUT);          // Blah blah defining inputs
  pinMode(tenPin, INPUT);             // Blah blah defining inputs
  pinMode(startPin, INPUT_PULLUP);    // Blah blah defining inputs (I do know why this one needs to be pulled up)
  digitalWrite(relay, HIGH);          // Set the realay to turn on
  digitalWrite(relay2, LOW);          // Set the other relay to turn off
  notIdle = 0;                        // Sets the initial not-idle time
  if (isnan(EEPROM.get(0, hi))) {     // Check if there is a high score stored in EEPROM (permanent memory)
    uint16_t fix = 0;                 // make a quick unsigned 16 bit integer 0
    EEPROM.put(0, fix);               // put that 0 in as the high score
  }
  hi = EEPROM.get(0, hi);  // Put the highscore on a variable so you dont have to keep reading the EEPROM
}

void loop() {
  start = digitalRead(startPin);          // Check if start button is pressed
  if (millis() - notIdle >= 20000UL) {    // Idle after 20 seconds
    if (millis() - notIdle <= 23000UL) {  // Make the highscore animation
      display.clear();
      display.writeLowLevel(0, 0b11111100111100);  // Make first segment H
      display.writeLowLevel(1, 0b00111100000000);  // Make second segment I
      display.show();
    } else {
      display.clear();
      display.print(hi);  // Show the high score
      notIdle += 6000UL;
    }
  }
  if (start == 0) {                            // Start Button Pressed
    score = 0;                                 // Reset score
    digitalWrite(relay, LOW);                  // Turn off LED in start button
    display.clear();                           // Clear the display
    display.print(0);                          // Print the initial score of 0
    startTime = millis();                      // Get the current time
    endTime = millis();                        // Get the current time (again)
    while (endTime - startTime <= 20000UL) {  // Run the game for 2 minutes (120000UL = 120 seconds)
      run = 0;                                 // The run system allows simultaneous holes to score while preventing double scoring
      run += int(!digitalRead(hundredPin)) * 100;
      run += int(!digitalRead(fiftyPin)) * 50;
      run += int(!digitalRead(fortyPin)) * 40;
      run += int(!digitalRead(thirtyPin)) * 30;
      run += int(!digitalRead(twentyPin)) * 20;
      run += int(!digitalRead(tenPin)) * 10;

      if (run != 0) {          // If the player scored then add it to the score
        score += run;          // addition
        if (run ==100) {
          digitalWrite(relay2, HIGH);      //100 Light (thanks blake)
        }
        display.clear();       // Clear the display
        display.print(score);  // Print the new score
        delay(800);            // 0.8 second delay prevents double scoring
        digitalWrite(relay2, LOW);
      }
      endTime = millis();  // take the current time to see if the game is over
    }
    /*
    if (score > EEPROM.get(0, hi)) {               // Check if the high score was beaten
      EEPROM.put(0, score);                        // Put the new high score in the EEPROM (permanent memory)
      hi = EEPROM.get(0, hi);                      // Update the high score variable not in the EEPROM
      display.clear();                             // Clear the display
      display.setColorFont(0x00ff00);              // Make the display Green
      display.writeLowLevel(0, 0b11111100111100);  // Make first segment H
      display.writeLowLevel(1, 0b00111100000000);  // Make second segment I
      display.writeLowLevel(2, 0b00000000101100);  // Make third segment H
      display.writeLowLevel(3, 0b00000000101100);  // Make fourth segment I
      display.show();                              // Display HI HI
      delay(2000);                                 // Wait 2 seconds
      display.setColorFont(0xff0000);              // Set the color back to red
      display.clear();                             // Clear the display
      display.print(score);                        // Print the score again
    }
    */
    notIdle = millis();         // Reset the idle-time
    digitalWrite(relay, HIGH);  // Turn on the LED in the start button
  }
}
