#include <Arduino.h>

// ------------------------------------------------------ TIMER INTERRUPT SETUP--
// IRC counter, volatile to exempt from compiler optimizations
// volatile int interruptCounter;

// define timer as hw_timer_t type, set to Null
hw_timer_t *doubleClickTimer = NULL;

// define the time variable for a double click
int8_t doubleClickTime = 350;

// define variable type portMUX_TYPE, to allow for synchronization
// between the main loop and the ISR, when modifying a shared variable
// portMUX_TYPE timerMUX = portMUX_INITIALIZER_UNLOCKED;
 
void IRAM_ATTR onTimer(){
  digitalwrite //ENTER SINGLE BUTTON FUNCTION TO CALLBACK LATER --TODO--
}
// ------------------------------------------------------------------------------

// -------------------------------------------------------------- BUTTON SETUP --
struct Button {
  const uint8_t PIN;
  uint32_t numberKeyPresses;
  bool pressed;
  uint32_t lastChange;
  uint32_t curChange;
  uint32_t timePressed;
  bool released;
  uint32_t lastRelease;
  uint32_t curRelease;
  bool toggleOn;
};

// Button Initialization
Button button1 = {0,  0, false, 0, 0, 0, false, 0, 0, true};
Button button2 = {26, 0, false, 0, 0, 0, false, 0, 0, true};

// Main Interrupt for button presses
void ARDUINO_ISR_ATTR isr(void* arg) {
  Button* s = static_cast<Button*>(arg);
  // Get current time of event
  s->curChange = millis();
  // Debounce & factor for double press on changes for buttons with
  // inline diodes (button B on M5 StickCPlus)
  if ((s->curChange - s->lastChange) <= 50){
    return;
  }  
  // Check if this is a button press or button release
  if (s->pressed == false) {
    s->pressed = true;
  } else if (s->pressed == true) {
    s->timePressed = (s->curChange - s->lastChange);
    s->pressed = false;
    s->released = true;
  }
  s->lastChange = s->curChange;
}
// ------------------------------------------------------------------------------


void setup() {
  Serial.begin(115200);

  // ---------------------------------------------------- TIMER INTERRUPT SETUP--
  // Initialize the timer, with variables: (#/4) of which interrupt 
  // (lower is higher priority?), value of prescaler to get 1 millisecond, 
  // whether to count up /true or down /false
  doubleClickTimer = timerBegin(0, 80, true); 

  // Attach ISR to a handling function, with variables:
  // Name of function, ISR function to attach (&onTimer), whether 
  // to trigger on edge /true, or level /false
  timerAttachInterrupt(doubleClickTimer, &onTimer, true);

  // timername, time of timer, whether to repeat
  timerAlarmWrite(doubleClickTimer, doubleClickTime, false);
  timerAlarmEnable(doubleClickTimer);

  timerEnd(doubleClickTimer);
  // ----------------------------------------------------------------------------

  // ------------------------------------------------------------ BUTTON SETUP --
  pinMode(button1.PIN, INPUT_PULLUP);
  attachInterruptArg(button1.PIN, isr, &button1, CHANGE);
  pinMode(button2.PIN, INPUT_PULLUP);
  attachInterruptArg(button2.PIN, isr, &button2, CHANGE);
}

void loop() {

  // ------------------------------------------------------ BUTTON INPUT CHECK --
  // Button 1 ------------------------------ //
  if (button1.released) {
    button1.released = false; // Reset released value regardless of press type
    if (button1.timePressed >= 3000) { // Button 1 Long press length definition
      // If button 1 is long pressed
      Serial.println("Button 1 was long pressed for longer than 3 seconds, and the interrupt function toggled on/off.");
      button1.toggleOn = !button1.toggleOn; // Does not detach the interrupt, just toggles this loop to register it or not
      return;
      // If button 1 interrupt is disabled and not long pressed
    } else if (button1.timePressed <= 3000 && button1.toggleOn == false){
      return;
      // If button 1 interrupt is enabled and not long pressed
    } else {
      button1.curRelease = button1.lastChange;
      if((button1.curRelease - button1.lastRelease) <= doubleClickTime){ // Button 1 double click timing definition
        // If button 1 is double pressed (long press doesn't count as a second press)
        Serial.printf("Button 1 was double pressed!!! There were only %u milliseconds between the last two presses!\n", (button1.curRelease - button1.lastRelease));
      } else {
        // If button 1 is single pressed
        
        button1.numberKeyPresses++;
        Serial.printf("Button 1 has been pressed %u times. The last press was %u milliseconds long.\n", button1.numberKeyPresses, button1.timePressed);
      }
      button1.lastRelease = button1.lastChange;
    }  
  }

  // Button 2 ----------------------------- //
  if (button2.released) {  // If button 2 is single pressed (No double press or long press defined)
      button2SingleClick(); 
  }
}

void button2SingleClick() {
  button2.curRelease = button2.lastChange;
  button2.numberKeyPresses++;
  Serial.printf("Button 2 has been pressed %u times. The last press was %u milliseconds long.\n", button2.numberKeyPresses, button2.timePressed);
  button2.released = false;
  button2.curRelease = button2.lastChange;
}

/*
NEED TO SET UP A TIMER INTERRUPT TO SAME LENGTH AS DOUBLE CLICK TIMING, TO RUN
ONLY IF NO DOUBLE CLICK - ENABLE IT HERE, AND DISABLE IT IN THE DOUBLE CLICK FUNCTION,
AND MOVE THE SINGLE CLICK ACTIONS INTO THE TIMER INTERRUPT. ALSO, HAVE INDIVIDUAL 
FUNCTIONS CALLED OUT FOR EACH BUTTON#SINGLE,DOUBLE,LONG CLICKS, TO MORE EASILY ENTER 
WHAT EACH ONE DOES, RATHER THAN HAVING TO DIG THROUGH ALL THESE IF-THEN STATEMENTS
*/

