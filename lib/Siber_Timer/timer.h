#ifndef TIMER_H_
#define TIMER_H_

#include <avr/io.h>
#include <avr/interrupt.h>

// Zamanlayıcı kesmesini milisaniye (ms) bazında ateşler kanka (Maksimum 4000 ms)
void timer1_kesme_basla(uint16_t milisaniye);

// Zamanlayıcıyı tamamen durdurur
void timer1_durdur(void);
extern volatile uint32_t timern;
#endif /* TIMER_H_ */