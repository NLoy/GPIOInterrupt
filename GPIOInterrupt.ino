#include <Arduino.h>
#include <string>


// -------------------------------------------------------------- BUTTON SETUP --
struct Button {
  uint8_t PIN;
  String buttonName;
  uint8_t NUM;
  uint32_t numberKeyPresses;
  bool pressed;
  uint32_t lastTriggerTime;
  uint32_t curTriggerTime;
  uint32_t timePressed;
  bool released;
  bool toggleOn;
  bool singlePressActive;
  String singleClickValue;
  String doubleClickValue;
  String longPressValue;
  String buttonReturn;
};


// define the time variables for debounce, double click, and a long press
const uint8_t debounceTime = 50;
const uint16_t longPressTime = 3000;
const uint16_t doubleClickTime = 350;

// Button Initialization
Button buttonA = { 0, "Front", 0, 0, false, 0, 0, 0, false, true, false, "toggle_play_pause", "activate_google_assistant", "turn_off_screen", "none"};
Button buttonB = {26,  "Side", 1, 0, false, 0, 0, 0, false, true, false, "next_song",         "last_song", "navigate_home", "none"};
const uint8_t numButtons = 2;                                 // Match to total number of buttons, max of 4 timer interrupts for esp32
Button buttonsUsed[numButtons] = { buttonA, buttonB };  // Include all button variables
// ------------------------------------------------------------------------------



// ------------------------------------------------------ TIMER INTERRUPT SETUP--
// IRC counter, volatile to exempt from compiler optimizations
// volatile int interruptCounter;

// define timers as hw_timer_t type, set to NULL
hw_timer_t* doubleClickTimer0 = NULL;
hw_timer_t* doubleClickTimer1 = NULL;
hw_timer_t* doubleClickTimer2 = NULL;
hw_timer_t* doubleClickTimer3 = NULL;

// Create array of hw_timer_t pointer types
hw_timer_t* doubleClickTimer[numButtons] = {NULL};
// ------------------------------------------------------------------------------



// ------------------------------------------------- BUTTON INTERRUPT FUNCTION --
// ISR function that returns nothing, takes as argument a /void pointer/ to the memory address of a function
// "A void pointer is a pointer that has no associated data type with it. A void pointer can hold address of any type 
// and can be typecasted to any type. Void pointers cannot be dereferenced with *arg to get value of arg"
void ARDUINO_ISR_ATTR isr(void* arg) {
  //s is a pointer (Button type) to the given (void pointer arg) static_cast as a Button type
  Button *s = static_cast<Button*>(arg); 
    // static_cast<new-type>(expression) :::: Returns a value of type "new-type"

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
 // Interrupt timer function for button number 0
void IRAM_ATTR onTimer0() {
  Button tempButton = buttonsUsed[0];
  tempButton.buttonReturn = tempButton.singleClickValue;
  tempButton.singlePressActive = false;
  tempButton.released = true;
  Serial.println("onTimer0 debug");
}

 // Interrupt timer function for button number 1
void IRAM_ATTR onTimer1() {
  Button tempButton = buttonsUsed[1];
  tempButton.buttonReturn = tempButton.singleClickValue;
  tempButton.singlePressActive = false;
  tempButton.released = true;
  Serial.println("onTimer1 debug");
}

 // Interrupt timer function for button number 2
void IRAM_ATTR onTimer2() {
  Button tempButton = buttonsUsed[2];
  tempButton.buttonReturn = tempButton.singleClickValue;
  tempButton.singlePressActive = false;
  tempButton.released = true;
  Serial.println("onTimer2 debug");
}

 // Interrupt timer function for button number 3
void IRAM_ATTR onTimer3() { 
  Button tempButton = buttonsUsed[3];
  tempButton.buttonReturn = tempButton.singleClickValue;
  tempButton.singlePressActive = false;
  tempButton.released = true;
  Serial.println("onTimer3 debug");
}

 // Create an array of the interrupt timer functions
void (*onTimers[numButtons])() = {NULL};


// ------------------------------------------------------------------------------



// ---------------------------------------------------------------- VOID SETUP --
void setup() {
  Serial.begin(115200);

  for (uint8_t i = 0; i < numButtons; i++) {

    // --------------------------------- BUTTON INTERRUPT INITIALIZATION --------
    Button* button = &buttonsUsed[i];
    pinMode(button->PIN, INPUT_PULLUP);
    attachInterruptArg(button->PIN, isr, &button, CHANGE);
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

    // Add the individual onTimer# pointers to the pointer array of onTimers
    switch (i) {
      case 0:
        onTimers[i] = &onTimer0;
        break;
      case 1:
        onTimers[i] = &onTimer1;
        break;
      case 2:
        onTimers[i] = &onTimer2;
        break;
      case 3:
        onTimers[i] = &onTimer3;
        break;
    }

    // Attach ISR to a handling function, with variables:
    // Name of timer, ISR function pointer to attach (&onTimer), whether
    // to trigger on edge (true), or level (false)
    timerAttachInterrupt(doubleClickTimer[i], onTimers[i], true);
  //      attachInterruptArg(button.PIN, isr, &button, CHANGE);
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
    Button* button = &buttonsUsed[i];
    Serial.printf("Button %s . released = %d\n", button->buttonName, button->released);

    if (button->released) {
      Serial.printf("Button %s: %u\n", button->buttonName, button->buttonReturn);
      button->buttonReturn = "blank";  // base value, tasker to ignore if set to this
    
    }
  }
}