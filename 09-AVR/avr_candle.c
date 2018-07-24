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
static uint16_t RANDOM_MIDDLE = RANDOM_MAX/2+1;

/* PWM static values */
static uint16_t PWM_T = 100; /* PWM period in us */
static uint16_t MIN_PWMT; /* minimum T_on for PWM*/
static uint16_t MAX_PWMT; /* maximum T_on for PWM*/

/* count of PWM_T times with const brightness */
static uint8_t frame_len = 50;

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
 * diturbution (50% times duty will be max (1.0), 50% - will 
 * be uniformely distubed between 0.3 - 0.9) 
 * @random - value, recieved from uniform generator
 */
uint16_t map_duty(uint16_t random){
	if(random > RANDOM_MAX/2){
		return PWM_T;
	} else {
		return (uint16_t) map(0, RANDOM_MIDDLE, MIN_PWMT,
						MAX_PWMT, random);
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

int main(void)
{
	/* Configure GPIO */
	DDRB |= 1 << 3;			/* set PB3 to output */
	PORTB |= 1 << 3;		/* set output to 1   */
	
	/* PWM duty variables */
	uint16_t t_on, t_off;
	MIN_PWMT = 3*PWM_T/10;
	MAX_PWMT = 9*PWM_T/10;

	uint8_t frame_cnt;
	while(1) {
		/* Get random duty values*/
		t_on = map_duty(my_rand());
		t_off = PWM_T - t_on;
		
		/* Software PWM with const brightness */
		for(frame_cnt = 0; frame_cnt<frame_len; frame_cnt++){
			PORTB |= 1 << 3;
			_delay_us(t_on);
			PORTB &= 0xF7;
			_delay_us(t_off);
		}
	}
	return 0;
}