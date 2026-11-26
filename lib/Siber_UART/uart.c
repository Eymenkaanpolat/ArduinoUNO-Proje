#include "uart.h"
#include <stdlib.h> 
#include <ctype.h>  

volatile uint8_t hesap_hata = 0;

float parse_expression(char **str);
float parse_term(char **str);
float parse_factor(char **str);

float parse_factor(char **str) {
    if (hesap_hata) return 0.0;
    
    while (**str == ' ') (*str)++; 
    
    if (**str == '(') {
        (*str)++; 
        float val = parse_expression(str);
        if (**str == ')') {
            (*str)++; 
        } else {
            hesap_hata = 1; 
        }
        return val;
    }
    
    if (**str == '-') { 
        (*str)++;
        return -parse_factor(str);
    }
    
    if (!isdigit(**str) && **str != '.') {
        hesap_hata = 2; 
        return 0.0;
    }

    float val = 0.0;
    uint8_t basamak = 0;
    uint8_t ondalik_var = 0;
    float ondalik_carpan = 0.1;
    
    while (isdigit(**str) || **str == '.') {
        if (**str == '.') {
            if (ondalik_var) { hesap_hata = 2; return 0.0; }
            ondalik_var = 1;
            (*str)++;
            continue;
        }
        
        // 🚨 KANKA, BASAMAK LİMİTİ 6'YA ÇEKİLDİ 🚨
        if (basamak >= 6) { 
            hesap_hata = 3; // Basamak limiti aşıldı hatası
            return 0.0;
        }

        if (!ondalik_var) {
            val = val * 10.0 + (**str - '0');
        } else {
            val = val + (**str - '0') * ondalik_carpan;
            ondalik_carpan /= 10.0;
        }
        (*str)++;
        basamak++;
    }
    return val;
}

float parse_term(char **str) {
    float val = parse_factor(str);
    while (**str == '*' || **str == '/') {
        char op = **str;
        (*str)++;
        float next_val = parse_factor(str);
        
        if (op == '*') {
            val *= next_val;
        } else if (op == '/') {
            if (next_val == 0.0) {
                hesap_hata = 4; 
                return 0.0;
            }
            val /= next_val; 
        }
    }
    return val;
}

float parse_expression(char **str) {
    float val = parse_term(str);
    while (**str == '+' || **str == '-') {
        char op = **str;
        (*str)++;
        float next_val = parse_term(str);
        
        if (op == '+') val += next_val;
        else if (op == '-') val -= next_val;
    }
    return val;
}

void _hesap_deger_yaz(long deger) {
    if (deger < 0) {
        uart_harfW('-'); 
        deger = -deger;  
    }
    uart_degerW((uint32_t)deger); 
}

void _hesap_float_yaz(float deger) {
    if (deger < 0) {
        uart_harfW('-');
        deger = -deger;
    }
    
    deger += 0.00001; 
    
    // 🚨 KANKA, TAŞMA KONTROLÜ BURADA 🚨
    // Eğer sonuç 2.1 milyarı geçerse long'a çevirince taşar, hata verip çıkıyoruz.
    if (deger > 2147483647.0) {
        uart_yaz("Hata: Sonuc cok buyuk (Tasma)!");
        return;
    }
    
    long tam_kisim = (long)deger;
    long ondalik_kisim = (long)((deger - (float)tam_kisim) * 100.0); 

    _hesap_deger_yaz(tam_kisim);
    
    if (ondalik_kisim > 0) {
        uart_harfW('.');
        if (ondalik_kisim < 10) uart_harfW('0'); 
        _hesap_deger_yaz(ondalik_kisim);
    }
}

void uart_calculator(void) {
    char tampon[30]; 
    uint8_t indeks = 0;
    char gelen;

    uart_yaz("\r\n=== HESAP MAKINESI (Limitli) ===\r\n");
    uart_yaz("Islem girin (Max 6 hane): ");

    while (1) {
        while (!(UCSR0A & (1 << RXC0))); 
        gelen = UDR0;

        if (gelen == '\n') continue; 

        if (gelen == '\r') {
            tampon[indeks] = '\0'; 
            
            if (indeks == 0) {
                uart_yaz("\r\nYeni islem: ");
                continue;
            }

            uart_yaz("\r\nGirilen: ");
            uart_yaz(tampon);
            
            uint8_t gecerli_mi = 1;
            uint8_t parantez_derinligi = 0;

            for (uint8_t i = 0; i < indeks; i++) {
                char c = tampon[i];
                if (!isdigit(c) && c != '+' && c != '-' && c != '*' && c != '/' && c != '(' && c != ')' && c != '.') {
                    gecerli_mi = 0; 
                    break;
                }
                if (c == '(') parantez_derinligi++;
                if (c == ')') {
                    if (parantez_derinligi == 0) { gecerli_mi = 0; break; }
                    parantez_derinligi--;
                }
            }

            if (parantez_derinligi != 0) gecerli_mi = 0; 

            if (!gecerli_mi) {
                uart_yaz("\r\nHata: Gecersiz karakter veya dengesiz parantez!\r\n");
                indeks = 0;
                uart_yaz("Yeni islem: ");
                continue;
            }

            hesap_hata = 0;
            char *p = tampon;
            float sonuc = parse_expression(&p); 
            
            while (*p == ' ') p++;
            if (*p != '\0' && !hesap_hata) hesap_hata = 2; 

            if (hesap_hata) {
                if (hesap_hata == 1) uart_yaz("\r\nHata: Parantez hatasi!\r\n");
                else if (hesap_hata == 2) uart_yaz("\r\nHata: Sozdizimi hatasi!\r\n");
                else if (hesap_hata == 3) uart_yaz("\r\nHata: Sayi girisi 6 basamaktan fazla olamaz!\r\n");
                else if (hesap_hata == 4) uart_yaz("\r\nHata: Sifira bolunemez!\r\n");
            } else {
                uart_yaz(" = ");
                _hesap_float_yaz(sonuc); 
                uart_yaz("\r\n");
            }

            indeks = 0;
            uart_yaz("\r\nYeni islem: ");
        } 
        else {
            if (gelen == 127 || gelen == 8) { 
                if (indeks > 0) {
                    indeks--;
                    uart_yaz("\b \b"); 
                }
            }
            else if (indeks < 29 && gelen >= 32) { 
                tampon[indeks++] = gelen;
                uart_harfW(gelen); 
            } 
        }
    }
}

void uart_basla(void)
{
    UBRR0H = (uint8_t)(BAUD_ORANI >> 8);
    UBRR0L = (uint8_t)BAUD_ORANI;

    // TX ve RX Aktif
    UCSR0B = (1 << TXEN0) | (1 << RXEN0);
    // 8-bit veri, 1 stop biti, parity yok
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}
void uart_hexW(uint32_t sayi)
{

    char hex[] = "0123456789ABCDEF";

    uart_yaz("0x");

    for (int8_t i = 7; i >= 0; i--)
    {
        uart_harfW(hex[(sayi >> (i * 4)) & 0x0F]);
    }
}

// Tek bir karakter gönderir
void uart_harfW(char harf)
{
    while (!(UCSR0A & (1 << UDRE0)))
        ;
    UDR0 = harf;
}

// Metin (string) gönderir
void uart_yaz(const char *yazi)
{
    while (*yazi)
    {
        uart_harfW(*yazi++);
    }
}

// 32-bit sayıyı desimal (10'luk taban) olarak ekrana basar
void uart_degerW(uint32_t sayi)
{
    char tampon[11];
    uint8_t i = 0;

    if (sayi == 0)
    {
        uart_harfW('0');
        return;
    }

    while (sayi > 0)
    {
        tampon[i++] = (sayi % 10) + '0';
        sayi /= 10;
    }

    while (i > 0)
    {
        uart_harfW(tampon[--i]);
    }
}

// Seri porttan TEK karakter bekler ve döner (Bloklayan okuma)
char uart_char_oku(void)
{
    while (!(UCSR0A & (1 << RXC0)))
        ;
    return UDR0;
}

// Hatalı veya uzun girdi durumunda satır sonuna (\r veya \n) kadar olan tamponu temizler
static void tamponu_temizle(void)
{
    char c;
    do
    {
        c = uart_char_oku();
    } while (c != '\r' && c != '\n');
}

// Cihaz harfi (A-Z) ve peşinden gelen maks 6 haneli sayıyı okur
uint32_t uart_harfR(char *cihaz)
{
    char c;
    uint32_t sonuc;
    uint8_t rakam_sayisi;

    while (1)
    {
        sonuc = 0;
        rakam_sayisi = 0;

        // --- 1. AŞAMA: Cihaz Harfini Okuma ---
        c = uart_char_oku();

        // Satır başı, satır sonu veya boşluk karakterlerini yut
        if (c == '\r' || c == '\n' || c == ' ')
            continue;

        // Küçük harfi büyük harfe çevir
        if (c >= 'a' && c <= 'z')
            c -= 32;

        // Eğer harf değilse hata ver ve satırı temizle
        if (!(c >= 'A' && c <= 'Z'))
        {
            uart_yaz("\r\nHata: Ilk karakter harf olmali!\r\n");
            tamponu_temizle();
            continue;
        }

        *cihaz = c;
        uart_harfW(c); // Okunan harfi ekrana geri bas (Echo)

        // --- 2. AŞAMA: Rakamları Okuma ---
        while (1)
        {
            c = uart_char_oku();

            // Enter (Satır Sonu) algılandı
            if (c == '\r' || c == '\n')
            {
                if (rakam_sayisi == 0)
                {
                    uart_yaz("\r\nHata: Sayi girilmedi!\r\n");
                    break; // Ana döngüye dön, yeniden harf bekle
                }

                // Temiz bir yeni satıra geç ve değeri tamamla
                uart_yaz("\r\n");
                return sonuc;
            }

            // Sayı kontrolü
            if (c >= '0' && c <= '9')
            {
                if (rakam_sayisi < 6)
                {
                    uart_harfW(c); // Rakamı ekrana geri bas (Echo)
                    sonuc = (sonuc * 10) + (c - '0');
                    rakam_sayisi++;
                }
                else
                {
                    uart_yaz("\r\nHata: En fazla 6 rakam girilebilir!\r\n");
                    tamponu_temizle();
                    break; // Ana döngüye dön
                }
            }
            else
            {
                uart_yaz("\r\nHata: Gecersiz karakter!\r\n");
                tamponu_temizle();
                break; // Ana döngüye dön
            }
        }
    }
}