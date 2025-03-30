# PS2_KEYBOARD_KEYER_W7GLF
PS2 CW Keyboard Keyer For PERIXX-409 Keyboards

This is code adapted from Mark VandeWettering K6HX and Jan Swanepoel to allow an Arduino NANO or UNO to snd CW
using the Perixx-409 keyboard.  This keyboard requires an initial command to enable its ability to send SCAN codes.

I have modified the code to allow the use of Uparrow to increase the CW sending speed and Downarrow to decrease the CW sending speed.

I also added the ability to use the DEL key to send 8 dits (error code) and to use the semicolon to send SK.
The Left and Right arrows can be used to change the monitor frequency which by default is tied to pin 10.
All the pin assignments are at the top of the file.

This code also works on standard PS2 keyboards.

When the keyboard is plugged in the top status lights should blink once indicating the board is ready.

Made a few other improvements.  Now macros are called using function keys and added underscore to send AS.
In order to get function keys to work I had to move files from ps2keyboard PJRC library into this folder.
These are the files ps2keyboard.cpp, ps2keyboard.h (changed to handle function keys), and int_pins.h

I added the ability to do Farnsworth with characters at 30 WPM.  To enable/disable it use shift F12.


