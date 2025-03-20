# PS2_KEYBOARD_KEYER_W7GLF
PS2 CW Keyboard Keyer For PERIXX Keyboards

This is code adapted from Mark VandeWettering K6HX and Jan Swanepoel to allow an Arduino NANO or UNO to snd CW
using the Perixx-409 keyboard.  This keyboard requires an initial command to enable its ability to send SCAN codes.

I have also modified to allow the use of Uparrow to increase the CW sending speed and Downarrow to decrease the CW sending speed.
I also added the ability to use the DEL key to send 8 dits (error code) and to use the semi-colon to send SK.
The Left and Right arrows can be used to change the monitor frequency which by default is tied to pin 10.  All the pin 
assignments are at the top of the file.
