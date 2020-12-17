
#ifndef TA6932_h
#define TA6932_h

#include "Arduino.h"

#define CTRL_BASE   0b10000000 // 0x80
#define WRITE_LOC   0b01000100 // 0x44
#define WRITE_INC   0b01000000 // 0x40
#define DISP_ADDR   0b11000000 // 0xC0

class TA6932
{
  public:
    TA6932(uint8_t pin_strobe, uint8_t pin_clock, uint8_t pin_data);

    void begin();
    void sendCommand(uint8_t value);
    void reset();
    void setBrightness(uint8_t b);
    void displaySS(uint8_t position, uint8_t value);
    void updateDisplay();
    void displayOn();
    void displayOff();
    
    uint8_t displayCache[16];

  private:
    uint8_t PIN_STB;
    uint8_t PIN_DIN;
    uint8_t PIN_CLK;
    uint8_t _brightness = 0;
    bool _displayOn = true;
};

#endif
