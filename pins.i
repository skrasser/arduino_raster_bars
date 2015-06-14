#if defined (__AVR_ATmega328P__) || defined (__AVR_ATmega328__)
#define PORTB 0x05
#define PORTC 0x08
#define PORTD 0x0b
#else
#error Unknown MCU
#endif
