#include "key_app.h"

uint8_t key_val,key_old,key_down,key_up;

uint8_t key_read()
{
	uint8_t temp = 0;
	
if(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_15)==GPIO_PIN_RESET)temp=6;
	if(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_13)==GPIO_PIN_RESET)temp=2;
	if(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_11)==GPIO_PIN_RESET)temp=3;
	if(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_9)==GPIO_PIN_RESET)temp=4;
	if(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_7)==GPIO_PIN_RESET)temp=5;
	if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0)==GPIO_PIN_RESET)temp=1;
	
	return temp;
}

void key_task()
{
	key_val = key_read();
	key_down = key_val & (key_old ^ key_val);
	key_up = ~key_val & (key_old ^ key_val);
	key_old = key_val;
	
	if(key_down == 1)
	{
		ucLed[0] ^= 1;
	}
	
}



//if(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_15)==GPIO_PIN_RESET)temp=1;
//	if(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_13)==GPIO_PIN_RESET)temp=2;
//	if(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_11)==GPIO_PIN_RESET)temp=3;
//	if(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_9)==GPIO_PIN_RESET)temp=4;
//	if(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_7)==GPIO_PIN_RESET)temp=5;
//	if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0)==GPIO_PIN_RESET)temp=1;


