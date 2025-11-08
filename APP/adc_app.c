#include "adc_app.h"
#include "adc.h"
#include "math.h"
#include "dac.h"
#include "tim.h"
#include "usart_app.h"
#include "string.h"
#include "usart.h"


// --- 全局变量 --- 
#define WAVE_SAMPLES 100    // 一个周期内的采样点数
#define DAC_MAX_VALUE 4095 // 12 位 DAC 的最大数字值 (2^12 - 1)

uint16_t SineWave[WAVE_SAMPLES]; // 存储正弦波数据的数组
uint16_t triangle_buffer[WAVE_SAMPLES];
uint16_t square_buffer[WAVE_SAMPLES];

// 频率列表 (单位: Hz)
const float frequency_steps[] = {50.0f, 100.0f, 150.0f, 200.0f, 250.0f};
const uint8_t num_frequency_steps = sizeof(frequency_steps) / sizeof(frequency_steps[0]);

// 当前频率档位的索引
volatile uint8_t current_freq_index = 0; // volatile很重要，因为中断会修改它
// --- 生成正弦波数据的函数 ---
/**
 * @brief 生成正弦波查找表
 * @param buffer: 存储波形数据的缓冲区指针
 * @param samples: 一个周期内的采样点数
 * @param amplitude: 正弦波的峰值幅度 (相对于中心值)
 * @param phase_shift: 相位偏移 (弧度)
 * @retval None
 */
void Generate_Sine_Wave(uint16_t* buffer, uint32_t samples, uint16_t amplitude, float phase_shift)
{
  // 计算每个采样点之间的角度步进 (2*PI / samples)
  float step = 2.0f * 3.14159f / samples; 
  
  for(uint32_t i = 0; i < samples; i++)
  {
    // 计算当前点的正弦值 (-1.0 到 1.0)
    float sine_value = sinf(i * step + phase_shift); // 使用 sinf 提高效率

    // 将正弦值映射到 DAC 的输出范围 (0 - 4095)
    // 1. 将 (-1.0 ~ 1.0) 映射到 (-amplitude ~ +amplitude)
    // 2. 加上中心值 (DAC_MAX_VALUE / 2)，将范围平移到 (Center-amp ~ Center+amp)
    buffer[i] = (uint16_t)((sine_value * amplitude) + (DAC_MAX_VALUE / 2.0f));
    
    // 确保值在有效范围内 (钳位)
    if (buffer[i] > DAC_MAX_VALUE) buffer[i] = DAC_MAX_VALUE;
    // 由于浮点计算精度问题，理论上不需要检查下限，但加上更健壮
    // else if (buffer[i] < 0) buffer[i] = 0; 
  }
}

/**
 * @brief 生成三角波查找表
 * @param buffer: 存储波形数据的缓冲区指针
 * @param samples: 一个周期内的采样点数
 * @param amplitude: 波形的峰值幅度 (相对于中心值)
 * @param phase_shift: 相位偏移 (弧度, 0 到 2*PI)
 * @retval None
 */
void Generate_Triangle_Wave(uint16_t* buffer, uint32_t samples, uint16_t amplitude, float phase_shift)
{
  uint32_t half_samples = samples / 2;
  float normalized_value = 0.0f;

  // 根据相位偏移计算起始采样点，实现波形左右平移
  // fmodf 用于取浮点数的模，确保相位在 0-2PI 范围内
  uint32_t start_offset = (uint32_t)((fmodf(phase_shift, 2.0f * 3.14159f) / (2.0f * 3.14159f)) * samples);

  for(uint32_t i = 0; i < samples; i++)
  {
    // 计算考虑了相位偏移的当前位置
    uint32_t current_pos = (i + start_offset) % samples;

    // --- 第一步：生成标准化的值 (-1.0 to 1.0) ---
    if(current_pos < half_samples)
    {
      // 前半周期：线性上升
      // 从 -1.0 开始，用 (current_pos / half_samples) 这个比例 * 2.0 来增加
      normalized_value = -1.0f + ( (float)current_pos / (float)half_samples ) * 2.0f;
    }
    else
    {
      // 后半周期：线性下降
      // 从 1.0 开始，用 ((current_pos - half_samples) / half_samples) 这个比例 * 2.0 来减小
      normalized_value = 1.0f - ( (float)(current_pos - half_samples) / (float)half_samples ) * 2.0f;
    }

    // --- 第二步：将标准化值映射到 DAC 范围 ---
    buffer[i] = (uint16_t)((normalized_value * amplitude) + (DAC_MAX_VALUE / 2.0f));
    
    // 钳位，确保值在有效范围内
    if (buffer[i] > DAC_MAX_VALUE) buffer[i] = DAC_MAX_VALUE;
  }
}

/**
 * @brief 生成方波查找表
 * @param buffer: 存储波形数据的缓冲区指针
 * @param samples: 一个周期内的采样点数
 * @param amplitude: 波形的峰值幅度 (相对于中心值)
 * @param duty_cycle: 占空比 (0.0 到 1.0)
 * @retval None
 * @note   这里用占空比代替了相位参数，因为它对方波更有意义。
 */
void Generate_Square_Wave(uint16_t* buffer, uint32_t samples, uint16_t amplitude, float duty_cycle)
{
  float normalized_value = 0.0f;

  // 确保占空比在 0.0 和 1.0 之间
  if(duty_cycle < 0.0f) duty_cycle = 0.0f;
  if(duty_cycle > 1.0f) duty_cycle = 1.0f;
  
  // 计算高电平应该持续的采样点数
  uint32_t high_duration_samples = (uint32_t)(samples * duty_cycle);

  for(uint32_t i = 0; i < samples; i++)
  {
    // --- 第一步：生成标准化的值 (-1.0 或 1.0) ---
    if(i < high_duration_samples)
    {
      // 高电平部分
      normalized_value = 1.0f;
    }
    else
    {
      // 低电平部分
      normalized_value = -1.0f;
    }

    // --- 第二步：将标准化值映射到 DAC 范围 ---
    // 注意：这里的 amplitude 决定了方波的峰峰值。
    // 例如，如果 amplitude 是 1000，中心值是 2048，则输出在 1048 和 3048 之间跳变。
    buffer[i] = (uint16_t)((normalized_value * amplitude) + (DAC_MAX_VALUE / 2.0f));
    
    // 理论上，由于输入只有-1和1，只要 amplitude <= 2047，就不需要钳位。
    // 但保留是个好习惯。
    if (buffer[i] > DAC_MAX_VALUE) buffer[i] = DAC_MAX_VALUE;
  }
}


/**
 * @brief 设置DAC输出波形的频率
 * @param freq_hz: 目标频率 (Hz)
 * @retval None
 */
void Set_DAC_Frequency(float freq_hz)
{
    // --- 根据公式反算 ARR ---
    // ARR = (定时器时钟 / (频率 * 点数 * (PSC+1))) - 1
    
    // 从CubeMX获取你的定时器时钟频率 (例如 SystemCoreClock / 2，取决于总线)
    // 假设htim6挂在APB1上，且APB1预分频不为1，则TIMxCLK = 2 * PCLK1
    // 如果APB1预分频为1，则TIMxCLK = PCLK1
    // 最简单的办法是在CubeMX中看TIM6的时钟源频率
    uint32_t tim_clk = 72000000; // **重要：请根据你的实际配置修改此值！**
    
    // 从CubeMX获取你的PSC值
    uint32_t psc = htim6.Init.Prescaler; // 或者直接写你设置的常数值
    
    // 获取查找表点数
    uint32_t samples = WAVE_SAMPLES; // 假设你的所有波形点数都一样
    
    // 防止除以零
    if (freq_hz == 0) return;
    
    uint32_t arr_value = (uint32_t)(tim_clk / (freq_hz * samples * (psc + 1))) - 1;

    // --- 更新定时器寄存器 ---
    // 检查ARR值是否在有效范围内 (16位定时器是 0-65535)
    if (arr_value > 65535) {
        arr_value = 65535; // 限制最大值，防止溢出
    }
    
    // 使用HAL库函数动态修改ARR的值
    // htim6 是你的DAC触发定时器的句柄
    __HAL_TIM_SET_AUTORELOAD(&htim6, arr_value);
    
    // 可选：打印调试信息
    // my_printf(&huart1, "Set Freq: %.1f Hz, New ARR: %lu\r\n", freq_hz, arr_value);
}




// --- 初始化函数 (在 main 函数或外设初始化后调用) ---
void dac_sin_init(void)
{
    // 1. 生成正弦波查找表数据
    //     amplitude = DAC_MAX_VALUE / 2 产生最大幅度的波形 (0-4095)
	
	switch(mode)
	{
		case 0:
    Generate_Sine_Wave(triangle_buffer, WAVE_SAMPLES, DAC_MAX_VALUE / 2, 0.0f);
    break;
		case 1:
    Generate_Triangle_Wave(triangle_buffer, WAVE_SAMPLES, DAC_MAX_VALUE / 2, 0.0f);
    break;
		case 2:
    Generate_Square_Wave(triangle_buffer, WAVE_SAMPLES, DAC_MAX_VALUE / 2, 0.7);
    break;
	}
    // 2. 启动触发 DAC 的定时器 (例如 TIM6)
    HAL_TIM_Base_Start(&htim6); // htim6 是 TIM6 的句柄
    
    // 3. 启动 DAC 通道并通过 DMA 输出查找表数据
    //    hdac: DAC 句柄
    //    DAC_CHANNEL_1: 要使用的 DAC 通道
    //    (uint32_t *)SineWave: 查找表起始地址 (HAL 库常需 uint32_t*)
    //    SINE_SAMPLES: 查找表中的点数 (DMA 传输单元数)
    //    DAC_ALIGN_12B_R: 数据对齐方式 (12 位右对齐)
    HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t *)triangle_buffer, WAVE_SAMPLES, DAC_ALIGN_12B_R);
}

// --- 无需后台处理任务 --- 
// 一旦 dac_sin_init 调用完成，硬件会自动循环输出波形
// adc_task() 中可以移除 dac 相关的处理



#define BUFFER_SIZE 1000

extern DMA_HandleTypeDef hdma_adc1;

uint32_t dac_val_buffer[BUFFER_SIZE / 2];
__IO uint32_t adc_val_buffer[BUFFER_SIZE];

__IO uint8_t AdcConvEnd = 0;

void adc_tim_dma_init(void)
{
		HAL_TIM_Base_Start(&htim3);
    HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_val_buffer, BUFFER_SIZE);
    __HAL_DMA_DISABLE_IT(&hdma_adc1, DMA_IT_HT);
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    UNUSED(hadc);
    if(hadc == &hadc1)
    {
        HAL_ADC_Stop_DMA(hadc);
        AdcConvEnd = 1;
    }
}

void adc_task(void)
{
    // 一次数据转换 3 + 12.5 =15.5个ADC时钟周期
    // 一次数据转换 15.5 / 22.5 = 0.68us
    // 1000个数据转换需要 1000 * 10 = 10000us = 10ms
//    while(!AdcConvEnd);
    if(AdcConvEnd)
    {
        for(uint16_t i = 0; i < BUFFER_SIZE / 2; i++)
        {
            dac_val_buffer[i] = adc_val_buffer[i * 2 + 1];
        }
        for(uint16_t i = 0; i < BUFFER_SIZE / 2; i++)
        {
            my_printf(&huart1, "%d\r\n", (int)dac_val_buffer[i]);
        }
        memset(dac_val_buffer, 0, sizeof(uint32_t) * (BUFFER_SIZE / 2));
        HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_val_buffer, BUFFER_SIZE);
        __HAL_DMA_DISABLE_IT(&hdma_adc1, DMA_IT_HT);
        AdcConvEnd = 0;
    }
}



/*

	1.DA可以发送正弦波 方波 三角波
	2.按键可以控制波形的周期
	3.旋钮可以控制波形的峰峰值
	4.串口最少可以打印出两个周期的波形（可以启动暂停）
	5.可以通过串口查询指令进行参数查询
	可查询参数：
		波形的类型[可以通过DA发送的模式变量去读取、通过AD读取的数据进行分析] 
		频率[不能通过定时器直接得出结果 必须通过AD计算] 
		峰峰值[不能通过定时器直接得出结果 必须通过AD计算]）
	6.可以通过串口控制指令进行模式切换以及参数设置[波形、周期、峰峰值]（具体指令集自定）

*/

