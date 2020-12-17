
#include <TA6932.h>

// Power should go to 3.3-5v and GND
// Power MUST be 5v in case using Blue color LED

#define PIN_TA6932_STB   8
#define PIN_TA6932_CLK   9
#define PIN_TA6932_DIN  10

TA6932 tm(PIN_TA6932_STB, PIN_TA6932_CLK, PIN_TA6932_DIN);

uint8_t p, v;
uint8_t value = 1;

void setup() {
    // put your setup code here, to run once:
    tm.begin();
    Serial.begin(115200);
    Serial.println("Started");

    tm.displayCache[6]  = 0x7F; //8
    tm.displayCache[7]  = 0x07; //7
    tm.displayCache[8]  = 0x01; //:
    tm.displayCache[9]  = 0x7D; //6
    tm.displayCache[10] = 0x6D; //5
    tm.updateDisplay();
    delay(1000);
    
    for( uint8_t b=0; b<8; b++){
      tm.setBrightness(b);
      delay(1000);
    }

    tm.displayCache[8]  = 0x00; //:
    tm.updateDisplay();
    delay(1000);
    tm.displayCache[8]  = 0x01; //:
    tm.updateDisplay();
    delay(1000);

    tm.displayOff();
    delay(3000);

    tm.displayOn();
    delay(3000);

    tm.displayOff();
    delay(3000);

    tm.displayCache[6] = 0;
    tm.displayCache[7] = 0;
    tm.displayCache[8] = 0;
    tm.displayCache[9] = 0;
    tm.displayCache[10] = 0;

    tm.displayOn();
    delay(3000);

    //tm.updateDisplay();
}

void loop() {
    // put your main code here, to run repeatedly:
    for(p=6;p<10;p++){
        value = 1;
        for(v=0;v<8;v++){
            Serial.print("Pos: ");
            Serial.print(p, DEC);
            Serial.print(", v: ");
            Serial.print(v, DEC);
            Serial.print(", Value: ");
            Serial.println(value, DEC);
            
            tm.displaySS(p, value);
            tm.updateDisplay();
            delay(500);
            
            value = value << 1;
        }
    }
}
