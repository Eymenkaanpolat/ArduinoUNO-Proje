#include "timer.h"

// Kesme fonksiyonunda kullanılacak olan fonksiyon göstericisi (callback)
// Eğer Timer dışında (örneğin main içinde) bir şeyleri tetiklemek istersen burayı kullanabilirsin.
// Şimdilik sadece sayacı artıracağız.
volatile uint32_t timer_sayaci = 0;

void timer1_kesme_basla(uint16_t milisaniye) {
    // 1. Maksimum 4000 ms sınırını kontrol et
    if (milisaniye == 0 || milisaniye > 4000) {
        return; // Geçersiz değer, çık.
    }

    // 2. Timer'ı durdur (Durdurup ayar yapmak daha güvenlidir)
    TCCR1A = 0; 
    TCCR1B = 0; 
    TCNT1 = 0; // Sayacı sıfırla

    // 3. CTC Modunu ayarla (WGM12 biti)
    // Bu modda timer, OCR1A'daki değere ulaşınca sıfırlanır.
    TCCR1B |= (1 << WGM12);

    // 4. Prescaler ayarla: 64
    // 16MHz / 64 = 250,000 Hz (1 saniyedeki tick sayısı)
    // 1 ms = 250 tick
    TCCR1B |= (1 << CS11) | (1 << CS10);

    // 5. Hedef değeri hesapla ve OCR1A'ya yaz
    // (1 ms = 250 tick) olduğu için: milisaniye * 250
    // -1 yapıyoruz çünkü sayıcı 0'dan başlıyor (0-249 = 250 tick eder).
    uint32_t hedef_tick = ((uint32_t)milisaniye * 250UL) - 1;
    OCR1A = (uint16_t)hedef_tick;

    // 6. Kesmeyi etkinleştir
    // TIMSK1 register'ının OCIE1A bitini 1 yapıyoruz (Output Compare A Match Interrupt Enable)
    TIMSK1 |= (1 << OCIE1A);

    // 7. Global kesmeleri aç
    sei();
}

void timer1_durdur(void) {
    // Timer'ı tamamen durdur
    TCCR1B = 0; 
    
    // Kesmeyi kapat
    TIMSK1 &= ~(1 << OCIE1A);
}

// Timer1 Compare Match A Kesme Vektörü (ISR)
// Bu fonksiyon, her OCR1A değerine ulaşıldığında otomatik olarak çalışır.
ISR(TIMER1_COMPA_vect) {
    // Kanka, buraya kesme her tetiklendiğinde çalışmasını istediğin kodu yazabilirsin.
    // Örneğin: Bir LED'i yakıp söndürmek, sensör okumak veya bir değişkeni artırmak.
    timer_sayaci++; 
    
    // Örnek: Her kesme çalıştığında 13. pindeki LED'in durumunu değiştir (Toggle)
    // PORTB ^= (1 << 5); 
}