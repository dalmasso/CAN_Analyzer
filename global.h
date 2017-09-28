#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_usart.h"
#include "stm32f4xx_exti.h"
#include "misc.h"
#include "string.h"
#include "stdio.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "com_pc.h"
#include "com_can.h"
#include "leds.h"
#include "com_can.h"
#include "process_can.h"
#include "process_can_inj.h"

// CAN = PA0
// white wire = RX = PB6
// green wire = TX = PB7

// PORT D
#define LED_Pin_1 GPIO_Pin_12 // LD4
#define LED_Pin_2 GPIO_Pin_13 // LD3
#define LED_Pin_3 GPIO_Pin_14 // LD5
#define LED_Pin_4 GPIO_Pin_15 // LD6

// CAN bus speed
#define CAN_FREQ_HZ			125000
#define TIMER1_CLK_HZ		84000000
#define TIMER1_PRESCALER	((TIMER1_CLK_HZ/CAN_FREQ_HZ)-1)

#define SIZE_STRING_PC 		30 // 20 caractères
#define MAX_LENGHT_FRAME	190	// 135 bits max

//! RX queue. Uart interrupt places characters on this queue as soon as they are received.
extern xQueueHandle RxQueue;
//! Process queue
extern xQueueHandle ProcessQueue;
//! ISR timer queue
extern xQueueHandle xQueueISRToRead;
//! ISR timer queue
extern xQueueHandle xQueueInject;
//! Mutex semaphore for allowing only one task to write to the tx buffer at once
extern xSemaphoreHandle TxMutexSemaphore;
