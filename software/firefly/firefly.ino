#include <avr/sleep.h>
#include <avr/wdt.h>

#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbijust
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif


int i;
int led = 4;
int seed_analog_pin = 2;
int on_delay = 1;
int off_delay = 1;

volatile boolean f_wdt = 1;
volatile boolean led_on = 0;


void setup() {
  // put your setup code here, to run once:
  pinMode(led, OUTPUT);
  pinMode(seed_analog_pin, INPUT);
  randomSeed(analogRead(seed_analog_pin));

  on_delay = random(1,25);
  off_delay = random(1,25);
  
  setup_watchdog(9); // approximately 4 seconds sleep
}


void flash_on(){
  // put your main code here, to run repeatedly:
  for(i=0; i<255; i++){
    analogWrite(led, i);
    delay(on_delay);
  }
}

void flash_off(){
  for(i=255; i>0; i--){
    analogWrite(led, i);
    delay(off_delay);
  }
  analogWrite(led,0);
}

void loop() {
  if (f_wdt==1) {  // wait for timed out watchdog / flag is set when a watchdog timeout occurs
    f_wdt=0;       // reset flag


    if (led_on) {
      flash_off();
      led_on = 0;
      setup_watchdog(random(7,9)); // approximately 8 seconds sleep
    } else {
      flash_on();
      led_on = 1;
      setup_watchdog(random(6,9)); // approximately 8 seconds sleep
    }
    system_sleep();
    
  }

}


// set system into the sleep state 
// system wakes up when wtchdog is timed out
void system_sleep() {
  cbi(ADCSRA,ADEN);                    // switch Analog to Digitalconverter OFF

  set_sleep_mode(SLEEP_MODE_PWR_DOWN); // sleep mode is set here
  sleep_enable();

  sleep_mode();                        // System sleeps here

  sleep_disable();                     // System continues execution here when watchdog timed out 
  sbi(ADCSRA,ADEN);                    // switch Analog to Digitalconverter ON
}

// 0=16ms, 1=32ms,2=64ms,3=128ms,4=250ms,5=500ms
// 6=1 sec,7=2 sec, 8=4 sec, 9= 8sec
void setup_watchdog(int ii) {

  byte bb;
  int ww;
  if (ii > 9 ) ii=9;
  bb=ii & 7;
  if (ii > 7) bb|= (1<<5);
  bb|= (1<<WDCE);
  ww=bb;

  MCUSR &= ~(1<<WDRF);
  // start timed sequence
  WDTCR |= (1<<WDCE) | (1<<WDE);
  // set new watchdog timeout value
  WDTCR = bb;
  WDTCR |= _BV(WDIE);
}
  
// Watchdog Interrupt Service / is executed when watchdog timed out
ISR(WDT_vect) {
  f_wdt=1;  // set global flag
}
