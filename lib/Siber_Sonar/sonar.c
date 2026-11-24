#include "sonar.h"
#include <avr/interrupt.h>
#include <util/delay.h>

volatile uint16_t yukselme_zamani = 0;
volatile uint16_t dusme_zamani = 0;
volatile uint16_t havada_gecen_sure = 0;
volatile uint8_t olcum_bitti_mi = 0;
volatile uint8_t olcum_hatali_mi = 0; // YENİ: Hata bayrağı kanka

void sonar_basla(void)
{
    DDRD |= (1 << PD4);
    DDRD &= ~(1 << PD2);

    // Timer1 Ayarı: Prescaler = 8 (0.5us)
    TCCR1B |= (1 << CS11);

    // INT0: Herhangi bir mantıksal değişimde tetiklensin
    EICRA |= (1 << ISC00);
    EICRA &= ~(1 << ISC01);

    EIMSK |= (1 << INT0);
    sei();
}

void sonar_tetikle(void)
{
    olcum_bitti_mi = 0;
    olcum_hatali_mi = 0; // Yeni ölçüm için hatayı sıfırla

    // Kanka, Timer'ı sıfırlıyoruz ki her ölçüm temiz başlasın
    TCNT1 = 0;
    // Timer taşma bayrağını temizle
    TIFR1 |= (1 << TOV1);

    PORTD |= (1 << PD4);
    _delay_us(10);
    PORTD &= ~(1 << PD4);
}

ISR(INT0_vect)
{
    if (PIND & (1 << PD2))
    {
        // ECHO HIGH oldu: Yankı başladı
        yukselme_zamani = TCNT1;
    }
    else
    {
        // ECHO LOW oldu: Yankı bitti
        dusme_zamani = TCNT1;

        // Kanka, İŞTE KRİTİK KONTROL:
        // Eğer ölçüm sırasında Timer taştıysa (TOV1 = 1 ise),
        // bu sensörün yankı alamadığı anlamına gelir (38ms bekleme).
        if (TIFR1 & (1 << TOV1))
        {
            olcum_hatali_mi = 1;   // Hata bayrağını kaldır
            TIFR1 |= (1 << TOV1);  // Bayrağı temizle
            havada_gecen_sure = 0; // Süreyi sıfırla
        }
        else
        {
            // Taşma olmadıysa süreyi hesapla
            havada_gecen_sure = dusme_zamani - yukselme_zamani;
        }

        olcum_bitti_mi = 1;
    }
}

uint16_t sonar_mesafe_oku(void)
{
    // Kanka, eğer ölçüm hatalıysa (yankı alınamadıysa),
    // 0 döndürmek yerine "geçersiz" olduğunu belirten bir değer döndürelim.
    // 0 döndürürsek sensör "önümde bir şey var" sanabiliriz.
    if (olcum_hatali_mi)
    {
        return 999; // Örneğin 999 cm (geçersiz) döndür
    }

    uint32_t mikrosaniye = havada_gecen_sure / 2;
    uint16_t cm = mikrosaniye / 58;

    return cm;
}