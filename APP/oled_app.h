/* oled_app.h */
#ifndef __OLED_APP_H__
#define __OLED_APP_H__

#include "main.h" // 或者你的主要头文件 "mydefine.h"
#include  "stdarg.h" 
#include  "stdio.h "
#include "oled.h"   // 包含底层 OLED 驱动头文件
#include "mydefine.h"
int Oled_Printf(uint8_t x, uint8_t y, const char *format, ...);
void oled_task(void);
uint8_t u8x8_byte_hw_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8g2_gpio_and_delay_stm32(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);

void OLED_SendBuff(uint8_t buff[4][128]);
#endif

