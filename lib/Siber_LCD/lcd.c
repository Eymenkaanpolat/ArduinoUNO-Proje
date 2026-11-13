
#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include "lcd.h"
#include <avr/io.h>
#include <util/delay.h>


// PCF8574T adresi
#define LCD_ADRES 0x27

// PCF8574T pin eşlemesi
#define LCD_RS  0x01
#define LCD_RW  0x02
#define LCD_EN  0x04
#define LCD_BL  0x08

// ---------------- I2C ----------------

static void i2c_basla(void)
{
    TWSR = 0x00;
    TWBR = 72;
    TWCR = (1 << TWEN);
}

static void i2c_start(void)
{
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);

    while (!(TWCR & (1 << TWINT)));
}

static void i2c_yaz(uint8_t veri)
{
    TWDR = veri;

    TWCR = (1 << TWINT) | (1 << TWEN);

    while (!(TWCR & (1 << TWINT)));
}

static void i2c_stop(void)
{
    TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
}

static void i2c_gonder(uint8_t veri)
{
    i2c_start();

    i2c_yaz(LCD_ADRES << 1);
    i2c_yaz(veri);

    i2c_stop();
}

// ---------------- PCF8574 ----------------

static void lcd_port(uint8_t veri)
{
    i2c_gonder(veri | LCD_BL);
}

static void lcd_enable(uint8_t veri)
{
    lcd_port(veri | LCD_EN);
    _delay_us(1);

    lcd_port(veri & ~LCD_EN);
    _delay_us(50);
}

// ---------------- LCD ----------------

static void lcd_nibble(uint8_t veri)
{
    lcd_enable(veri);
}

static void lcd_gonder(uint8_t veri, uint8_t rs)
{
    uint8_t kontrol = rs ? LCD_RS : 0;

    lcd_nibble((veri & 0xF0) | kontrol);

    lcd_nibble(((veri << 4) & 0xF0) | kontrol);
}

static void lcd_komut(uint8_t komut)
{
    lcd_gonder(komut, 0);

    if (komut == 0x01 || komut == 0x02)
        _delay_ms(2);
}

static void lcd_karakter(uint8_t karakter)
{
    lcd_gonder(karakter, LCD_RS);
}

// ---------------- KULLANICI FONKSİYONLARI ----------------

void lcd_basla(void)
{
    i2c_basla();

    _delay_ms(50);

    // 8-bit başlangıç sekansı
    lcd_nibble(0x30);
    _delay_ms(5);

    lcd_nibble(0x30);
    _delay_us(150);

    lcd_nibble(0x30);
    _delay_us(150);

    // 4-bit moda geçiş
    lcd_nibble(0x20);
    _delay_us(150);

    // LCD yapılandırması
    lcd_komut(0x28);  // 4-bit, 2 satır
    lcd_komut(0x0C);  // Ekran açık, imleç kapalı
    lcd_komut(0x06);  // Otomatik ilerleme
    lcd_komut(0x01);  // Ekranı temizle

    _delay_ms(2);
}

void lcd_temizle(void)
{
    lcd_komut(0x01);
}

void lcd_imlec(uint8_t satir, uint8_t sutun)
{
    if (sutun > 15)
        sutun = 15;

    uint8_t adres;

    if (satir == 0)
        adres = 0x80 + sutun;
    else
        adres = 0xC0 + sutun;

    lcd_komut(adres);
}

void lcd_yaz(uint8_t satir, const char *metin)
{
    lcd_imlec(satir, 0);

    for (uint8_t i = 0; i < 16; i++)
        lcd_karakter(' ');

    lcd_imlec(satir, 0);

    uint8_t sayac = 0;

    while (*metin && sayac < 16)
    {
        lcd_karakter(*metin++);
        sayac++;
    }
}

void lcd_degerW(uint8_t satir, uint32_t deger)
{
    char tampon[10];
    uint8_t i = 0;

    lcd_imlec(satir, 0);

    for (uint8_t j = 0; j < 16; j++)
        lcd_karakter(' ');

    lcd_imlec(satir, 0);

    if (deger == 0)
    {
        lcd_karakter('0');
        return;
    }

    while (deger > 0)
    {
        tampon[i++] = (deger % 10) + '0';
        deger /= 10;
    }

    while (i > 0)
        lcd_karakter(tampon[--i]);
}
