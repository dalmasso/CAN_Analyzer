#include "global.h"

// Config TIMER 3
void TIM3_Config(void)
{
	TIM_TimeBaseInitTypeDef		TIM_TimeBaseStructure;
	NVIC_InitTypeDef			NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);

	// Config Timer CAN BUS : 125Kbitbs/s
	TIM_TimeBaseStructure.TIM_Period = 4-1; // 125KHz
	TIM_TimeBaseStructure.TIM_Prescaler = 168-1; // 500KHz
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseStructure);

	// Enable interrupt
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	// Start timer & interrupt
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE);
	TIM_SetCounter(TIM3, 0);
	TIM_Cmd(TIM3,ENABLE);
}

// Config EXT interrupt Line 0
void EXTI0_Config(void)
{
	EXTI_InitTypeDef	EXT_BaseStructure;
	NVIC_InitTypeDef	NVIC_InitStructure;
	GPIO_InitTypeDef  	GPIO_InitStructure;

	// Initialize IO (GPIOA[0]) PIN R du transceiver
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

	EXT_BaseStructure.EXTI_Line = EXTI_Line0;
	EXT_BaseStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXT_BaseStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXT_BaseStructure.EXTI_LineCmd = ENABLE;

	// Enable interrupt
	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

	NVIC_Init(&NVIC_InitStructure);
	EXTI_Init(&EXT_BaseStructure);
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
}

// External Interrupt Handler
void EXTI0_IRQHandler(void)
{
	//GPIO_SetBits(GPIOA, GPIO_Pin_2);
	if (EXTI_GetITStatus(EXTI_Line0))
	{
		EXTI_ClearITPendingBit(EXTI_Line0);

		// Desinit EXTI
		EXTI_DeInit();

		// Config TIM3
		TIM3_Config();

		// Start Of Frame
	}
    //GPIO_ResetBits(GPIOA, GPIO_Pin_2);
}


// TM3 Interrupt Handler
void TIM3_IRQHandler(void)
{
	BaseType_t xHigherPriorityTaskWoken;
	static uint8_t pin_read[190] = {0};
	static uint8_t cnt = 0;
	static uint8_t cpt_end_frame = 0;

	//GPIO_SetBits(GPIOA, GPIO_Pin_2);
	xHigherPriorityTaskWoken = pdFALSE;

	if (TIM_GetITStatus(TIM3,TIM_IT_Update) != RESET)
	{
		// Read GPIO
		pin_read[cnt] = (GPIOA->IDR & GPIO_Pin_0) + 0x30;

		// Send GPIO value in queue
//		xQueueSendFromISR (xQueueISRToRead,&pin_read,&xHigherPriorityTaskWoken);

		// Detect End Of Frame
		if(0x31 == pin_read[cnt])
			cpt_end_frame++;
		else
			cpt_end_frame = 0;

		cnt++;

		if (cpt_end_frame >= 11) // 1 bit ACK delimiter + 7 bits EOF + 3 bit intertrame + le dernier bit à 0 pour le traitement
		{
			// Reset cpt_end_frame
			cpt_end_frame = 0;
			pin_read[cnt] = 0;
			cnt = 0;
			xQueueSendFromISR (xQueueISRToRead, pin_read,&xHigherPriorityTaskWoken);

			// Deinit TIM3
			TIM_DeInit(TIM3);

			// Init EXTI0
			EXTI0_Config();
		}

		TIM_ClearITPendingBit(TIM3,TIM_IT_Update);
	}
	portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
	//GPIO_ResetBits(GPIOA, GPIO_Pin_2);
}


void INIT_Injector(void)
{
	GPIO_InitTypeDef  	GPIO_InitStructure;

	// Initialize IO (GPIOA[1]) PIN T du transceiver
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

	GPIO_Init(GPIOA, &GPIO_InitStructure);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	// Set GPIOA[1]
	GPIO_WriteBit(GPIOA, GPIO_Pin_1, 1);
}

void TIM4_Config(void)
{
	TIM_TimeBaseInitTypeDef		TIM_TimeBaseStructure;
	NVIC_InitTypeDef			NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);

	// Config Timer CAN BUS : 125Kbitbs/s
	TIM_TimeBaseStructure.TIM_Period = 4-1; // 125KHz
	TIM_TimeBaseStructure.TIM_Prescaler = 168-1; // 500KHz
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM4,&TIM_TimeBaseStructure);

	// Enable interrupt
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void TIM4_Start(void)
{
	TIM4_Config();

	// Start timer & interrupt
	TIM_ITConfig(TIM4,TIM_IT_Update,ENABLE);
	TIM_SetCounter(TIM4, 0);
	TIM_Cmd(TIM4,ENABLE);
}

// TM4 Interrupt Handler
void TIM4_IRQHandler(void)
{
	BaseType_t xHigherPriorityTaskWoken;
	static uint8_t pin_read[190] = {0};
	static uint8_t cnt = 0;
	static uint8_t start = 0;

	//GPIO_SetBits(GPIOA, GPIO_Pin_2);
	xHigherPriorityTaskWoken = pdFALSE;

	if (TIM_GetITStatus(TIM4,TIM_IT_Update) != RESET)
	{

		// Read queue (once)
		if (0 == start)
		{
			xQueueReceiveFromISR(xQueueInject,pin_read,&xHigherPriorityTaskWoken);
			start = 1;
		}

		// Send to bus
		else
		{
			// Send to bus
			if (pin_read[cnt] != 0xFF)
			{
				GPIO_WriteBit(GPIOA, GPIO_Pin_1,pin_read[cnt]);
				cnt += 1;
			}

			// Stop transceiver
			else
			{
				start = 0;
				cnt = 0;

				// Deinit TIM4
				TIM_DeInit(TIM4);
			}
		}

		TIM_ClearITPendingBit(TIM4,TIM_IT_Update);
	}
	portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
}
