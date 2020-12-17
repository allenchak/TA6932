
// TA6932 custom library for STM8s, fully following the datasheet
#define TA6932_FUNC_SET_BRI    false //enable TA6932 setBrightness
#define TA6932_FUNC_DISPLAY    false //enable TA6932 display on/off

// DS3231 (Basic features only. Without Date, Alarm, etc)
// ✔ Get Hour 
// ✔ Get Minute
// ✔ Get Second
// ✔ Set Hour
// ✔ Set Minute
// ✔ Set Second (Enable manually)
// ✔ Get Temperture Celcius (Enable manually)
// ✔ Get Day of week (Enable manually)
// ✔ Set Day of week (Enable manually)
#define DS3231_FUNC_SET_SEC    true  //enable DS3231 Set Second (+85 Bytes)       [untest]
#define DS3231_FUNC_GET_TEMP   false //enable DS3231 Get temperture (+43 Bytes)   [ PASS ]
#define DS3231_FUNC_SET_DOW    false //enable DS3231 Set Day-of-week (+17 bytes)  [untest]
#define DS3231_FUNC_GET_DOW    false //enable DS3231 Get Day-of-week (+12 bytes)  [ PASS ]



#include <I2C.h> //For DS3231


//Pin mapping - https://tenbaht.github.io/sduino/hardware/stm8blue/#pin-number-mappings
#define PIN_TA6932_STB  PD2 // 3 Wires, Grey
#define PIN_TA6932_CLK  PD3 // 3 Wires, White
#define PIN_TA6932_DIN  PC7 // 3 Wires, Blue
#define PIN_DS3231_SCL  PD4 // Hardware I²C SCL
#define PIN_DS3231_SDA  PB5 // Hardware I²C SDA
#define PIN_BTN_A       PC4
#define PIN_BTN_OK      PC5
#define PIN_BTN_B       PC6


#define CFG_BTN_CD_LONG   1500 // in millisSecond
#define CFG_BTN_CD         500 // in millisSecond


//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

#if TA6932_FUNC_SET_BRI == true
#define TA6932_CMD_CTRL_BASE     0b10000000 // 0x80
#else
#define TA6932_CMD_CTRL_BASE     0b10000111 // 0x80
#endif
#define TA6932_CMD_WRITE_INC     0b01000000 // 0x40
#define TA6932_ADDR_FIRST_DIGIT  0b11000110 // 0xC6

uint8_t _ta6932_displayBuffer[5];
uint8_t _ta6932_brightness = 7;
bool _ta6932_displayOn = true;

static char ta6932_Digit_Table[] = {
  0x3F, // 0
  0x06, // 1
  0x5B, // 2
  0x4F, // 3
  0x66, // 4
  0x6D, // 5
  0x7D, // 6
  0x07, // 7
  0x7F, // 8
  0x6F, // 9

  0x77, // A
  0x7C, // b
  0x39, // C
  0x5E, // d
  0x79, // E
  0x71, // F
  0x00  // (OFF)
};

void TA6932_sendCommand(uint8_t value)
{
    digitalWrite(PIN_TA6932_STB, LOW);
    shiftOut(PIN_TA6932_DIN, PIN_TA6932_CLK, LSBFIRST, value);
    digitalWrite(PIN_TA6932_STB, HIGH);
}

void TA6932_empty()
{
    TA6932_sendCommand(TA6932_CMD_WRITE_INC); // set auto increment mode
    digitalWrite(PIN_TA6932_STB, LOW);
    shiftOut(PIN_TA6932_DIN, PIN_TA6932_CLK, LSBFIRST, TA6932_ADDR_FIRST_DIGIT);   // set starting address to 0
    for(uint8_t p=0;p<5;p++){
        shiftOut(PIN_TA6932_DIN, PIN_TA6932_CLK, LSBFIRST, 0x00);
        _ta6932_displayBuffer[p] = 0x00;
    }
    digitalWrite(PIN_TA6932_STB, HIGH);

    TA6932_sendCommand( TA6932_CMD_CTRL_BASE | ((uint8_t)_ta6932_displayOn << 3) | _ta6932_brightness );
}

void TA6932_begin()
{
    pinMode(PIN_TA6932_STB, OUTPUT);
    pinMode(PIN_TA6932_CLK, OUTPUT);
    pinMode(PIN_TA6932_DIN, OUTPUT);
    
    TA6932_empty();
}

#if TA6932_FUNC_SET_BRI == true
void TA6932_setBrightness(uint8_t b)
{
    if( b > 0x07){
        b = 0x07;
    }
    if( _ta6932_brightness != b ){
        _ta6932_brightness = b;
        TA6932_updateDisplay();
    }
}
#endif

#if TA6932_FUNC_DISPLAY == true
void TA6932_displayOn()
{
    if( !_ta6932_displayOn ){
        _ta6932_displayOn = true;
        TA6932_updateDisplay();
    }
}

void displayOff()
{
    if( _ta6932_displayOn ){
        _ta6932_displayOn = false;
        TA6932_updateDisplay();
    }
}
#endif

void TA6932_updateDisplay()
{
    TA6932_sendCommand(TA6932_CMD_WRITE_INC);
    digitalWrite(PIN_TA6932_STB, LOW);
    shiftOut(PIN_TA6932_DIN, PIN_TA6932_CLK, LSBFIRST, TA6932_ADDR_FIRST_DIGIT);   // set starting address to 0
    for(uint8_t p=0;p<5;p++){
        shiftOut(PIN_TA6932_DIN, PIN_TA6932_CLK, LSBFIRST, _ta6932_displayBuffer[p]);
    }
    digitalWrite(PIN_TA6932_STB, HIGH);

    TA6932_sendCommand( TA6932_CMD_CTRL_BASE | ((uint8_t)_ta6932_displayOn << 3) | _ta6932_brightness );
}

void TA6932_showTime(uint8_t left2, uint8_t right2, bool colonOn)
{
    uint8_t oleft2 = left2;
    uint8_t digit;
    if( right2 >= 0 ){
        //Fourth number
        digit = right2 % 10;
        _ta6932_displayBuffer[4] = ta6932_Digit_Table[digit];
        
        //Third number
        right2 = right2 / 10;
        digit = right2 % 10;
        _ta6932_displayBuffer[3] = ta6932_Digit_Table[digit];
    }else{
        _ta6932_displayBuffer[4] = 0x00;  // EMPTY
        _ta6932_displayBuffer[3] = 0x00;  // EMPTY
    }

    if(colonOn){
        _ta6932_displayBuffer[2] = 0x01;  // colon On
    }else{
        _ta6932_displayBuffer[2] = 0x00;  // colon Off
    }
    

    if( left2 >= 0 ){
        //Second number
        digit = left2 % 10;
        _ta6932_displayBuffer[1] = ta6932_Digit_Table[digit];
      
        //First number
        left2 = left2 / 10;
        digit = left2 % 10;
        if( oleft2 < 10U ) digit = 0x10; // EMPTY
        _ta6932_displayBuffer[0] = ta6932_Digit_Table[digit];
    }else if( left2 == -1 ){
        _ta6932_displayBuffer[1] = 0x00;  // EMPTY
        _ta6932_displayBuffer[0] = 0x00;  // EMPTY
    }else if( left2 == -2 ){
        _ta6932_displayBuffer[1] = 0x40;  // Hyphen
        _ta6932_displayBuffer[0] = 0x40;  // Hyphen
    }
    TA6932_updateDisplay();
}


//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

#define DS3231_I2C_ADDR          0x68  // CLOCK_ADDRESS
#define DS3231_ADDR_CMD_SECOND   0x00
#define DS3231_ADDR_CMD_MINUTE   0x01
#define DS3231_ADDR_CMD_HOUR     0x02
#define DS3231_ADDR_CMD_DOW      0x03


bool _DS3231_H12 = true;
bool _DS3231_PM = true;

uint8_t DS3231_bcd2dec( uint8_t val )
{
    return ( (val/16*10) + (val%16) );
}

uint8_t DS3231_dec2bcd( uint8_t val )
{
    return ( (val/10*16) + (val%10) );
}

uint8_t DS3231_readByte(uint8_t regAddr)
{
    I2C_write(DS3231_I2C_ADDR, regAddr);
    I2C_read(DS3231_I2C_ADDR, 1);
    uint8_t data = I2C_receive();
    return data;
}

#if DS3231_FUNC_SET_SEC == true
uint8_t DS3231_readControlByte( bool firstCtrlByte ){
    if( firstCtrlByte ){
        return DS3231_readByte( 0x0f );
    }else{
        return DS3231_readByte( 0x0e );
    }
}

void DS3231_writeControlByte(uint8_t control, bool firstCtrlByte){
    if( firstCtrlByte ){
        I2C_write_reg(DS3231_I2C_ADDR, 0x0f, control);
    }else{
        I2C_write_reg(DS3231_I2C_ADDR, 0x0e, control);
    }
}
#endif

uint8_t DS3231_getSecond()
{
    uint8_t seconds = DS3231_readByte(DS3231_ADDR_CMD_SECOND);
    return DS3231_bcd2dec(seconds);
}

uint8_t DS3231_getMinute()
{
    uint8_t minutes = DS3231_readByte(DS3231_ADDR_CMD_MINUTE);
    return DS3231_bcd2dec(minutes);
}

uint8_t DS3231_getHour()
{
    uint8_t hours = DS3231_readByte(DS3231_ADDR_CMD_HOUR);
    _DS3231_H12 = hours & 0b01000000;
    if( _DS3231_H12 ){
        _DS3231_PM = hours & 0b00100000;
        return DS3231_bcd2dec(hours & 0b00011111);
    }else{
        return DS3231_bcd2dec(hours & 0b00111111);
    }
    return hours;
}

#if DS3231_FUNC_SET_SEC == true
void DS3231_setSecond(uint8_t seconds)
{
    uint8_t val = DS3231_dec2bcd(seconds);
    I2C_write_reg(DS3231_I2C_ADDR, DS3231_ADDR_CMD_SECOND, val);

    //Reset OSF (Oscillator Stop Flag) flag
    uint8_t temp = DS3231_readControlByte( true );
    DS3231_writeControlByte( (temp & 0b01111111), 1);
}
#else
void DS3231_setSecond(uint8_t seconds){}
#endif

void DS3231_setMinute(uint8_t minutes)
{
    uint8_t val = DS3231_dec2bcd(minutes);
    I2C_write_reg(DS3231_I2C_ADDR, DS3231_ADDR_CMD_MINUTE, val);
}

void DS3231_setHour(uint8_t hours)
{
    uint8_t val;
    if(_DS3231_H12){
        if( hours > 12 ){
            val = DS3231_dec2bcd(hours-12) | 0b01100000;
        }else{
            val = DS3231_dec2bcd(hours) | 0b11011111;
        }
    }else{
        val = DS3231_dec2bcd(hours) & 0b10111111;
    }
    
    I2C_write_reg(DS3231_I2C_ADDR, DS3231_ADDR_CMD_HOUR, val);
}

#if DS3231_FUNC_GET_DOW == true
uint8_t DS3231_getDoW()
{
    // Gets the Day of Week
    uint8_t doW = DS3231_readByte(DS3231_ADDR_CMD_DOW);
    return DS3231_bcd2dec(doW);
}
#endif

#if DS3231_FUNC_SET_DOW == true
void DS3231_setDoW(uint8_t doW)
{
    // Sets the Day of Week
    uint8_t val = DS3231_dec2bcd(doW);
    I2C_write_reg(DS3231_I2C_ADDR, DS3231_ADDR_CMD_DOW, val);
}
#endif

#if DS3231_FUNC_GET_TEMP == true
float DS3231_getTemp()
{
    uint8_t tMSB, tLSB;
    float temp;

    //simply way to get 2 bytes from 0x11, but might fault
    tMSB = DS3231_readByte(0x11);
    tLSB = DS3231_readByte(0x12);

    temp = (((tMSB << 8) | tLSB) >> 6);
    temp = temp / 4.0;
    return temp; // Celcius
    //return (temp * 1.8) + 32.0; // Fahrenheit
}
#endif



uint32_t lastPressTs = 0;
uint32_t lastUpdateTs = 0;
uint16_t displayValue = 0;
uint8_t state = 0;
bool showSecondOnly = false;
bool editMode = false;
uint8_t editPart = 0;
bool editModeBlink = true;
int8_t set_H = 0;
int8_t set_M = 0;
int8_t set_S = 0;


void setup() {
    // put your setup code here, to run once:
    pinMode(PIN_BTN_A,  INPUT_PULLUP);
    pinMode(PIN_BTN_OK, INPUT_PULLUP);
    pinMode(PIN_BTN_B,  INPUT_PULLUP);
    
    TA6932_begin();
    I2C_begin();
}


void leavingEditMode()
{
    //Save To DS3231
    DS3231_setHour( set_H );
    DS3231_setMinute( set_M );
    DS3231_setSecond( set_S );
    editMode = false;
}


void loop() {
    // put your main code here, to run repeatedly:

    if( digitalRead( PIN_BTN_OK ) == LOW ){
        state = 1;
        delay( CFG_BTN_CD );
        if( digitalRead( PIN_BTN_OK ) == LOW ){
            delay( CFG_BTN_CD_LONG );
            if( digitalRead( PIN_BTN_OK ) == LOW ){
                state = 2;
            }
        }
    }

    if( editMode ){
        bool btnA = false;
        bool btnB = false;
        if( digitalRead( PIN_BTN_A ) == LOW ){
            btnA = true;
            delay( CFG_BTN_CD );
            lastPressTs = millis();
        }else if( digitalRead( PIN_BTN_B ) == LOW ){
            btnB = true;
            delay( CFG_BTN_CD );
            lastPressTs = millis();
        }

        if( state == 1 ){
            editPart++;
            if(editPart > 3) editPart = 1;
            lastPressTs = millis();
            state = 0;
        }else if( state == 2 ){
            leavingEditMode();
            state = 0;
        }else if( btnA || btnB ){
            if( editPart == 1 ){
                if( btnA )
                    set_H++;
                else
                    set_H--;
                if(set_H < 0 || set_H > 23) set_H = 0;
            }else if( editPart == 2 ){
                if( btnA )
                    set_M++;
                else
                    set_M--;
                if(set_M < 0 || set_M > 59) set_M = 0;
            }else if( editPart == 3 ){
                if( btnA )
                    set_S++;
                else
                    set_S--;
                if(set_S < 0 || set_S > 59) set_S = 0;
            }
        }
    }

    if( state == 1 ){
        state = 0;
        showSecondOnly = true;
        lastPressTs = millis();
    }else if( state == 2 ){
        state = 0;
        editMode = true;
        set_H = DS3231_getHour();
        set_M = DS3231_getMinute();
        set_S = DS3231_getSecond();
        lastPressTs = millis();
    }
    

    if( editMode && (lastPressTs + 15000L) <= millis() ){
        editMode = false;  // already at editMode in long enough, exit and NO saving
    }
    

    if( showSecondOnly && (lastPressTs + 5000L) <= millis() ){
        showSecondOnly = false;  // already show the Seconds in long enough, reset the showSecondOnly
    }
    
    if(  (lastUpdateTs + 1000L) < millis() ){
        lastUpdateTs = millis();
        if( editMode ){
            if( editModeBlink ){
                TA6932_empty();
            }else if(editPart == 0){
                TA6932_showTime(set_H, set_M, false);
            }else if( editPart == 1 ){
                TA6932_showTime(set_H, -1, false);  //Show Hour only
            }else if( editPart == 2 ){
                TA6932_showTime(-1, set_M, false);  //Show Minute only
            }else if( editPart == 3 ){
                TA6932_showTime(-2, set_S, false);  //Show Second only
            }
            editModeBlink = !editModeBlink;
        }else{
            if( showSecondOnly )
                TA6932_showTime(-1, DS3231_getSecond(), false);
            else{
                bool colonOn = (DS3231_getSecond() % 2) == 0;
                TA6932_showTime(DS3231_getHour(), DS3231_getMinute(), colonOn);
            }
        }
    }
}
