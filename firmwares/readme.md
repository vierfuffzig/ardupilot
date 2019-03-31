**custom firmware for revomini**
-----------------------------------

RC input on RCInput signal pin 1 (PB14)

SERIAL1 = Telem1 = USART1 on Mainport

SERIAL2 = Telem2 = UART4 on PWM output pins 5(rx) & 6(tx)

SERIAL3 = GPS = USART6 on RCInput pins 3(tx) & 4(rx)

PWM outputs 5&6 moved to RCInput pins 5 & 6

I2C2 (external = bus 1) on flexiport

ext. SPI for SD-Card on OPLink port



***OPLink port pin order***
--------------------------

1 - Gnd

2 - +5

3 - PD2 (detect, not used)

4 - PA15 CS 

5 - PC10 SCK

6 - PC12 MOSI

7 - PC11 MISO
