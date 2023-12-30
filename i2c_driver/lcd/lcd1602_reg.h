#ifndef _LCD1602_DEF_H_
#define _LCD1602_DEF_H_

typedef unsigned char   uint8_t;
uint8_t backlight_state = 1;
#define MAX_SIZE 255
#define I2C_ADDR 0x27 // I2C address of the PCF8574
#define RS_BIT 0 // Register select bit
#define EN_BIT 2 // Enable bit
#define BL_BIT 3 // Backlight bit
#define D4_BIT 4 // Data 4 bit
#define D5_BIT 5 // Data 5 bit
#define D6_BIT 6 // Data 6 bit
#define D7_BIT 7 // Data 7 bit

#define LCD_ROWS 2 // Number of rows on the LCD
#define LCD_COLS 16 // Number of columns on the LCD


#endif /* _LCD1602_DEF_H_ */
