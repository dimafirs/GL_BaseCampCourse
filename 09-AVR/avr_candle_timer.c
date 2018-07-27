/* Define MCU frequency */
#define F_CPU 16000000UL

/* Define option to get non-constant delay in _delay_us() */
#define __DELAY_BACKWARD_COMPATIBLE__

/* Define maximum return value of generator*/
#define RANDOM_MAX 16384

#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>

/* Generator static variables */ 
static uint16_t next = 413;
static uint16_t A = 505, C = 237;
static uint16_t RANDOM_MIDDLE = RANDOM_MAX/3;

/* PWM static values */
static uint16_t MAX_PWM_DUTY = 255; /* max PWM duty */
static uint16_t MIN_PWMD; /* minimum duty for distribution*/
static uint16_t MAX_PWMD; /* maximum duty for distribution*/

/* Time in us with const PWM duty (brightness) */
static uint8_t frame_time = 500;

/* map() - returns value, mapped to some range.
 * @in_min - minimum value of input range.
 * @in_max - maximum value of input range.
 * @out_min - minimum value of output range.
 * @out_max - maximum value of output range.
 * @input - input value, that need to be mapped.
 */
uint16_t map(uint16_t in_min, uint16_t in_max, 
	     uint16_t out_min, uint16_t out_max, uint16_t input)
{
	return (uint16_t) input*(out_max - out_min)/(in_max-in_min);
}

/* map_duty() - converts value from uniform to some custom 
 * diturbution (RANDOM_MIDDLE/RANDOM_MAX) * 100% times duty will 
 * be max (1.0), all other - will be uniformely distubed between 
 * 0.3 - 0.9 .
 * @random - value, recieved from uniform generator
 */
uint16_t map_duty(uint16_t random){
	if(random > RANDOM_MIDDLE){
		return MAX_PWM_DUTY;
	} else {
		return (uint16_t) map(0, RANDOM_MIDDLE, MIN_PWMD,
						MAX_PWMD, random);
	}
}

/* my_rand() - return psedorandom value between 0 and RANDOM_MAX.
 * All constants assigned like static variables at the top of 
 * file.
 */
uint16_t my_rand(){
	next = (next*A + C);
	return (uint16_t) next%(RANDOM_MAX + 1);
}

/* my_srand() - allow to init the start value of pseudorandom
 * generator (X0).
 * @seed - start value, that will init the generator
 */
void my_srand(uint16_t seed){
	next = seed;
}

void init_io(){
	/* Configure GPIO */
	DDRB |= 1 << 3;			/* set PB3 to output */
	PORTB |= 1 << 3;		/* set output to 1   */

	/* Configure timer */
	OCR2A = 255;			/*set duty on 100% */
	TCCR2A |= (1 << 7);		/* set non-inverting PWM mode*/
	TCCR2A |= 3;			/* set Fast PWM */
	TCCR2B |= 2; 			/* set prescalar 128 */ 
}

void set_pwm(uint8_t duty){
	OCR2A = duty;
}



int main(void)
{
	/* Configurate timer and GPIO */
	init_io();

	/* PWM duty variables */
	uint8_t pwm_duty;
	MIN_PWMD = 3*MAX_PWM_DUTY/10;
	MAX_PWMD = 9*MAX_PWM_DUTY/10;

	while(1) {
		/* Get random duty values*/
		pwm_duty = map_duty(my_rand());
		
		/* Configure PWM duty with recivied value */
		set_pwm(pwm_duty);
		
		/* Start const PWM frame after configuration */
		_delay_us(frame_time);
	}
	return 0;
}