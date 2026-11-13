
#ifndef LCD_H
#define LCD_H

#include <stdint.h>

void lcd_basla(void);
void lcd_temizle(void);
void lcd_imlec(uint8_t satir, uint8_t sutun);
void lcd_yaz(uint8_t satir, const char *metin);
void lcd_degerW(uint8_t satir, uint32_t deger);

#endif
