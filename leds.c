#include "global.h"

void LED_setup(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	GPIO_InitStructure.GPIO_Pin = LED_Pin_1 | LED_Pin_2 | LED_Pin_3 |LED_Pin_4;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
}

void vTaskLED1(void *pvParameters)
{
	while(1)
	{
		GPIO_SetBits(GPIOD, LED_Pin_1);
		vTaskDelay(10);
		GPIO_ResetBits(GPIOD, LED_Pin_1);
		vTaskDelay(10);
	}
}

void vTaskLED2(void *pvParameters)
{
	while(1)
	{
		GPIO_SetBits(GPIOD, LED_Pin_2);
		vTaskDelay(100);
		GPIO_ResetBits(GPIOD, LED_Pin_2);
		vTaskDelay(100);
	}
}

void vTaskLED3(void *pvParameters)
{
	while(1)
	{
		GPIO_SetBits(GPIOD, LED_Pin_3);
		vTaskDelay(500);
		GPIO_ResetBits(GPIOD, LED_Pin_3);
		vTaskDelay(500);
	}
}

void vTaskLED4(void *pvParameters)
{
	while(1)
	{
		GPIO_SetBits(GPIOD, LED_Pin_4);
		vTaskDelay(1000);
		GPIO_ResetBits(GPIOD, LED_Pin_4);
		vTaskDelay(1000);
	}
}
