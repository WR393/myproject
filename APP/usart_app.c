#include "usart_app.h"

uint32_t uart_rx_ticks = 0;
uint16_t uart_rx_index = 0;
uint8_t uart_flag = 0;
uint8_t uart_rx_buffer[128] = {0};

uint8_t uart_rx_dma_buffer[128] = {0};
uint8_t uart_dma_buffer[128] = {0};
struct rt_ringbuffer uart_ringbuffer;
uint8_t ringbuffer_pool[128];

struct rt_ringbuffer Web_ringbuffer;
uint8_t Web_ringbuffer_pool[128];
uint8_t Web_rx_dma_buffer[128] = {0};
uint8_t Web_dma_buffer[128] = {0};

int my_printf(UART_HandleTypeDef *huart, const char *format, ...)
{
	char buffer[512];
	va_list arg;
	int len;
	// 初始化可变参数列表
	va_start(arg, format);
	len = vsnprintf(buffer, sizeof(buffer), format, arg);
	va_end(arg);
	HAL_UART_Transmit(huart, (uint8_t *)buffer, (uint16_t)len, 0xFF);
	return len;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART1)
	{
		uart_rx_ticks = uwTick;
		uart_rx_index++;
		HAL_UART_Receive_IT(&huart1, &uart_rx_buffer[uart_rx_index], 1);
	}
}

/**
 * @brief UART DMA接收完成或空闲事件回调函数
 * @param huart UART句柄
 * @param Size 指示在事件发生前，DMA已经成功接收了多少字节的数据
 * @retval None
 */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    // 1. 确认是目标串口 (USART1)
    if (huart->Instance == USART1)
    {
        // 2. 紧急停止当前的 DMA 传输 (如果还在进行中)
        //    因为空闲中断意味着发送方已经停止，防止 DMA 继续等待或出错
        HAL_UART_DMAStop(huart);

       rt_ringbuffer_put(&uart_ringbuffer,uart_rx_dma_buffer,Size);

        // 5. 清空 DMA 接收缓冲区，为下次接收做准备
        //    虽然 memcpy 只复制了 Size 个，但清空整个缓冲区更保险
        memset(uart_rx_dma_buffer, 0, sizeof(uart_rx_dma_buffer));

        // 6. **关键：重新启动下一次 DMA 空闲接收**
        //    必须再次调用，否则只会接收这一次
        HAL_UARTEx_ReceiveToIdle_DMA(&huart1, uart_rx_dma_buffer, sizeof(uart_rx_dma_buffer));
        
        // 7. 如果之前关闭了半满中断，可能需要在这里再次关闭 (根据需要)
        __HAL_DMA_DISABLE_IT(&hdma_usart1_rx, DMA_IT_HT);
    }
		
		 if (huart->Instance == USART6)
    {
        // 2. 紧急停止当前的 DMA 传输 (如果还在进行中)
        //    因为空闲中断意味着发送方已经停止，防止 DMA 继续等待或出错
        HAL_UART_DMAStop(huart);

       rt_ringbuffer_put(&Web_ringbuffer,Web_rx_dma_buffer,Size);

        // 5. 清空 DMA 接收缓冲区，为下次接收做准备
        //    虽然 memcpy 只复制了 Size 个，但清空整个缓冲区更保险
        memset(Web_rx_dma_buffer, 0, sizeof(Web_rx_dma_buffer));

        // 6. **关键：重新启动下一次 DMA 空闲接收**
        //    必须再次调用，否则只会接收这一次
        HAL_UARTEx_ReceiveToIdle_DMA(&huart6, Web_rx_dma_buffer, sizeof(Web_rx_dma_buffer));
        
        // 7. 如果之前关闭了半满中断，可能需要在这里再次关闭 (根据需要)
        __HAL_DMA_DISABLE_IT(&hdma_usart6_rx, DMA_IT_HT);
    }
}

void uart_task(void)
{
	// 如果接收索引为0，说明没有数据需要处理，直接返回
	

	uint16_t length;
	length = rt_ringbuffer_data_len(&uart_ringbuffer);
	if(length==0)return;
	
	rt_ringbuffer_get(&uart_ringbuffer,uart_dma_buffer,length);
		my_printf(&huart1, "uart data: %s\n", uart_dma_buffer);

		// 清空接收缓冲区，将接收索引置零
		memset(uart_dma_buffer, 0, sizeof(uart_dma_buffer));
	
	huart1.pRxBuffPtr=uart_rx_buffer;
}

void Web_task(void)
{
	// 如果接收索引为0，说明没有数据需要处理，直接返回
	

	uint16_t length;
	length = rt_ringbuffer_data_len(&Web_ringbuffer);
	if(length==0)return;
	
	rt_ringbuffer_get(&Web_ringbuffer,Web_dma_buffer,length);
		my_printf(&huart1, "uart6 data: %s\n", Web_dma_buffer);

		// 清空接收缓冲区，将接收索引置零
		memset(Web_dma_buffer, 0, sizeof(Web_dma_buffer));
	
	huart6.pRxBuffPtr=uart_rx_buffer;
}
