#if defined (__AVR_ATmega328P__) || defined (__AVR_ATmega328__)
#define PORTB 0x05	// H-Sync and red background channel
#define PORTC 0x08	// V-Sync and green background channel
#define PORTD 0x0b	// Primary color and background flag
#else
#error Unknown MCU
#endif
