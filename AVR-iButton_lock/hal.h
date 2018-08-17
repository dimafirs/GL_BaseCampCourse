#include <stdint.h>

struct hal_Port {
	volatile uint8_t *DDR;	/* addr of GPIO DDRx direction register */
	volatile uint8_t *PIN;	/* addr of GPIO PINx input register */
	volatile uint8_t *PORT;	/* addr of GPIO PORTx data register */
};

struct hal_Pin {
	struct hal_Port *port;	/* GPIO port */
	uint8_t pin;		/* number of pin in GPIO port */
};
