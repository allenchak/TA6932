#include "Arduino.h"
#include "TA6932.h"


TA6932::TA6932(uint8_t pin_strobe, uint8_t pin_clock, uint8_t pin_data) {
  PIN_STB = pin_strobe;
  PIN_DIN = pin_data;
  PIN_CLK = pin_clock;

}

void TA6932::begin()
{
    pinMode(PIN_STB, OUTPUT);
    pinMode(PIN_CLK, OUTPUT);
    pinMode(PIN_DIN, OUTPUT);
    
    sendCommand(ACTIVATE);
    reset();
}

void TA6932::sendCommand(uint8_t value)
{
    digitalWrite(PIN_STB, LOW);
    shiftOut(PIN_DIN, PIN_CLK, LSBFIRST, value);
    digitalWrite(PIN_STB, HIGH);
}

void TA6932::reset()
{
    sendCommand(WRITE_INC); // set auto increment mode
    digitalWrite(PIN_STB, LOW);
    shiftOut(PIN_DIN, PIN_CLK, LSBFIRST, 0xc0);   // set starting address to 0
    for (uint8_t i = 0; i < 16; i++){
        shiftOut(PIN_DIN, PIN_CLK, LSBFIRST, 0x00);
        displayCache[i] = 0x00;
    }
    digitalWrite(PIN_STB, HIGH);
    sendCommand(WRITE_INC); // set auto increment mode
    sendCommand(ACTIVATE);
}

void TA6932::displaySS(uint8_t position, uint8_t value) { // call 7-segment
    displayCache[position] = value;
    //updateDisplay();
}

void TA6932::updateDisplay()
{
    sendCommand(WRITE_INC);
    digitalWrite(PIN_STB, LOW);
    shiftOut(PIN_DIN, PIN_CLK, LSBFIRST, 0xC0);   // set starting address to 0
    for(uint8_t p=0;p<16;p++)
    {
        shiftOut(PIN_DIN, PIN_CLK, LSBFIRST, displayCache[p]);
    }
    digitalWrite(PIN_STB, HIGH);
    sendCommand(ACTIVATE); // set auto increment mode
}
