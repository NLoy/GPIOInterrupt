#include <Arduino.h>

// ------------------------------------------------------ TIMER INTERRUPT SETUP--
// IRC counter, volatile to exempt from compiler optimizations
// volatile int interruptCounter;

// define timer as hw_timer_t type, set to Null
hw_timer_t* doubleClickTimer = NULL;

// define variable type portMUX_TYPE, to allow for synchronization
// between the main loop and the ISR, when modifying a shared variable
// portMUX_TYPE timerMUX = portMUX_INITIALIZER_UNLOCKED;
void IRAM_ATTR onTimer(void* arg2) {
  buttonTemp.singlePressActive = false;
  buttonTemp.clickType = "s";
  buttonTemp.released = true;
}
// ------------------------------------------------------------------------------

void ARDUINO_ISR_ATTR isr(void* arg) {
  Button* s = static_cast<Button*>(arg);



// -------------------------------------------------------------- BUTTON SETUP --
// define the time variables for debounce, double click, and a long press
const uint8_t  debounceTime = 50;
const uint16_t longPressTime 3000;
const uint16_t doubleClickTime = 350;

struct Button {
  const uint8_t PIN;
  char buttonName;
  uint32_t numberKeyPresses;
  bool pressed;
  uint32_t lastTriggerTime;
  uint32_t curTriggerTime;
  uint32_t timePressed;
  bool released;
  bool toggleOn;
  bool singlePressActive;
  char clickType;
  char singleClickValue;
  char doubleClickValue;
  char longPressValue;
  char buttonReturn = "z";
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

// Button Initialization
Button button1 = { 0, "1" 0, false, 0, 0, 0, false, true, false, "l", "d", "x", "z"};
Button button2 = { 26, "2", 0, false, 0, 0, 0, false, true, false, "n", "v", "x", "z"};
uint8_t numButtons = 2;  // Match to total number of buttons
Button buttonsUsed[numButtons] = {button1, button2};


// Main Interrupt for button presses
void ARDUINO_ISR_ATTR isr(void* arg) {
  Button* s = static_cast<Button*>(arg);

  // Get current time of event
  s->curTriggerTime = millis();

  // Debounce & factor for double press on changes for buttons with
  // inline diodes (button B on M5 StickCPlus)
  if ((s->curTriggerTime - s->lastTriggerTime) <= debounceTime) {
    return;
  }

  // Check if this is a button press or button release
  if (s->pressed == false) {             // ON BUTTON PRESS
    s->pressed = true;

  } else {                               // ON BUTTON RELEASE
    s->pressed = false;
    s->timePressed = (s->curTriggerTime - s->lastTriggerTime);
    s->lastTriggerTime = s->curTriggerTime;

    // Check if single click or second of a double click
    if (s->timePressed > longPressTime) {       // ON LONG PRESS
      s->clickType = "l";
      s->released = true;

    } else if (s->singlePressActive != true) {  // ON FIRST CLICK
      s->singlePressActive = true;
      TODO start doubleclick timer##############

    } else {                                    // ON SECOND CLICK
      TODO end doubleclick timer##############
      s->singlePressActive = false;
      s->clickType = "d";
      s->released = true;
    }
  }
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
  for (uint8_t i = 0; i < numButtons; i++) {
    if (buttonsUsed[i].released) {
      switch (buttonsUsed[i].clickType) {
        case "s":  // SINGLE CLICK FUNCTION CALL
          buttonReturn = buttonsUsed[i].singleClickValue());
          break;
        case "d":  // DOUBLE CLICK FUNCTION CALL
          buttonReturn = buttonsUsed[i].doubleClickValue());
          break;
        case "l":  // LONG PRESS FUNCTION CALL
          buttonReturn = buttonsUsed[i].longPressValue());
          break;
      }
    Serial.println(buttonUsed[i].buttonName + ": " + buttonReturn);
    buttonReturn = "z"; // base value, tasker to ignore if set to this
    }
    
  }

  Serial.println(String()


/*
  if (button1.released) {
    button1.released = false;        // Reset released value regardless of press type
    if (button1.clickType == "s") {  // Button 1
      // If button 1 is long pressed
      Serial.println("Button 1 was long pressed for longer than 3 seconds, and the interrupt function toggled on/off.");
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
        Serial.printf("Button 1 was double pressed!!! There were only %u milliseconds between the last two presses!\n", (button1.curRelease - button1.lastRelease));
      } else {
        // If button 1 is single pressed

        button1.numberKeyPresses++;
        Serial.printf("Button 1 has been pressed %u times. The last press was %u milliseconds long.\n", button1.numberKeyPresses, button1.timePressed);
      }
      button1.lastRelease = button1.lastTriggerTime;
    }
  }

  // Button 2 ----------------------------- //
  if (button2.released) {  // If button 2 is single pressed (No double press or long press defined)
    button2SingleClick();
  }
*/

}

/*
void button2SingleClick() {
  button2.curRelease = button2.lastTriggerTime;
  button2.numberKeyPresses++;
  Serial.printf("Button 2 has been pressed %u times. The last press was %u milliseconds long.\n", button2.numberKeyPresses, button2.timePressed);
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
