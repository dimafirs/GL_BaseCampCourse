#define 	F_CPU   16000000UL

#include "hal.h"
#include "onewire.h"

#include <avr/io.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdbool.h>


/* Some HALs for ports and peripherals */
static struct hal_Port PD = {
	.DDR = &DDRD,
	.PIN = &PIND,
	.PORT = &PORTD,
};

static struct hal_Port PB = {
	.DDR = &DDRB,
	.PIN = &PINB,
	.PORT = &PORTB,
};

static struct hal_Pin   mode_btn = {.port = &PD, .pin = 2},
			ow_pin = {.port = &PB, .pin = 0},
			gled_pin = {.port = &PD, .pin = 5},
			rled_pin = {.port = &PD, .pin = 6};

/* Core functions for One Wire line */
void line_low(void)
{
	*(ow_pin.port->PORT) &= ~(1 << ow_pin.pin);
	*(ow_pin.port->DDR) |= (1 << ow_pin.pin);
}

void line_release(void)
{
	*(ow_pin.port->DDR) &= ~(1 << ow_pin.pin);
	*(ow_pin.port->PORT) |= (1 << ow_pin.pin);
}

bool line_read(void)
{
	uint8_t val = *(ow_pin.port->PIN) & (1 << ow_pin.pin);
	return val;
}

/* Software delay for OneWire interface */
void soft_delay_us(uint16_t wait_us){
	while(wait_us--){
		_delay_us(1);
	}
}

/* bool btn_go_low() - return true if button in parameters is pressed
 @btn - HAL struct of button pin 
 Author: Timofei Hodnev (@thodnev) */ 
static bool btn_go_low(struct hal_Pin *btn)
{
	if (!(*(btn->port->PIN) & (1 << btn->pin))) {
		_delay_ms(10);
		if (!(*(btn->port->PIN) & (1 << btn->pin)))
			return true;
	}
	return false;
}

/* 2 modes: 0-normal, 1-key programming */
static bool mode = false;

/* Number of key bytes that will store in EEPROM */
static uint8_t store_len = 7;

/* Interrupt service routine for buttons */
ISR(INT0_vect){
	if(btn_go_low(&mode_btn))
		mode=!mode;
}

/* void init_io() - initialize MCU peripherial. */ 
static void init_io(void){
	/* We use internal pullup resitor for 1-wire line */
	*(ow_pin.port->DDR) &= ~(1 << ow_pin.pin);
	*(ow_pin.port->PORT) |= (1 << ow_pin.pin);

	/* Configure LED outputs */ 
	*(gled_pin.port->DDR) |= (1 << gled_pin.pin);
	*(rled_pin.port->DDR) |= (1 << rled_pin.pin);
	*(gled_pin.port->PORT) &= ~(1 << gled_pin.pin);
	*(rled_pin.port->PORT) &= ~(1 << rled_pin.pin);
	
	/* Setup mode button pin as input */
	*(mode_btn.port->DDR) &= ~(1 << mode_btn.pin);
	
	/* Enable global interupts */
	sei();
	
	/* Enable INT0 for mode button (interrupt on falling edge)*/
	EICRA |= (1 << ISC01);
	EICRA &= ~(1 << ISC00);
	EIMSK |= (1 << INT0);
}

/* bool key_match() - find and compare @id with all keys in EEPROM. Return true if
 * 		      key @id is matched with any already added key.
 * @id		- pointer to unique id.
 * @key_cnt	- number of already added keys.
 */
static bool key_match(const uint8_t *id, uint8_t key_cnt){
	if(key_cnt<=0)	return false;
	uint8_t eeprom_id[store_len];
	for(uint8_t i=0; i<key_cnt; i++){
		uint16_t cur_addr = i*store_len;
		eeprom_read_block((void *)eeprom_id, (void *)cur_addr,store_len);
		bool result = true;
		for(uint8_t j=0; j<store_len; j++){
			if(eeprom_id[j] != id[j]){
				result = false;
				break;
			}
		}
		if(result) 
			return true;
	}
	return false;			 
}

/* void key_add() - add @id key to first free place in eeprom and inc number of keys.
 * @id		- pointer to unique key. 
 * @key_cnt	- pointer to number of already added keys.
 */
static void key_add(const uint8_t *id, uint8_t *key_cnt){
	uint16_t addr = *key_cnt * store_len;
	eeprom_write_block((const void *)id, (void *) addr, store_len);
	(*key_cnt)++;
}

int main()
{
	init_io();
	ow_Pin pin;
	ow_Pin_init(&pin, &line_low, &line_release, &line_read, &soft_delay_us, 5, 60, 60, 5);
	ow_err err;

	/* Use this line one you want to delete all keys */
	//eeprom_write_byte((uint8_t *) 1023,(uint8_t) 0);
	uint8_t key_number = eeprom_read_byte((uint8_t *) 1023);
	uint8_t ibutton_id[8];
	uint8_t crc;
	while (1) {
		if(!mode){
			err = ow_cmd_readrom(&pin, ibutton_id, &crc, true, false);
			/* If compare matched with EEPROM value - long flash with green
			 * LED, key is unknown - long flash with red LED, default - short
			 * red LED flashes.						*/ 			
			if(err == OW_EOK){
				if(key_match(ibutton_id, key_number)){
					*(gled_pin.port->PORT) |= 1 << gled_pin.pin;
					_delay_ms(1500);
					*(gled_pin.port->PORT) &= ~(1 << gled_pin.pin);
				} else {
					*(rled_pin.port->PORT) |= 1 << rled_pin.pin;
					_delay_ms(1500);
					*(rled_pin.port->PORT) &= ~(1 << rled_pin.pin);
				}
			} else {
				*(rled_pin.port->PORT) |= 1 << rled_pin.pin;
				_delay_ms(100);
				*(rled_pin.port->PORT) &= ~(1 << rled_pin.pin);
				_delay_ms(100);
			}
				
		} else {
			err = ow_cmd_readrom(&pin, ibutton_id, &crc, true, false);
			if(err == OW_EOK){
				/* If someone wants to add a new key add and blink with both
				   LEDs, else - just blink red.				  */
				if(!key_match(ibutton_id, key_number)){
					key_add(ibutton_id, &key_number);
					eeprom_write_byte((uint8_t *) 1023, key_number);
					*(gled_pin.port->PORT) |= 1 << gled_pin.pin;
					*(rled_pin.port->PORT) |= 1 << rled_pin.pin;
					_delay_ms(1000);
					*(gled_pin.port->PORT) &= ~(1 << gled_pin.pin);
					*(rled_pin.port->PORT) &= ~(1 << rled_pin.pin);
				} else {
					*(rled_pin.port->PORT) |= 1 << rled_pin.pin;
					_delay_ms(500);
					*(rled_pin.port->PORT) &= ~(1 << rled_pin.pin);
				}
			}
		}
	}
}