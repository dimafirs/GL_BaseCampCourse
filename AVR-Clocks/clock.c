#ifndef F_CPU
#define F_CPU   16000000UL
#endif
#define __DELAY_BACKWARD_COMPATIBLE__

#include "clock.h"
#include "display.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdbool.h>

/* bool btn_go_low() - return true if button in parameters is pressed
 @btn - HAL struct of button pin 
 Author: Timofei Hodnev (@thodnev) */ 
bool btn_go_low(struct segm_Pin *btn)
{
	if (!(*(btn->port->PIN) & (1 << btn->pin))) {
		_delay_ms(5);
		if (!(*(btn->port->PIN) & (1 << btn->pin)))
			return true;
	}
	return false;
}

/* Some HALs for ports and peripherals */
static struct segm_Port PD = {
	.DDR = &DDRD,
	.PIN = &PIND,
	.PORT = &PORTD,
};

static struct segm_Port PB = {
	.DDR = &DDRB,
	.PIN = &PINB,
	.PORT = &PORTB,
};

static struct segm_Display display = {
	.SHCP = {.port = &PD, .pin = 2},
	.STCP = {.port = &PD, .pin = 3},
	.DS   = {.port = &PD, .pin = 4},
	.delay_func = &_delay_loop_1,	/* 3 cycles / loop, busy wait */
	.sleep_ms_func = &_delay_ms,	/* 3 cycles / loop, busy wait */
	.is_comm_anode = false		/* We have common cathode display */
};



struct segm_Pin mode_btn = {.port = &PB, .pin = 2},
		inc_btn = {.port = &PB, .pin = 1},
		dec_btn = {.port = &PB, .pin = 0},
		alarm_pin = {.port = &PB, .pin = 3};

/* init_io() - initialize all peripherals needed for clocks*/ 
void init_io(void){
	segm_init(&display);
	*(mode_btn.port->DDR) &= ~(1 << mode_btn.pin);
	*(inc_btn.port->DDR) &= ~(1 << inc_btn.pin);
	*(dec_btn.port->DDR) &= ~(1 << dec_btn.pin);
	*(alarm_pin.port->DDR) |= (1 << alarm_pin.pin);
	
	/* PCINT setup */
	PCICR |= (1 << PCIE0);
	PCMSK0 |= (1 << PCINT2) | (1 << PCINT1) | (1 << PCINT0);

	/* Configure timer */
	TCNT1 = 0;
	TCCR1A &=  ~((1 << WGM11) | (1 << WGM10) );
	TCCR1A |= (1 << COM1A0);
	/* Prescalar 1024 and CTC mode) */ 
	TCCR1B |= (1 << WGM12) | (1 << CS12) | (1 << CS10); 
	/* Output compare value for 1s */	
	OCR1A = 15625;  
	/* Enable compare iterrupt */
	TIMSK1 |= (1 << OCIE1A);
    	/* Enable global interrupts */
    	sei();
}

struct Time current_time = {
	.hours = 15, 
	.minutes = 32,
	.seconds = 0
};

struct Time alarm_time = {
	.hours = 15, 
	.minutes = 52,
	.seconds = 0
};

/* cast_time() - cast time from Time struct to 7-segm symbols 
 @sym_arr - array of uint8_t for symbols 
 @time - struct with time that need to be casted */
void cast_time(uint8_t *sym_arr, struct Time time){
	uint8_t hours = time.hours;
	uint8_t minutes = time.minutes;
	/* Save dot-point state */
	if(sym_arr[1] & 0x80){
		sym_arr[1] = segm_sym_table[hours%10] | 0x80;
	} else {	
		sym_arr[1] = segm_sym_table[hours%10];
	}
	sym_arr[3] = segm_sym_table[minutes%10];
	hours/=10;
	minutes/=10;
	sym_arr[0] = segm_sym_table[hours%10] ;
	sym_arr[2] = segm_sym_table[minutes%10];	
}


static uint8_t mode = 0;
static bool time_chng = true;

/* Interrupt service routine for buttons */
ISR(PCINT0_vect){
	if(btn_go_low(&mode_btn)){
		mode++;
		/* 6 states: 0-clock, 1-set clock mns,
			     2-set clock hrs, 3-alarm,
			     4-set alarm mns, 5-set alarm hrs. */
		mode%=6; 
	}
	switch (mode){
			case 1: if(btn_go_low(&inc_btn)){
					current_time.minutes++;
					current_time.minutes%=60;
					time_chng = true;
				}
				if(btn_go_low(&dec_btn)){
					if(--current_time.minutes == 255)
						current_time.minutes = 59;
					time_chng = true;
				}
				break;
			case 2: if(btn_go_low(&inc_btn)){
					current_time.hours++;
					current_time.hours%=24;
					time_chng = true;
				}
				if(btn_go_low(&dec_btn)){
					if(--current_time.hours == 255)
						current_time.hours = 23;
					time_chng = true;
				}
				break;
			case 4: if(btn_go_low(&inc_btn)){
					alarm_time.minutes++;
					alarm_time.minutes%=60;
					time_chng = true;
				}
				if(btn_go_low(&dec_btn)){
					if(--alarm_time.minutes == 255)
						alarm_time.minutes = 59;
					time_chng = true;
				}
				break;
			case 5: if(btn_go_low(&inc_btn)){
					alarm_time.hours++;
					current_time.hours%=24;
					time_chng = true;
				}
				if(btn_go_low(&dec_btn)){
					if(--alarm_time.hours == 255)
						alarm_time.hours = 23;
					time_chng = true;
				}
				break;
			default: break;

			}	
}

/* Static arrays for symbols of current and alarm time */
static uint8_t time_symbols[] = {0, 0, 0, 0};
static uint8_t alarm_symbols[] = {0, 0, 0, 0};

/* Interrupt service routine for time measurment (occurs every 1s)*/
ISR(TIMER1_COMPA_vect){
	current_time.seconds++;
	/* Dot-point blink*/
	time_symbols[1] ^= (1 << 7);
	if(current_time.seconds >= 60){
		current_time.seconds%=60;
		current_time.minutes++;
		if(current_time.minutes>=60){
			current_time.minutes%=60;
			current_time.hours++;
			if(current_time.hours>=24)
				current_time.hours%=24;
		}
	}
	time_chng = true;		
}

int main(void)
{
	init_io();
	
	while(1) {
		if(!time_chng){
			switch (mode){
			case 0: segm_indicate(&display, time_symbols, 0x0F);
				break;
			case 1: segm_indicate(&display, time_symbols, 0x03);
				break;
			case 2: segm_indicate(&display, time_symbols, 0x0C);
				break;
			case 3: segm_indicate(&display, alarm_symbols, 0x0F);
				break;
			case 4: segm_indicate(&display, alarm_symbols, 0x03);
				break;
			case 5: segm_indicate(&display, alarm_symbols, 0x0C);
				break;

			}
		} else {
			/* Alarm sounds for 1 minute if it occurs */
			if( (alarm_time.minutes == current_time.minutes)
			     & (alarm_time.hours == current_time.hours) )
			{
				*alarm_pin.port->PORT |= (1 << alarm_pin.pin);
			} else {
				*alarm_pin.port->PORT &= ~(1 << alarm_pin.pin);
			}				
			cast_time(alarm_symbols, alarm_time);
			cast_time(time_symbols, current_time);
			time_chng = false;
		}
		
	}
}