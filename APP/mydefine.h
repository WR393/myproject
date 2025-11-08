#include "stdio.h"
#include "string.h"
#include "stdarg.h"




#include "main.h"

#include "scheduler.h"
#include "adc_app.h"
#include "led_app.h"
#include "usart.h"
#include "key_app.h"
#include "btn_app.h"
#include "ebtn.h" 
#include "usart_app.h"
#include "ringbuffer.h"
#include "oled_app.h"
#include "oled.h"
#include "u8g2.h"
#include "WouoUI.h"      // WouoUI 核心框架
#include "WouoUI_user.h" // 用户自定义的菜单结构和回调函数 (通常需要用户创建或修改)
#include "i2c.h"  // 确保包含 I2C 句柄定义 (例如 hi2c1)
#include "gd25qxx.h"
#include "flash_app.h"
#include "lfs_port.h"
#include "lfs.h"

extern uint16_t uart_rx_index;
extern uint32_t uart_rx_ticks;
extern uint8_t uart_rx_buffer[128];
extern uint8_t uart_rx_dma_buffer[128];
extern UART_HandleTypeDef huart1;
extern DMA_HandleTypeDef hdma_usart1_rx;

extern UART_HandleTypeDef huart6;
extern DMA_HandleTypeDef hdma_usart6_rx;
extern struct rt_ringbuffer uart_ringbuffer;
extern uint8_t ringbuffer_pool[128];


extern struct rt_ringbuffer Web_ringbuffer;
extern uint8_t Web_ringbuffer_pool[128];
extern uint8_t Web_rx_dma_buffer[128];
extern uint8_t Web_dma_buffer[128];


extern const float frequency_steps[] ;
extern volatile uint8_t current_freq_index;
extern uint8_t mode;
extern uint8_t ucLed[6];

extern u8g2_t u8g2;

extern lfs_t lfs;
extern struct lfs_config cfg;







