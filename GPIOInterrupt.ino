#include <Arduino.h>


// -------------------------------------------------------------- BUTTON SETUP --
struct Button {
  uint8_t PIN;
  char buttonName;
  uint8_t NUM;
  uint32_t numberKeyPresses;
  bool pressed;
  uint32_t lastTriggerTime;
  uint32_t curTriggerTime;
  uint32_t timePressed;
  bool released;
  bool toggleOn;
  bool singlePressActive;
  char singleClickValue;
  char doubleClickValue;
  char longPressValue;
  char buttonReturn;
};

/*  CHAR CODE FOR BUTTON CHAR VALUES
n = next song
l = last song
p = toggle pause/play
v = activate google assistant
x = turn off screen
u = volume up
d = volume down
z = base value, Tasker to ignore if set to this
*/

// define the time variables for debounce, double click, and a long press
const uint8_t debounceTime = 50;
const uint16_t longPressTime = 3000;
const uint16_t doubleClickTime = 350;

// Button Initialization
Button buttonA = { 0, 'A', 0, 0, false, 0, 0, 0, false, true, false, 'l', 'd', 'x', 'z'};
Button buttonB = {26, 'B', 1, 0, false, 0, 0, 0, false, true, false, 'n', 'v', 'x', 'z'};
const uint8_t numButtons = 2;                                 // Match to total number of buttons, max of 4 timer interrupts for esp32
Button buttonsUsed[numButtons] = { buttonA, buttonB };  // Include all button variables
// ------------------------------------------------------------------------------



// ------------------------------------------------------ TIMER INTERRUPT SETUP--
// IRC counter, volatile to exempt from compiler optimizations
// volatile int interruptCounter;

// define timer as hw_timer_t type, set to NULL
hw_timer_t * doubleClickTimer[numButtons] = {};
// ------------------------------------------------------------------------------



// ------------------------------------------------- BUTTON INTERRUPT FUNCTION --
void ARDUINO_ISR_ATTR isr(void *arg) {
  Button *s = static_cast<Button*>(arg);
  // s is a pointer to the static_cast pointer of the arg

  // Get current time of event
  s->curTriggerTime = millis();

  // Debounce & factor for double press on changes for buttons with
  // inline diodes (button B on M5 StickCPlus)
  if ((s->curTriggerTime - s->lastTriggerTime) <= debounceTime) {
    return;
  }

  // Check if this is a button press or button release
  if (s->pressed == false) {  // ON BUTTON PRESS
    s->pressed = true;

  } else {  // ON BUTTON RELEASE
    s->pressed = false;
    s->timePressed = (s->curTriggerTime - s->lastTriggerTime);
    s->lastTriggerTime = s->curTriggerTime;

    // Check if single click or second of a double click
    if (s->timePressed > longPressTime) {  // ON LONG PRESS
      s->buttonReturn = s->longPressValue;
      s->released = true;

    } else if (s->singlePressActive != true) {  // ON FIRST CLICK
      s->singlePressActive = true;
      //////////////////////////////////// TODO start doubleclick timer##############
      timerStart(doubleClickTimer[s->NUM]);
    } else {  // ON SECOND CLICK
      //////////////////////////////////// TODO end doubleclick timer##############
      timerStop(doubleClickTimer[s->NUM]);
      timerWrite(doubleClickTimer[s->NUM], 0);
      s->singlePressActive = false;
      s->buttonReturn = s->doubleClickValue;
      s->released = true;
    }
  }
}
// ------------------------------------------------------------------------------



// -------------------------------------------------- TIMER INTERRUPT FUNCTION --
Void IRAM_ATTR onTimer() {
  Button timerButton = timerButtonTemp;
  timerButton.buttonReturn = timerButton.singleClickValue;
}


  //////////////////TO DO - SET UP A WAY TO KEEP TRACK OF WHICH BUTTONS ARE
// ACTIVE ON TIMERS, AND HAVE THE onTimer CHECK ALL OF THEM FOR WHICH IS THE 
// CURRENT ONE TO SET IT OFF WHENEVER IT'S CALLED. NO ARG PASSING ALLOWED TO 
// onTimer function apparently, so this will be a workaround





// ------------------------------------------------------------------------------



// ---------------------------------------------------------------- VOID SETUP --
void setup() {
  Serial.begin(115200);

  for (uint8_t i = 0; i < numButtons; i++) {

    // --------------------------------- BUTTON INTERRUPT INITIALIZATION --------
    Button button = buttonsUsed[i];
    pinMode(button.PIN, INPUT_PULLUP);
    attachInterruptArg(button.PIN, isr, &button, CHANGE);
    // --------------------------------------------------------------------------

    // ---------------------------------- TIMER INTERRUPT INITIALIZATION --------
    // Check to see if i is greater than available # of timer interrupts
    if (i >= 4) {
      continue;
    }
    // Initialize the timer, with variables: (# = 0,1,2,3) of which interrupt
    // (lower is higher priority?), value of prescaler to get 1 millisecond,
    // whether to count up /true or down /false
    doubleClickTimer[i] = timerBegin(i, 80, true);

    // Attach ISR to a handling function, with variables:
    // Name of function, ISR function to attach (&onTimer), whether
    // to trigger on edge (true), or level (false)
    Button timerButtonTemp = buttonsUsed[i];
    timerAttachInterrupt(doubleClickTimer[i], void (*onTimer)(), true);

    // timername, time of timer, whether to repeat
    timerAlarmWrite(doubleClickTimer[i], doubleClickTime, false);

    // Enables the timerAlarm of: timername - does NOT start the timer!
    timerAlarmEnable(doubleClickTimer[i]);
    // --------------------------------------------------------------------------
  }
}

void loop() {

  // ------------------------------------------------------ BUTTON INPUT CHECK --
  for (uint8_t i = 0; i < numButtons; i++) {
    Button button = buttonsUsed[i];
    if (button.released) {
      Serial.printf("Button %u: %u\n", button.buttonName, button.buttonReturn);
      button.buttonReturn = 'z';  // base value, tasker to ignore if set to this
    
    }
  }
}

  /*
  if (button1.released) {
    button1.released = false;        // Reset released value regardless of press type
    if (button1.clickType == 's') {  // Button 1
      // If button 1 is long pressed
      Serial.println('Button 1 was long pressed for longer than 3 seconds, and the interrupt function toggled on/off.');
      button1.toggleOn = !button1.toggleOn;  // Does not detach the interrupt, just toggles this loop to register it or not
      return;
      // If button 1 interrupt is disabled and not long pressed
    } else if (button1.timePressed <= longPressTime && button1.toggleOn == false) {
      return;
      // If button 1 interrupt is enabled and not long pressed
    } else {
      button1.curRelease = button1.lastTriggerTime;
      if ((button1.curRelease - button1.lastRelease) <= doubleClickTime) {  // Button 1 double click timing definition
        // If button 1 is double pressed (long press doesn't count as a second press)
        Serial.printf('Button 1 was double pressed!!! There were only %u milliseconds between the last two presses!\n', (button1.curRelease - button1.lastRelease));
      } else {
        // If button 1 is single pressed

        button1.numberKeyPresses++;
        Serial.printf('Button 1 has been pressed %u times. The last press was %u milliseconds long.\n', button1.numberKeyPresses, button1.timePressed);
      }
      button1.lastRelease = button1.lastTriggerTime;
    }
  }

  // Button 2 ----------------------------- //
  if (button2.released) {  // If button 2 is single pressed (No double press or long press defined)
    button2SingleClick();
  }
*/


/*
void button2SingleClick() {
  button2.curRelease = button2.lastTriggerTime;
  button2.numberKeyPresses++;
  Serial.printf('Button 2 has been pressed %u times. The last press was %u milliseconds long.\n', button2.numberKeyPresses, button2.timePressed);
  button2.released = false;
  button2.curRelease = button2.lastTriggerTime;
}
*/
/*
NEED TO SET UP A TIMER INTERRUPT TO SAME LENGTH AS DOUBLE CLICK TIMING, TO RUN
ONLY IF NO DOUBLE CLICK - ENABLE IT HERE, AND DISABLE IT IN THE DOUBLE CLICK FUNCTION,
AND MOVE THE SINGLE CLICK ACTIONS INTO THE TIMER INTERRUPT. ALSO, HAVE INDIVIDUAL 
FUNCTIONS CALLED OUT FOR EACH BUTTON#SINGLE,DOUBLE,LONG CLICKS, TO MORE EASILY ENTER 
WHAT EACH ONE DOES, RATHER THAN HAVING TO DIG THROUGH ALL THESE IF-THEN STATEMENTS
*/
