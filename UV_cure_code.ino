#include <Wire.h>
#include <SPI.h>
#include <LiquidCrystal_PCF8574.h>

#include "Stampduino.h"

// Debug?
#define DEBUG

// GPIO
#define LED PAD_B0
#define LEFT PAD_B2
#define RIGHT PAD_B3
#define ENTER PAD_B4
#define CANCEL PAD_B5
#define UV_LED PAD_B6
#define SPEAKER PAD_B7

// Masks
#define L_MASK (_BV(PINB2))
#define R_MASK (_BV(PINB3))
#define E_MASK (_BV(PINB4))
#define C_MASK (_BV(PINB5))

// Sets or clears specified bit in specified register
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))

// LCD Address
#define LCD_ADDR 0x27

// Constants
#define DEBOUNCE_TIME 10 // debounce delay in ms
#define BEEP_TIME 800 // speaker duration in ms

// UI variables
enum state
{
  MENU,
  TIMER,
  RUN,
  DONE,
  BRIGHTNESS
};
state cur_state = MENU;
int menu_var = 0, timer_var = 0, brightness_var = 0;

// Timer variables
unsigned long start_time = 0, end_time = 0, cur_time = 0;

// volatile data
volatile bool buttons_flag = false;

LiquidCrystal_PCF8574 lcd(LCD_ADDR);

String str1 = "                ";
String str2 = "                ";
char str_temp1[] = "                ";
char str_temp2[] = "                ";
String old_str1 = "                ";
String old_str2 = "                ";

void setup() {
  // GPIO
  digitalWrite(LED, LOW);
  digitalWrite(UV_LED, LOW);
  digitalWrite(SPEAKER, LOW);
  pinMode(LED, OUTPUT);
  pinMode(LEFT, INPUT_PULLUP);
  pinMode(RIGHT, INPUT_PULLUP);
  pinMode(ENTER, INPUT_PULLUP);
  pinMode(CANCEL, INPUT_PULLUP);
  pinMode(UV_LED, OUTPUT);
  pinMode(SPEAKER, OUTPUT);

  // Interrupt setup
  sbi(PCICR, PCIE0); // enable interrupts on PCI0
  sbi(PCMSK0, PCINT2); // enables LEFT to trigger PCI0
  sbi(PCMSK0, PCINT3); // enables RIGHT to trigger PCI0
  sbi(PCMSK0, PCINT4); // enables ENTER to trigger PCI0
  sbi(PCMSK0, PCINT5); // enables CANCEL to trigger PCI0

  // Debug serial
  #ifdef DEBUG
  Serial.begin(115200);
  #endif

  // I2C
  Wire.begin();
  Wire.beginTransmission(LCD_ADDR);

  // LCD
  lcd.begin(16, 2);
  lcd.setBacklight(255);
  lcd.home();
  
}

void loop() {
  // get buttons
  byte buttons = debounce(&buttons_flag);

  // Update state machine based on buttons
  switch(cur_state){
    case MENU:
      // Menu; buttons let you cycle through options
      if(menu_var == 0){
        str1 = "TIMER";
      } else if(menu_var == 1){
        str1 = "ADJST BRIGHTNESS";
      }
      str2 = "<    >    O     ";

      // Disable UV LED + Speaker
      digitalWrite(UV_LED, LOW);
      digitalWrite(SPEAKER, LOW);

      // Read buttons
      if((buttons & L_MASK) == 0){
        menu_var = max(menu_var - 1, 0);
      }else if((buttons & R_MASK) == 0){
        menu_var = min(menu_var + 1, 1);
      } else if((buttons & E_MASK) == 0){
        if(menu_var == 0){
          cur_state = TIMER;
        } else if(menu_var == 1){
          cur_state = BRIGHTNESS;
        }
      }
      break;
    
    case TIMER:
      // Timer; buttons let you adjust how long to run UV lamp
      str1 = "TIME: " + String(30 + 10 * timer_var) + " min";
      str2 = "<    >    O    X";

      // Disable UV LED + Speaker
      digitalWrite(UV_LED, LOW);
      digitalWrite(SPEAKER, LOW);
      
      // Read buttons
      if((buttons & L_MASK) == 0){
        timer_var = max(timer_var - 1, 0);
      }else if((buttons & R_MASK) == 0){
        timer_var = min(timer_var + 1, 15);
      } else if((buttons & E_MASK) == 0){
        start_time = millis();
        end_time = start_time + (30 + 10 * (unsigned long)timer_var) * 60 * 1000;
        cur_state = RUN;
      } else if((buttons & C_MASK) == 0){
        cur_state = MENU;
      }
      break;
      
    case RUN:
      // RUN; allows UV chamber to run
      cur_time = millis()-start_time;
//      cur_time = cur_time * 600; // for debugging
      unsigned long mins, secs;
      
      secs = cur_time/1000;
      mins = secs/60;
      secs = secs - 60 * mins;      

      // format str1
      if(mins < 10){
        str1 = "0" + String(mins);
      } else{
        str1 = String(mins);
      }
      str1 = str1 + ":";
      if(secs < 10){
        str1 = str1 + "0" + String(secs);
      } else{
        str1 = str1 + String(secs);
      }
      str1 = str1 + " / " + String(30+10*timer_var) + " min";
      str2 = "               X";
      
      // Change when time is done
      if(mins >= (30 + 10*timer_var)){
        cur_state = DONE;
        end_time = millis() + BEEP_TIME;
      }

      // Disable speaker
      digitalWrite(SPEAKER, LOW);
      
      // Write to UV LED
      analogWrite(UV_LED, map(100 - 10*brightness_var, 0, 100, 0, 255));
      
      // Read buttons
      if((buttons & C_MASK) == 0){
        cur_state = MENU;
      }
      break;

    case DONE:
      // DONE: play speaker for set amount of time
      cur_time = millis();
      if(cur_time < end_time){
        str1 = "Done!";
        str2 = "";

        // play multiple beeps
        if(cur_time < (end_time - ((float)BEEP_TIME*2/3))){
          analogWrite(SPEAKER, 128);
        } else if(cur_time < (end_time - ((float)BEEP_TIME*1/3))){
          digitalWrite(SPEAKER, LOW);
        } else{
          analogWrite(SPEAKER, 128);
        }
      } else{
        digitalWrite(SPEAKER, 0);
        cur_state = MENU;
      }
      break;
      
    case BRIGHTNESS:
      // BRIGHTNESS; can adjust UV brightness here
      str1 = "LEVEL: " + String(100 - 10 * brightness_var) + "%";
      str2 = "<    >    O     ";

      // Disable UV LED
      digitalWrite(UV_LED, LOW);
      
      // Read buttons
      if((buttons & L_MASK) == 0){
        brightness_var = min(brightness_var + 1, 9);
      }else if((buttons & R_MASK) == 0){
        brightness_var = max(brightness_var - 1, 0);
      } else if((buttons & E_MASK) == 0){
        cur_state = MENU;
      }
      break;
  }
  
  // Only update LCD as necessary
  if(!(old_str1 == str1) || !(old_str2 == str2)){
    lcd.clear();
    lcd.print(str1);
    lcd.setCursor(0,1);
    lcd.print(str2);
  }
  old_str1 = str1;
  old_str2 = str2;
}

// debounce function
// checks buttons after debounce time
byte debounce(volatile bool *flag){
  static unsigned long start_time = 0;
  if(*flag){
    if(millis() > start_time + DEBOUNCE_TIME){
      // debounce time has passed; clear flag + sample
      *flag = false;
      return (PINB & (L_MASK | R_MASK | E_MASK | C_MASK));
    } else{
      // still debouncing, so nothing to report
      return 0xFF;
    }
  } else{
    // no flag set, so update start_time
    start_time = millis();
    return 0xFF;
  }
}

// ISR for when any button is pressed
ISR (PCINT0_vect)
{
  // Interrupt triggers if any of the buttons are pressed
  // Set flag, will be read by debounce function
  buttons_flag = true;
}
