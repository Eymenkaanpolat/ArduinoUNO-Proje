#include "timer.h"

volatile uint32_t timern = 0;

void timer1_kesme_basla(uint16_t milisaniye) {
    if (milisaniye == 0 || milisaniye > 4000) {
        return; 
    }

    // Timer'ı durdur ve sıfırla
    TCCR1A = 0; 
    TCCR1B = 0; 
    TCNT1 = 0; 

    // CTC Modu
    TCCR1B |= (1 << WGM12);

    uint32_t hedef_tick = 0;

    // Kanka, Eğer süre 262 ms'den büyükse taşmayı önlemek için Prescaler'ı 1024 yapıyoruz
    if (milisaniye > 262) {
        TCCR1B |= (1 << CS12) | (1 << CS10); // Prescaler 1024 (16MHz / 1024 = 15625 Hz)
        // 1 ms = 15.625 tick
        hedef_tick = ((uint32_t)milisaniye * 15625UL) / 1000UL;
    } else {
        TCCR1B |= (1 << CS11) | (1 << CS10); // Prescaler 64 (16MHz / 64 = 250000 Hz)
        // 1 ms = 250 tick
        hedef_tick = ((uint32_t)milisaniye * 250UL);
    }

    OCR1A = (uint16_t)(hedef_tick - 1);
    TIMSK1 |= (1 << OCIE1A);
    sei();
}

void timer1_durdur(void) {
    TCCR1B = 0; 
    TIMSK1 &= ~(1 << OCIE1A);
}

