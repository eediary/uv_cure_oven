// Wrapper for Stampduino
// Remaps Stampduino pins to match Micro

/*
 * ATMEGA32U4 Pin functions
 * 
 * PB0 - PCINT0, SS
 * PB1 - PCTIN1, SCLK
 * PB2 - PCINT2, MOSI,  PDI
 * PB3 - PCINT3, MISO,  PDO
 * PB4 - PCINT4, ADC11
 * PB5 - PCINT5, OC1A,  OC4B#, ADC12
 * PB6 - PCINT6, OC1B,  OC4B,  ADC13
 * PB7 - PCINT7, OC0A,  OC1C,  RTS#
 * 
 * PC6 - OC3A,   OC4A#
 * PC7 - OC4A,   CLK0,  ICP3
 * 
 * PD0 - SCL,    INT0,  OC0B
 * PD1 - SDA,    INT1
 * PD2 - RXD1,   INT2
 * PD3 - TXD1,   INT3
 * PD4 - ICP1,   ADC8
 * PD5 - XCK1,   CTS#
 * PD6 - OC4D#,  T1,    ADC9
 * PD7 - OC4D,   TO,    ADC10
 * 
 * PE2 - HWB#
 * PE6 - INT6,   AIN0
 * 
 * PF0 - ADC0
 * PF1 - ADC1
 * PF4 - ADC4,   TCK
 * PF5 - ADC5,   TMS
 * PF6 - ADC6,   TDO
 * PF7 - ADC7,   TDI
  */
  
#ifndef stampduino_h
#define stampduino_h

#define PAD_B0 17
#define PAD_B1 15
#define PAD_B2 16
#define PAD_B3 14
#define PAD_B4 8
#define PAD_B5 9
#define PAD_B6 10
#define PAD_B7 11

#define PAD_C6 5
#define PAD_C7 13

#define PAD_D0 3
#define PAD_D1 2
#define PAD_D2 0
#define PAD_D3 1
#define PAD_D4 4
#define PAD_D5 30
#define PAD_D6 12
#define PAD_D7 6

// PAD_PE2 is not accessible for the Micro
#define PAD_E6 7

#define PAD_F0 23
#define PAD_F1 22
#define PAD_F4 21
#define PAD_F5 20
#define PAD_F6 19
#define PAD_F7 18

#define LED_BUILTIN_GREEN PAD_B0
#define LED_BUILTIN_BLUE  PAD_B1

#endif
