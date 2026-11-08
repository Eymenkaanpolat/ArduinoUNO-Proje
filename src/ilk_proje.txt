#include "gpio.h"
#include "uart.h"
#include "ir.h"
#include "servo.h"
#include <util/delay.h>

int main(void)
{
    uart_basla();

    // Pin Ayarları
    pin_mod('D', 4, 0);
    pin_mod('D', 3, 0);
    pin_mod('B', 0, 1); // LED (D8 / PB0)
    pin_mod('D', 5, 1); // Servo (D9 / PB1)
    // Sensörleri ve Servoyu Başlat
    ir_basla();
    pinW('D', 5, 1);
    _delay_ms(100);
    pinW('D', 5, 0); // Servo başlangıçta 0 derece konumunda
    pinW('B', 0, 1);
    servo_basla('B', 1); // Servo (D9 / PB1)

    int16_t i = 90; // Başlangıç 90 derece (orta)
    uint8_t eski_i = i;
    volatile uint8_t e = 0;

    while (1)
    {
        if (ir_veri_var())
        {
            uint32_t kod = ir_oku();

            if (kod == 0xA55AFF00)
            { // İleri tuşu
                if (i <= 165)
                { // 180'e 15 kalana kadar artır
                    i += 15;
                }
                else
                {
                    i = 180; // Sınırı aşarsa 180'de sabitle
                }
            }
            else if (kod == 0xF708FF00)
            { // Geri tuşu
                if (i >= 15)
                { // 0'a 15 kalana kadar azalt
                    i -= 15;
                }
                else
                {
                    i = 0; // Sınırı aşarsa 0'da sabitle
                }
            }
            else if (kod == 0xE31CFF00)
            {
                e = !e;
            }

            ir_temizle(); // IR verisini temizle
        }

        // Servoyu SADECE değer değiştiğinde hareket ettir
        if (i != eski_i)
        {
            servo_yaz('B', 1, i);
            eski_i = i;

            uart_degerW(i); // Ekrana açıyı yazdır
            uart_yaz("\r\n");

            _delay_ms(100); // Servonun 15 derecelik hareketi tamamlaması için kısa bir bekleme
        }

        _delay_ms(10);
        uint8_t pir_sinyal = pinR('D', 4); // PIR sensör sinyalini oku
        
        uart_yaz("\r\n");
        if (e == 0)
        {
            if (pir_sinyal == 1)
            {
                pinW('D', 5, 1);
                uart_yaz("PIR Sinyal: ");
                uart_degerW(pir_sinyal);
                uart_yaz("\r\n");
            }
            else
            {
                pinW('D', 5, 0);
                uart_yaz("PIR Sinyal: ");
                uart_degerW(pir_sinyal);
                uart_yaz("\r\n");
            }
        }
    }
}