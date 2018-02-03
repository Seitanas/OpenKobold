/*
 * File:   main.c
 * Author: Seitan
 *
 * Created on Ketvirtadienis, 2017, Gruodžio 21, 09.10
 */
#pragma config FOSC = INTRCIO   // Oscillator Selection bits (INTOSC oscillator: I/O function on GP4/OSC2/CLKOUT pin, I/O function on GP5/OSC1/CLKIN)
#pragma config WDTE = ON      // Watchdog Timer Enable bit (WDT enabled)
#pragma config PWRTE = OFF      // Power-Up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // GP3/MCLR pin function select (GP3/MCLR pin function is digital I/O, MCLR internally tied to VDD)
#pragma config BOREN = OFF      // Brown-out Detect Enable bit (BOD disabled)
#pragma config CP = OFF         // Code Protection bit (Program Memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)

#define _XTAL_FREQ 4000000

#define INTERVAL 1 // how often (in hours) should beep occur

#define PULSE_LENGTH1 230  // pulse lenghth in us (first tone)
#define PULSE_LENGTH2 390 // pulse lenghth in us (second tone)
#define BEEPER_PIN GPIO0
#include <xc.h>

void initial_delay(){
    /*
     * we cannot use __delay_ms(2000),
     * because watchdog will reset device after 1.1s.
     * we should loop through 200ms delays and clear
     * watchdog prescaler each time.
     */
    int x=10;
    while (x){
        __delay_ms(200);
        CLRWDT();
        --x;
    }    
    
}

void kobold_beep(int interval1, int interval2){
    // interval = how long singal will be emmitted (pulse_length*interval) = total signal time.
    int signal = 1;
    while (interval1){
        BEEPER_PIN = signal;
        signal = ~signal;
        __delay_us(PULSE_LENGTH1);
        
        /* we need to clear watchdog prescaler each time, because if
         * watchdog timer overflows when microcontroller is awake,
         * it will reset it. It should not overflow because this function takes
         * less than 1.1s interval, but it's better to take precautions.
         */
        CLRWDT(); 
        --interval1;
    }
    signal = 1;
    while (interval2){
        BEEPER_PIN = signal;
        signal = ~signal;
        __delay_us(PULSE_LENGTH2);
        
        /* we need to clear watchdog prescaler each time, because if
         * watchdog timer overflows when microcontroller is awake,
         * it will reset it. It should not overflow because this function takes
         * less than 1.1s interval, but it's better to take precautions.
         */
        CLRWDT(); 
        --interval2;
    }
    BEEPER_PIN = 0;
}


void main(void) {
    TRISIO = 0x00;
    BEEPER_PIN = 0;
    
    // prescaler to WDT:
    PSA = 1;
    
    // 1.1s:
    PS0 = 0;
    PS1 = 1;
    PS2 = 1;
    // short beep on startup.
    kobold_beep(50, 0);
    
    /* delay at initial startup - without delay pc becomes
     * irresponsive to some programmers
     * and is no longer re-programmable. 
     * (erasing in low-voltage mode fixes microcontroller though)
     */    
    initial_delay();
    int seconds = 0;
    int minutes = 0;
    int hours = 0;
    int random_minute = 0;
    int has_run = 0;
    int interval = INTERVAL;
    random_minute = rand() % 60;
    while (1){
        ++seconds;
        if (seconds == 60){
            seconds = 0;
            ++minutes;
        }
        if (minutes == 60){
            minutes = 0;
            ++hours;
            has_run = 0;
            ++interval;
        }
        if (hours == 24)
            hours = 0;
        if (minutes == random_minute && !has_run && interval == INTERVAL){
            random_minute = rand() % 60;
            has_run = 1;
            interval = 0;
            kobold_beep(640, 128);
        }
        /* enter low-power sleep mode.
         * Watchdog timer will wake processor in 1.1s
         */ 
        SLEEP(); 
    }
}
