// Version
String version = "1.0";
/******************************************************/
/*            Arena Control Unit (A.C.U.)             */
/******************************************************

  Free script designed to control combat robot arena buttons, lights, timer, trapdoor servo and cameras.

  *****
  FEATURES:
  *****
  - Designed for Arduino Nano microprosessor
  - Ready/pause/forfeit button for two competitors
  - Reset button
  - 3min adjustable 7-segment LED strip timer display (Format: 8:88)
  - LED strip control (Lights react to game status.)
  - Piezo buzzer controlled with mosfet
  - Trap door servo control (Opens at last minute)
  - Camera servo control (Starts to record before countdown and stops 5 secs after match end or forfeit.)

  *****
  HARDWARE EXAMPLE:
  *****
  - DFRduino Nano (Arduino Nano Compatible)
  - Gravity: Nano I/O Shield for Arduino Nano
  - 3pcs Gravity: Digital Push Buttons (White, red & blue)
  - 3pcs 10kOhm pulldown resistors for buttons if not using Gravity buttons attached before starting unit (Othervise game will start and do it's own because pins are floating.)
  - 3pcs USB-A Female connectors for buttons
  - 3pcs USB-A Male connectors for buttons
  - 5pcs MR30PW-Male connectors for lights, timer and servos
  - 5pcs MR30PB-Female connectors for lights, timer and servos
  - 3-pole cables for lights, timer and servos
  - 2m 12V WS2811 or WS2815 RGB Led strip for timer for arena lights
  - 12V power supply and connectors
  - 3D printed cases electronics
  
  Timer:
  - 2m 12V 60 led WS2811 or WS2815 RGB Led strip for timer
  - 6pcs Male JST
  - 6pcs Female JST
  - 2m 3-pole 18-22AWG wire
  - 3D printed timer parts
  
  *****
  ARDUINO IDE BOARD MANAGER & LIBRARIES:
  *****
  Tested stable versions:
  - Board:          Arduino AVR Boards v.1.8.6        Install from Arduino IDE
  - Leds:           Adafruit Neopixel v.1.15.1        Install from Arduino IDE
  - Servos:         Servo v.1.2.2                     Install from Arduino IDE (https://docs.arduino.cc/libraries/servo/)

  *****
  BUTTONS & FUNCTIONS:
  *****
  - Game starts with countdown when both competitors have pressed ready buttons.
  - Last minute is alarmed and trapdoor servo opens.
  - Game ending is alarmed.
  - Game can be paused by pressing button and continued when both players have pressed buttons again.
  - Competitor can forfeit round by pressing button long.
  
  Buttons:
  - Player button, Short press  =   Ready/Pause
  - Player button, Long press   =   Forfeit/Tapout
  - Reset button, Long press    =   Resets round.

  *****
  DISCLAIMER:
  *****

  This script has been made respecting the regulations of combat robotics and they should be usable all around the world.
  Script builders and component manufacturers are not responsiple from possible damages.
  Third party script writers hasn't got nothing to do with this project except I'm using their libraries.

  Copyright (C) Ville Ollila (RoboticsIsForNerds, Rebuild Robotics). Licensed under MIT license.
  Examples used are linked in functions.

  Bug reports through https://github.com/RebuildRobotics /program/Issues. Other contacts related this program: support@rebuildrobotics.fi.

*/
// ---------------------- Libraries --------------------
#include <Adafruit_NeoPixel.h>
#include <Servo.h>
Servo trap;
Servo cameras;
/******************************************************/
/* --------------------- PRESETS -------------------- */
/******************************************************/
// ----------------------- Timer -----------------------
#define TIMER_DURATION      180       // Total time in seconds
// ------------------- Trapdoor Servo ------------------
#define TRAP_OPEN           0         // Trapdoor open position   // 0-180 deg
#define TRAP_CLOSE          90        // Trapdoor closed position // 0-180 deg
// -------------------- Camera Servos ------------------
#define CAM_REC             15        // Camera record position   // 0-180 deg
#define CAM_IDLE            0         // Camera stop position     // 0-180 deg
#define CAM_DELAY           5000      // Camera record delay      // ms
// ------------------------ Pins -----------------------
#define PIN_TIMER           11        // LED timer display
#define PIN_TRAP            12        // Trapdoor servo
#define PIN_BUZ             4         // Buzzer
#define PIN_LIGHTS          6         // Lights
#define PIN_CAMS            5         // Camera servos
#define PIN_BUT_RES         7         // Reset button             // Use 10kOhm pulldown resistor (Included in Gravity: Digital Push Buttons.)
#define PIN_BUT_R           9         // Red players button       // Use 10kOhm pulldown resistor (Included in Gravity: Digital Push Buttons.)
#define PIN_BUT_B           8         // Blue players button      // Use 10kOhm pulldown resistor (Included in Gravity: Digital Push Buttons.)
// --------------------- LED Strips --------------------
#define LIGHTS_IC_TOTAL     40        // Amount of IC's controlling leds // Default 2 x 2m 60led/m = 20ICs/m = 40ICs.
Adafruit_NeoPixel lights(LIGHTS_IC_TOTAL, PIN_LIGHTS, NEO_BRG + NEO_KHZ800); // Change NEO_BRG to NEO_RGB etc. if colors are wrong.
// ----------------- LED Timer display -----------------
#define WS2811                        // LED Strip type // WS2811 or WS2815
#define DISPLAYS            2         // Number of timer displays
#if defined(WS2811)
#define TIMER_IC_TOTAL      44        // Amount of IC's controlling leds (WS2811 1xIC = 3leds.) // Includes all displays!
#define TIMER_IC_CHAR       7         // Timers digit IC count
#define TIMER_IC_COLON      1         // Timers colon IC count
#endif
#if defined(WS2815)
#define TIMER_IC_TOTAL      132       // Amount of IC's controlling leds (WS2815 1xIC = 1led.) // Includes all displays!
#define TIMER_IC_CHAR       21        // Timers digit IC count
#define TIMER_IC_COLON      3         // Timers colon IC count
#endif
Adafruit_NeoPixel timer(TIMER_IC_TOTAL, PIN_TIMER, NEO_BRG + NEO_KHZ800); // Change NEO_BRG to NEO_RGB etc. if colors are wrong.
// ------------------ Timer characters -----------------
/* LED Character segment pattern
  2
-1 3
  7 
 6 4
  5
*/
// WS2811 (One IC for 3 leds. 1 ledpixel = 3 leds.) // First byte is just to fill 8bit // Leds are started to read from right // Listed as binary values
#if defined(WS2811)
const uint8_t chars[13] = {
  0b00111111, // 0
  0b00001100, // 1
  0b01110110, // 2
  0b01011110, // 3
  0b01001101, // 4
  0b01011011, // 5
  0b01111011, // 6
  0b00001110, // 7
  0b01111111, // 8
  0b01011111, // 9
  0b00000000, // Empty char
  0b01000000, // -
  0b00000001  // :
};
#endif
// WS2815 (IC in LED, all LEDs can be programmed separately. 1 ledpixel = 1 led.) // First 11 bytes are just to fill 32bit // Leds are started to read from right // Listed as binary values
#if defined(WS2815)
const uint32_t chars[13] = {
  0b00000000000000111111111111111111, // 0
  0b00000000000000000000111111000000, // 1
  0b00000000000111111111000111111000, // 2
  0b00000000000111000111111111111000, // 3
  0b00000000000111000000111111000111, // 4
  0b00000000000111000111111000111111, // 5
  0b00000000000111111111111000111111, // 6
  0b00000000000000000000111111111000, // 7
  0b00000000000111111111111111111111, // 8
  0b00000000000111000111111111111111, // 9
  0b00000000000000000000000000000000, // Empty char
  0b00000000000000000000000000000111, // -
  0b00000000000000000000000000000101  // :
};
#endif
// -------------------- Buzzer tones -------------------
// D major scale = D,E,F#,G,A,H,C#
#define NOTE_CNT            1480      // Countdown    // F#6
#define NOTE_STA            2960      // Start        // F#7
#define NOTE_MIN            1319      // Last minute  // E6
#define NOTE_END            1109      // Match ended  // C#6
#define NOTE_RES            1760      // Reset        // A6
#define NOTE_RED            1976      // Start        // H6
#define NOTE_PAU            1109      // Pause        // C#6
#define NOTE_UNPAU          1568      // Unpause      // G6
#define NOTE_TAP            1175      // Tapout       // D6
/******************************************************/
/* ------------------- END OF PRESETS --------------- */
/******************************************************/
// ---------------------- Values -----------------------
// Cameras
bool recording = false;
// Buttons
bool lastStateReset;
bool lastStateRed;
bool lastStateBlue;
uint8_t debounceDelay = 50;
unsigned long toggleTimeReset;
unsigned long toggleTimeRed;
unsigned long toggleTimeBlue;
unsigned long resetPressedTime;
unsigned long redPressedTime;
unsigned long bluePressedTime;
// Competitors
enum Competitors {Red, Blue};
// States
enum GameStates {Idle, Count, Run, Pause};
GameStates statusGame = Idle;
enum PlayerStates {NotReady, Ready, Paused, Tapout};
PlayerStates statusRed = NotReady;
PlayerStates statusBlue = NotReady;
enum ButtonStates {NotPressed, Pressed, LongPressed};
ButtonStates resetButtonStatus = NotPressed;
// Timer
uint8_t timerCounter = TIMER_DURATION;
unsigned long prevMillis;
bool lastMinuteAlarmed = false;
// ---------------------- Functions --------------------
void updateButtons()
{
  // Reset button //
  bool stateReset = digitalRead(PIN_BUT_RES);
  // Do after button debounce delay
  if ((millis() - toggleTimeReset) >= debounceDelay)
  {
    // Add time for debounce if state has been changed
    if (stateReset != lastStateReset) toggleTimeReset = millis();
    // Pressing button started
    if (stateReset && !lastStateReset)
    {
      lastStateReset = HIGH;
      resetButtonStatus = Pressed;
      resetPressedTime = millis();
    }
    // Calculate time pressed
    long resetPressedDiff = millis() - resetPressedTime;
    // LongPress // Reset
    if (resetButtonStatus != LongPressed && stateReset && lastStateReset && resetPressedDiff >= 2000)
    {
      resetButtonStatus = LongPressed;
      reset();
    }
    // Button released // Shortpress
    if (!stateReset && lastStateReset)
    {
      lastStateReset = LOW;
      resetButtonStatus = NotPressed;
      /*
      if (resetPressedDiff < 2000) // Shortpress
      {
        
      }
      */
    }
  }
  // Red button //
  bool stateRed = digitalRead(PIN_BUT_R);
  // Do after button debounce delay
  if ((millis() - toggleTimeRed) >= debounceDelay)
  {
    // Add time for debounce if state has been changed
    if (stateRed != lastStateRed) toggleTimeRed = millis();
    // Pressing button started
    if (stateRed && !lastStateRed)
    {
      lastStateRed = HIGH;
      redPressedTime = millis();
    }
    // Calculate time pressed
    long redPressedDiff = millis() - redPressedTime;
    // LongPress // Forfeit
    if (statusRed != NotReady && statusRed != Tapout && stateRed && lastStateRed && redPressedDiff >= 2000)
      forfeit(Red);
    // Button released // Shortpress // Ready, Pause, Unpause
    if (!stateRed && lastStateRed)
    {
      lastStateRed = LOW;
      if (redPressedDiff < 2000) // Shortpress
      {
        if (statusRed == Ready && statusGame == Run) // Pause
        {
          togglePause();
          statusRed = Paused;
          Serial.println("[^] Red paused");
        }
        else if (statusRed == NotReady || statusRed == Paused) // Ready, Unpause
        {
          statusRed = Ready;
          tone(PIN_BUZ, NOTE_RED, 250);
          Serial.println("[^] Red ready");
          // Turn all leds red one by one
          for (int i = 0; i < LIGHTS_IC_TOTAL; i++)
          {            
            lights.setPixelColor(i, lights.Color(255, 0, 0));
            lights.show();
            delay(10);
          }
          (statusBlue != Ready) ? showTimer(DISPLAYS, 11, 10, 10, 10, 1, 0, 0) : showTimer(DISPLAYS, 11, 10, 10, 11, 0, 1, 0);
        }
      }
    }
  }
  // Blue button //
  bool stateBlue = digitalRead(PIN_BUT_B);
  // Do after button debounce delay
  if ((millis() - toggleTimeBlue) >= debounceDelay)
  {
    // Add time for debounce if state has been changed
    if (stateBlue != lastStateBlue) toggleTimeBlue = millis();
    // Pressing button started
    if (stateBlue && !lastStateBlue)
    {
      lastStateBlue = HIGH;
      bluePressedTime = millis();
    }
    // Calculate time pressed
    long bluePressedDiff = millis() - bluePressedTime;
    // LongPress // Forfeit
    if (statusBlue != NotReady && statusBlue != Tapout && stateBlue && lastStateBlue && bluePressedDiff >= 2000)
      forfeit(Blue);
    // Button released // Shortpress // Ready, Pause, Unpause
    if (!stateBlue && lastStateBlue)
    {
      lastStateBlue = LOW;
      if (bluePressedDiff < 2000) // Shortpress
      {
        if (statusBlue == Ready && statusGame == Run) // Pause
        {
          togglePause();
          statusBlue = Paused;
          Serial.println("[^] Blue paused");
        }
        else if (statusBlue == NotReady || statusBlue == Paused) // Ready, Unpause
        {
          statusBlue = Ready;
          tone(PIN_BUZ, NOTE_RED, 250);
          Serial.println("[^] Blue ready");
          // Turn all leds blue one by one
          for (int i = 0; i < LIGHTS_IC_TOTAL; i++)
          {            
            lights.setPixelColor(i, lights.Color(0, 0, 255));
            lights.show();
            delay(10);
          }
          (statusRed != Ready) ? showTimer(DISPLAYS, 10, 10, 10, 11, 0, 0, 1) : showTimer(DISPLAYS, 11, 10, 10, 11, 0, 1, 0);
        }
      }
    }
  }
}

void reset()
{
  Serial.println("[^] Reset");
  tone(PIN_BUZ, NOTE_RES, 125);
  delay(250);
  tone(PIN_BUZ, NOTE_RES, 125);
  // Reset values
  lastStateReset = LOW;
  lastStateRed = LOW;
  lastStateBlue = LOW;
  statusGame = Idle;
  statusRed = NotReady;
  statusBlue = NotReady;
  lastMinuteAlarmed = false;
  timerCounter = TIMER_DURATION;
  showTimer(DISPLAYS, 10, 10, 10, 10, 0, 0, 1);
  resetLights();
  trap.write(TRAP_CLOSE);
  if (recording) toggleRec();
}

void forfeit(Competitors player)
{
  tone(PIN_BUZ, NOTE_TAP, 1000);
  if (player == Red) statusRed = Tapout;
  else if (player == Blue) statusBlue = Tapout;
  // Turn on winners color
  for (int i = 0; i < LIGHTS_IC_TOTAL; i++)
  {
    (statusRed == Tapout) ? lights.setPixelColor(i, lights.Color(0, 0, 255)) : lights.setPixelColor(i, lights.Color(255, 0, 0));
  }
  lights.show();
  Serial.println("[^] Tapout: " + player);
  // Stop cameras
  delay(CAM_DELAY);
  if (recording) toggleRec();
}

void togglePause()
{
  if (statusGame == Run)
  {
    Serial.println("[^] Paused");
    statusGame = Pause;
    statusBlue = Paused;
    statusRed = Paused;
    tone(PIN_BUZ, NOTE_PAU, 125);
    delay(250);
    tone(PIN_BUZ, NOTE_PAU, 125);
  }
  else
  {
    delay(3000);
    Serial.println("[^] Unpaused");
    statusGame = Run;
    statusBlue = Ready;
    statusRed = Ready;
    tone(PIN_BUZ, NOTE_UNPAU, 500);
    // Reset lights to white
    resetLights();
  }
}

void startRound()
{
  // Turn all leds green one by one
  for (int i = 0; i < LIGHTS_IC_TOTAL; i++)
  {            
    lights.setPixelColor(i, lights.Color(0, 255, 0));
    lights.show();
    delay(10);
  }
  // Start cameras
  if (!recording) toggleRec();
  // Wait before counting
  delay(CAM_DELAY);
  statusGame = Count;
}
  
void countDown()
{
  // Reset time
  timerCounter = TIMER_DURATION;
  // Count
  for (int i = 4; i > 0; i--)
  {
    showTimer(DISPLAYS, 11, 10, i, 11, 0, 1, 0);
    tone(PIN_BUZ, NOTE_CNT, 250);
    Serial.print("[*] ");
    Serial.println(i);
    delay(1000);
  }
  // Start round
  uint8_t minutes = timerCounter / 60;
  uint8_t seconds = timerCounter % 60; // Modulo operator
  showTimer(DISPLAYS, minutes % 10, 12, (seconds / 10) % 10, seconds % 10, 0, 0, 1);
  tone(PIN_BUZ, NOTE_STA, 1000);  
  Serial.println("[!] START");
  statusGame = Run;
  prevMillis = millis();
  // Reset lights to white
  resetLights();
}

void runRound()
{
  // Count time
  if (timerCounter > 0)
  {
    // Ensure that second has past
    if (millis() - prevMillis >= 1000)
    {
      // Substrack current second from counter
      prevMillis = millis();
      timerCounter -= 1;
      // Count time from counter
      uint8_t minutes = timerCounter / 60;
      uint8_t seconds = timerCounter % 60; // Modulo operator
      // Print time
      (timerCounter <= 60) ? showTimer(DISPLAYS, 10, 12, (seconds / 10) % 10, seconds % 10, 1, 0, 0) : showTimer(DISPLAYS, minutes % 10, 12, (seconds / 10) % 10, seconds % 10, 0, 0, 1);
    }
    // Last minute
    if (timerCounter == 60 && !lastMinuteAlarmed)
    {
      tone(PIN_BUZ, NOTE_MIN, 1000);
      lastMinuteAlarmed = true;
      Serial.println("[!] LAST MINUTE");
      // Open trapdoor servo
      trap.write(TRAP_OPEN);
    }
  }
  // Game over
  else
  {
    tone(PIN_BUZ, NOTE_END, 500);
    delay(750);
    tone(PIN_BUZ, NOTE_END, 1500);
    statusGame = Idle;
    statusRed = NotReady;
    statusBlue = NotReady;
    for (int i = 0; i < LIGHTS_IC_TOTAL; i++)
      lights.setPixelColor(i, lights.Color(255, 0, 0));
    lights.show();
    Serial.println("[!] GAME OVER");
    // Stop cameras
    delay(CAM_DELAY);
    if (recording) toggleRec();
  }
}

void resetLights()
{
  for (int i = 0; i < LIGHTS_IC_TOTAL; i++)
    lights.setPixelColor(i, lights.Color(255, 255, 255));
  lights.show();
}

void setPixel(uint8_t led, uint8_t ledVal, bool R, bool G, bool B)
{
  timer.setPixelColor(led, timer.Color(R * ledVal, G * ledVal, B * ledVal));
}

void showChar(uint8_t value, uint8_t length, uint8_t &startLed, bool R, bool G, bool B)
{
  for (uint8_t i = 0; i < length; i++)
    setPixel(startLed + i, bitRead(chars[value], i) * 255, R, G, B);
  startLed += length; // Move pointer forward
}

void showTimer(uint8_t disp, uint8_t mins, uint8_t col, uint8_t secTens, uint8_t secs, bool R, bool G, bool B) 
{
#if defined(WS2811) || defined(WS2815)
  uint8_t lastLed = 0;
  for (uint8_t d = 0; d < disp; d++)
  {
    // Layout: M:SS  (Minutes, Colon, Seconds Tens, Seconds Ones)
    showChar(mins, TIMER_IC_CHAR, lastLed, R, G, B);
    showChar(col, TIMER_IC_COLON, lastLed, R, G, B);
    showChar(secTens, TIMER_IC_CHAR, lastLed, R, G, B);
    showChar(secs, TIMER_IC_CHAR, lastLed, R, G, B);
  }
  timer.show();
#endif
}

void toggleRec()
{
  cameras.write(CAM_REC);
  delay(1000);
  cameras.write(CAM_IDLE);
  recording = !recording;
}
// ------------------------ Setup ----------------------
void setup()
{
  // Start Serial
  Serial.begin(115200);
  // Add header
  Serial.println("------------ [ A.C.U. V." + version + " ] -----------");
  Serial.println("[*] Setting up devices...");
  // Init buttons
  pinMode(PIN_BUT_RES, INPUT);
  pinMode(PIN_BUT_R, INPUT);
  pinMode(PIN_BUT_B, INPUT);
  // Init Arena lights and turn them on
  lights.begin();
  resetLights();
  // Init trap servo
  trap.attach(PIN_TRAP);
  // Close trap
  trap.write(TRAP_CLOSE);
  // Init camera servos
  cameras.attach(PIN_CAMS);
  // Init LED Timer display
  timer.begin();
  // Show 8:88 in timer as test  
  showTimer(DISPLAYS, 8, 12, 8, 8, 0, 0, 1);
  delay(5000);
  showTimer(DISPLAYS, 10, 10, 10, 10, 0, 0, 1);
  // Add footer
  Serial.println("---------- [ Setup complete ] -----------");
}
// ---------------------- Main loop --------------------
void loop()
{
  // Buttons
  updateButtons();
  // Unpause
  if (statusGame == Pause && statusRed == Ready && statusBlue == Ready)
    togglePause();
  // Start
  if (statusGame == Idle && statusRed == Ready && statusBlue == Ready)
    startRound();
  // Countdown
  while (statusGame == Count)
    countDown();
  // Round
  if (statusGame == Run)
    runRound();
}