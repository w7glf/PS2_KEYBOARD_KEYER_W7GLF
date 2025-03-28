#include <Keyboard.h>
#include <KeyboardLayout.h>

// W7GLF - Use local files and not library so I can use Function Keys
//#include <PS2Keyboard.h>
#include "PS2Keyboard.h"
 
//  _____ _            _          _      _            __  __                 
// |_   _| |_  ___    /_\  _ _ __| |_  _(_)_ _  ___  |  \/  |___ _ _ ___ ___ 
//   | | | ' \/ -_)  / _ \| '_/ _` | || | | ' \/ _ \ | |\/| / _ \ '_(_-</ -_)
//   |_| |_||_\___| /_/ \_\_| \__,_|\_,_|_|_||_\___/ |_|  |_\___/_| /__/\___|
//                                                                           
//  _  __                 
// | |/ /___ _  _ ___ _ _ 
// | ' </ -_) || / -_) '_|
// |_|\_\___|\_, \___|_|  
//           |__/   
//
// Version for the PS2 Keyboard
// using the library from http://www.pjrc.com/teensy/td_libs_PS2Keyboard.html
// 
// Written by Mark VandeWettering K6HX
//
// This is just a quick Morse keyer program.
//
/*
 * Code to reset keyboard for PS2 Keyboard Article
 * Jan Swanepoel, 2019 - added by W7GLF to allow working with Perixx keyboard
 *                       which requires a PS/2 keyboard comamnd to enable keycode scanning. 
 * 28-JAN-2025  Modified code to allow the use of Macros called up ny using the Function keys.  I also added DEL key
 *              to send 8 dots (error) and _ (underscore) to send prosign AS (standby).
 *              Added PAGE_DOWN to do Key Down and PAGE_UP to do Key Up
 * 
*/
 
////////////////////////////////////////////////////////////////////////
//
// Here is a queue to store the characters that I've typed.
// To simplify the code, it can store a maximum of QUEUESIZE-1 characters
// before it fills up.  What is a byte wasted between friends?
//
////////////////////////////////////////////////////////////////////////

// QUEUESIZE must be a power of two 
#define QUEUESIZE       (128)
#define QUEUEMASK       (QUEUESIZE-1)
#define DEBUG false
#define USE_FUNCTION_KEYS true

int aborted = 0 ;
int qhead = 0 ;
int qtail = 0 ;
char queue[QUEUESIZE] ;
 
unsigned int freq = 700;
unsigned int wpm = 15;

////////////////////////////////////////////////////////////////////////
#define pin 13                  // blink the LED for now...
#define keyboard_data 4 
#define keyboard_clock 3 
#define tpin 10                 // tone pin
#define SPEAKER false           // Generate tones
int ditlen = 1200 / wpm ;

#define CALLSIGN "W7GLF "

char CQ_FD_MSG [] = "CQ CQ FD DE " CALLSIGN CALLSIGN "K\r\n";
char CALL_MSG [] = CALLSIGN CALLSIGN "K K\r\n";
char RSP_FD_MSG [] = "1E 1E WWA WWA 73 TU\r\n";
char QRZ_FD_MSG [] = "QRZ FD DE " CALLSIGN "K\r\n";
char CQ_MSG [] = "CQ CQ CQ DE " CALLSIGN CALLSIGN "K\r\n";
char RSP_MSG [] = "RRR 73 TU " CALLSIGN ": E E\r\n";

void
queueadd(char ch)
{
    queue[qtail++] = ch ;
    qtail &= QUEUEMASK ;
}
 
void
queueadd(char *s)
{
  while (*s)
      queueadd(*s++) ;
}
 
char
queuepop()
{
    char ch ;
    ch = queue[qhead++] ;
    qhead &= QUEUEMASK ;
    return ch ;
}
 
int
queuefull()
{
    return (((qtail+1)%QUEUEMASK) == qhead) ;
}
 
int
queueempty()
{
    return (qhead == qtail) ;
}
 
void
queueflush()
{
    qhead = qtail ;
}
 
 
PS2Keyboard kbd ;
 
inline void
ps2flush()
{
    while (kbd.available());
} 
inline void
ps2poll()
{    
    char ch ;
 
    while (kbd.available()) {
        if (queuefull()) {
            Serial.print("") ;
        } else {
            switch (ch=kbd.read()) {
            case PS2_ENTER:
                break ;
            case PS2_TAB:
                break ;
            case PS2_PAGEDOWN:
                keydown();
                break ;
            case PS2_PAGEUP:
                keyup();
                break ;
            case PS2_LEFTARROW:
                if (freq < 300) break;
                freq -= 50;
#if DEBUG 
                Serial.print(F("FREQUENCY = ")) ;
                Serial.println(freq) ;
#endif                
                break ;
            case PS2_RIGHTARROW:
                if (freq > 2000) break;
                freq += 50; 
#if DEBUG 
                Serial.print(F("FREQUENCY = ")) ;
                Serial.println(freq) ;                
#endif                
                break ;
            case PS2_UPARROW:
                if (wpm < 5) break;
                wpm -= 1; 
                ditlen = 1200 / wpm ;
#if DEBUG 
                Serial.print(F("WPM = ")) ;
                Serial.println(wpm) ;                
#endif                
                break ;
            case PS2_DOWNARROW:
                if (wpm > 30) break;
                wpm += 1; 
                ditlen = 1200 / wpm ;
#if DEBUG 
                Serial.print(F("WPM = ")) ;
                Serial.println(wpm) ;                
#endif                
                break ;
            case PS2_DELETE:
                queueadd(0x7F) ; // code for DELETE
                break ;
            case PS2_ESC:
                queueflush() ;
                Serial.flush() ;
                Serial.println(F("== FLUSH ==")) ;
                aborted = 1 ;
                break ;
#if USE_FUNCTION_KEYS
            case PS2_F1:
                queueadd(CQ_FD_MSG);
                break ;
            case PS2_F2:
                queueadd(CALL_MSG);
                break ;
            case PS2_F3:
                queueadd(RSP_FD_MSG);
                break ;
            case PS2_F4:
                queueadd(QRZ_FD_MSG);
                break ;
            case PS2_F5:
                queueadd(CQ_MSG);
                break ;
            case PS2_F6:
                queueadd(RSP_MSG);
                break ;
#else
            case '!':
                queueadd(CQ_FD_MSG);
                break ;
            case '@':
                queueadd(CALL_MSG);
                break ;
            case '#':
                queueadd(RSP_FD_MSG);
                break ;
            case '$':
                queueadd(QRZ_FD_MSG);
                break ;
            case '%':
                queueadd(CQ_MSG);
                break ;
            case '^':
                queueadd(RSP_MSG);
                break ;
#endif
            default:
                queueadd(ch) ;
                break ;
            }
        }
    }
}
 
void
mydelay(unsigned long ms)
{
    unsigned long t = millis() ;
    while (millis()-t < ms)
        ps2poll() ;
}
  
void
scale()
{
  long f = 220L ;
  int i ;
   
  for (i=0; i<=12; i++) {
      #if SPEAKER 
         tone(tpin, (int)f) ;
      #endif
      f *= 1059L ;
      f /= 1000L ;
      Serial.println(f) ;
      delay(300) ;
  }
  #if SPEAKER
     noTone(tpin) ;
  #endif       
}
 
void
dit()
{
    digitalWrite(pin, HIGH) ;
    #if SPEAKER 
       tone(tpin, freq) ;
    #endif
    mydelay(ditlen) ;
    digitalWrite(pin, LOW) ;
    #if SPEAKER 
      noTone(tpin) ;
    #endif
    mydelay(ditlen) ;
}
 
void
dah()
{
    digitalWrite(pin, HIGH) ;
    #if SPEAKER 
       tone(tpin, freq) ;
    #endif
    mydelay(3*ditlen) ;
    digitalWrite(pin, LOW) ;
    #if SPEAKER 
      noTone(tpin) ;
    #endif
    mydelay(ditlen) ;
}
 
void
keydown()
{
    digitalWrite(pin, HIGH) ;
}
 
void
keyup()
{
    digitalWrite(pin, LOW) ;
}
 
void
lspace()
{
    mydelay(2*ditlen) ;
}
 
void
space()
{
    mydelay(4*ditlen) ;
}
 

//  The following code was written by Jan Swanepoel 
/*
 * Example code for PS2 Keyboard Article
 * Jan Swanepoel, 2019
*/

// Set which pins are clock and data
#define pinClock    keyboard_clock
#define pinData     keyboard_data
    
uint8_t clkPrev;
uint8_t keyCodeReceived;

// Parity check
// Return true for odd parity.
bool ParityCheck(uint16_t data)
{
  uint8_t count = 0;

  // Test first 10 bits, since start is always 0 it does not matter to include it
  for (int i = 0; i < 10; i++)
    if (data & (1 << i))
      count++;

  // Check if odd
  if (count % 2)
    return true;
      
  return false;
}


// Listen for scan codes from keyboard
bool Listen(uint8_t* keyCode)
{
  static uint16_t rxCode = 0;               // 16 bit variable use to store the received bits
  static uint8_t rxBitCount = 0;            // Keep track of number of bits clocked

  uint8_t clkNow = digitalRead(pinClock);   // Get current clock level
   
  // Look for falling clock edge
  if (clkNow < clkPrev) 
  {      
    if (digitalRead(pinData) > 0)
      rxCode = rxCode | 0x0400;             // Sets the 11th bit
    else
      rxCode = rxCode & 0x0BFF;             // Clears the 11th bit
      
    rxBitCount++;
 
    if (rxBitCount == 11)                   // New 11 bit data received from keyboard?
    {
      if (ParityCheck(rxCode))              // Retrieve keyCode if parity bit is valid
      {
        rxCode = rxCode >> 1;               // Remove start bit
        *keyCode = rxCode & 0x00FF;         // Remove parity & stop bits and assign the key to keyCode
        clkPrev = clkNow;
        rxBitCount = 0;
        rxCode = 0;
        return true;
      }
      else
      {
        // Parity check failed
      }

      rxCode = 0;
      rxBitCount = 0;
    }
    rxCode = rxCode >> 1;                 // Shift rxCode to ready for next bit
  }

  clkPrev = clkNow;
  return false;
}


void Command(uint8_t cmdCode)
{
  uint16_t cmd = cmdCode;                 // Cast to 16 bit

  if (!ParityCheck(cmd))
    cmd = cmd | 0x0100;                   // Update parity bit

   cmd |= 0x0200;                         // Add stop bit

  // Set pins to output
  pinMode(pinClock, OUTPUT);
  pinMode(pinData, OUTPUT);

  // Start Request to Send
  // Pull clock low for 100us
  digitalWrite(pinClock, LOW);
  delayMicroseconds(100);

  // Pull data low (which represents the start bit)
  digitalWrite(pinData, LOW);
  delayMicroseconds(20);

  // Release clock
  pinMode(pinClock, INPUT_PULLUP);

  // Write 8 data bits + 1 parity bit + 1 stop bit
  for (int i = 0; i < 10; i++)
  {
    // Wait for keyboard to take clock low
    while(digitalRead(pinClock) > 0);
    
    // Load bit  
    digitalWrite(pinData, cmd & 0x0001);
    cmd = cmd >> 1;

    // Wait for keyboard to take clock high (sample bit)
    while(digitalRead(pinClock) < 1); 
  }
  
  // Release data line (which keeps it high for the stop bit)
  pinMode(pinData, INPUT_PULLUP);

  // Wait for keyboard to take clock low
  while(digitalRead(pinClock) > 0);
  
  //Wait for data low, which would be the ack from keyboard
  while(digitalRead(pinData) > 0);  
}
//  End of code written by Jan Swanepoel 
 
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
 
 
char ltab[] = {
    0b101,              // A
    0b11000,            // B 
    0b11010,            // C
    0b1100,             // D
    0b10,               // E
    0b10010,            // F
    0b1110,             // G
    0b10000,            // H
    0b100,              // I
    0b10111,            // J
    0b1101,             // K
    0b10100,            // L
    0b111,              // M
    0b110,              // N
    0b1111,             // O
    0b10110,            // P
    0b11101,            // Q
    0b1010,             // R
    0b1000,             // S
    0b11,               // T
    0b1001,             // U
    0b10001,            // V
    0b1011,             // W
    0b11001,            // X
    0b11011,            // Y
    0b11100             // Z
} ;
 
char ntab[] = {
    0b111111,           // 0
    0b101111,           // 1
    0b100111,           // 2
    0b100011,           // 3
    0b100001,           // 4
    0b100000,           // 5
    0b110000,           // 6
    0b111000,           // 7
    0b111100,           // 8
    0b111110            // 9
} ;

void
sendcode_error(void)
{
  int i;

  for (i = 0; i<8; i++) {
    dit();
  }
  lspace();
}
 
void
sendcode(char code)
{
    int i ;
 
    for (i=7; i>= 0; i--)
        if (code & (1 << i))
            break ;
 
    for (i--; i>= 0; i--) {
        if (code & (1 << i))
            dah() ;
        else
            dit() ;
    }
    lspace() ;
}
 
void
send(char ch)
{
 
    if (isalpha(ch)) {
        if (islower(ch)) ch = toupper(ch) ;
        sendcode(ltab[ch-'A']) ;
    } else if (isdigit(ch))
        sendcode(ntab[ch-'0']) ;
    else if (ch == ' ' || ch == '\r' || ch == '\n')
        space() ;
    else if (ch == '.')
        sendcode(0b1010101) ;
    else if (ch == ',')
        sendcode(0b1110011) ;
    else if (ch == '!')
        sendcode(0b1101011) ;
    else if (ch == '?')
        sendcode(0b1001100) ;
    else if (ch == '/')
        sendcode(0b110010) ;
    else if (ch == '+')
        sendcode(0b101010) ;
    else if (ch == '-')
        sendcode(0b1100001) ;
    else if (ch == '=')
        sendcode(0b110001) ;
    else if (ch == '@')         // hardly anyone knows this!
        sendcode(0b1011010) ;
    else if (ch == ':')         // SK
        sendcode(0b1000101) ;
    else if (ch == '_')         // AS Standby
        sendcode(0b101000) ;
    else if (ch == PS2_DELETE)
        sendcode_error() ;
    else
        return ;                // ignore anything else
 
    if (!aborted) {
      Serial.print(ch) ;
      if (ch == 13) Serial.print((char) 10) ;
    }
    aborted = 0 ;
}
 
////////////////////////////////////////////////////////////////////////

void
setup()
{
  uint8_t ack = 0;
   
    pinMode(pin, OUTPUT) ;
    pinMode(tpin, OUTPUT) ; 
  // Set pins to input with pull-up resistors
  pinMode(pinClock, INPUT_PULLUP);
  pinMode(pinData, INPUT_PULLUP);
    Serial.begin(9600) ;
    kbd.begin(keyboard_data, keyboard_clock) ;
    Serial.println(F("Morse Code Keyboard by K6HX")) ;
    Serial.println(F("Adaptation for Perixx PS2 keyboard by W7GLF")) ;
  

  delay(100);

  // Switch on CAPS, NUM and SCROLL lock LEDs
  Command(0xED);              // Send command
  while(!Listen(&ack));       // Wait for acknowledge
  
  Command(0x07);              // Send argument
  while(!Listen(&ack));       // Wait for acknowledge

  delay (500);

  // Switch off CAPS, NUM and SCROLL lock LEDs
  Command(0xED);              // Send command
  while(!Listen(&ack));       // Wait for acknowledge
  
  Command(0x00);              // Send argument
  while(!Listen(&ack));       // Wait for acknowledge

  ps2poll() ;                 // Make sure buffer is empty before we start
  queueflush();
}



void
loop()
{
    ps2poll() ;
 
    if (!queueempty()) {
        send(queuepop()) ;
    }
}
