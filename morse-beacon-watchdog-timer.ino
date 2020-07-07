//
// Morse code generation routines adapted from "Simple Arduino Morse Beacon" by Mark VandeWettering K6HX

#include <avr/sleep.h> //Needed for sleep_mode
#include <avr/wdt.h> //Needed to enable/disable watch dog timer

struct t_mtab { char c, pat; } ;

struct t_mtab morsetab[] = {
  {'.', 106},
  {',', 115},
  {'?', 76},
  {'/', 41},
  {'A', 6},
  {'B', 17},
  {'C', 21},
  {'D', 9},
  {'E', 2},
  {'F', 20},
  {'G', 11},
  {'H', 16},
  {'I', 4},
  {'J', 30},
  {'K', 13},
  {'L', 18},
  {'M', 7},
  {'N', 5},
  {'O', 15},
  {'P', 22},
  {'Q', 27},
  {'R', 10},
  {'S', 8},
  {'T', 3},
  {'U', 12},
  {'V', 24},
  {'W', 14},
  {'X', 25},
  {'Y', 29},
  {'Z', 19},
  {'1', 62},
  {'2', 60},
  {'3', 56},
  {'4', 48},
  {'5', 32},
  {'6', 33},
  {'7', 35},
  {'8', 39},
  {'9', 47},
  {'0', 63}
} ;

#define N_MORSE  (sizeof(morsetab)/sizeof(morsetab[0]))

#define SPEED  (20)
#define DOTLEN  (1200/SPEED)
#define DASHLEN  (3*(1200/SPEED))
#define TONEfreq (1000)
#define WAITLOOPS (3) //number of 8s loops to sleep before resending

int LEDpin = 1; //Data pin
int TXpin = 2; //Transmitter power pin

void
dash()
{
  analogWrite(LEDpin, 128);
  delay(DASHLEN);
  digitalWrite(LEDpin, LOW);
  delay(DOTLEN);
}

void
dit()
{
    analogWrite(LEDpin, 128);
  delay(DOTLEN);
  digitalWrite(LEDpin, LOW);
  delay(DOTLEN);
}

void
send(char c)
{
  int i ;
  if (c == ' ') {
    delay(7*DOTLEN) ;
    return ;
  }
  for (i=0; i<N_MORSE; i++) {
    if (morsetab[i].c == c) {
      unsigned char p = morsetab[i].pat ;
      while (p != 1) {
          if (p & 1)
            dash() ;
          else
            dit() ;
          p = p / 2 ;
      }
      delay(3*DOTLEN) ;
      return ;
    }
  }
}

void
sendmsg(char *str)
{
  digitalWrite(TXpin, HIGH);
  while (*str)
    send(*str++) ;
  digitalWrite(TXpin, LOW);
}

//This runs each time the watch dog wakes us up from sleep
ISR(WDT_vect) {
  //Don't do anything. This is just here so that we wake up.
}

//Sets the watchdog timer to wake us up, but not reset
//0=16ms, 1=32ms, 2=64ms, 3=128ms, 4=250ms, 5=500ms
//6=1sec, 7=2sec, 8=4sec, 9=8sec
//From: http://interface.khm.de/index.php/lab/experiments/sleep_watchdog_battery/
void setup_watchdog(int timerPrescaler) {

  if (timerPrescaler > 9 ) timerPrescaler = 9; //Limit incoming amount to legal settings

  byte bb = timerPrescaler & 7; 
  if (timerPrescaler > 7) bb |= (1<<5); //Set the special 5th bit if necessary

  //This order of commands is important and cannot be combined
  MCUSR &= ~(1<<WDRF); //Clear the watch dog reset
  WDTCR |= (1<<WDCE) | (1<<WDE); //Set WD_change enable, set WD enable
  WDTCR = bb; //Set new watchdog timeout value
  WDTCR |= _BV(WDIE); //Set the interrupt enable, this will keep unit from resetting after each int
}

void setup() {
  ADCSRA &= ~(1<<ADEN); //Disable ADC, saves ~230uA
  pinMode(LEDpin, OUTPUT);
  pinMode(TXpin, OUTPUT);
  pinMode(0, OUTPUT); // Set other pins to output to save power
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); //Power down everything, wake up from WDT
  sleep_enable();
  setup_watchdog(9); //Setup watchdog to go off after 8sec
}

void loop() {
  sendmsg("LOCATE ME TO FIND TREASURE");
  int i;
  for (i=0; i < WAITLOOPS; i++) {
      sleep_mode(); //Go to sleep! Wake up 8sec later
    }
 }
