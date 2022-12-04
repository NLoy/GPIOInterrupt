//button.h
#ifndef Button_h
#define Button_h


class Button {
  public:
    const uint8_t PIN;
    Button(uint8_t PIN);
    char buttonName;
    uint32_t numberKeyPresses;
    bool pressed;
    uint32_t lastChange;
    uint32_t curChange;
    uint32_t timePressed;
    bool released;
    bool toggleOn;
    bool singlePressActive;
    char clickType;
    void singleClickChar();
    void doubleClickChar();
    void longPressChar();
};

#endif