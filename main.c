#include "global.h"
#include "process_can.h"

//! RX queue. Uart interrupt places characters on this queue as soon as they are received.
xQueueHandle RxQueue;
//! Process queue
xQueueHandle ProcessQueue;
//! ISR timer3 queue
xQueueHandle xQueueISRToRead;
//! ISR timer4 queue
xQueueHandle xQueueInject;

//!< Mutex semaphore for allowing only one task to write to the tx buffer at once
xSemaphoreHandle TxMutexSemaphore = 0;

int main(void)
{
	char start[20] = "\nCAN v0.9\n";

	// Create RX Queue
	RxQueue = xQueueCreate(3, sizeof( uint8_t [30] ) );
	if( RxQueue == 0 )
	{
		return -1; // Failed to create the queue.
	}

	// Create Process Queue
	ProcessQueue = xQueueCreate(7, sizeof( Process_Message ) );
	if( ProcessQueue == 0 )
	{
		return -1; // Failed to create the queue.
	}

	xQueueISRToRead = xQueueCreate(3, sizeof( uint8_t [190]));
	if( xQueueISRToRead == 0 )
	{
		return -1; // Failed to create the queue.
	}

	//Queue send to the transceiver
	xQueueInject = xQueueCreate(3, sizeof( uint8_t [190]));
	if( xQueueInject == 0 )
	{
		return -1; // Failed to create the queue.
	}

	// Create TX mutex semaphore
	TxMutexSemaphore = xSemaphoreCreateMutex();
	if( TxMutexSemaphore == NULL )
	{
		return -1; /* There was insufficient FreeRTOS heap available for the semaphore to
		be created successfully. */
	}

	// 4 bits for preemp priority, 0 bit for sub priority
	// lower number has higher priority
//	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	LED_setup();
	USART1_setup();
	EXTI0_Config();
	INIT_Injector();
	TIM4_Config();
	USART1_puts(start);

	// Low priority numbers denote low priority tasks
	xTaskCreate( vTaskLED1, "LED1", configMINIMAL_STACK_SIZE, NULL, 0,( xTaskHandle * ) NULL);
	xTaskCreate( vTaskLED2, "LED2", configMINIMAL_STACK_SIZE, NULL, 0,( xTaskHandle * ) NULL);
	xTaskCreate( vTaskLED3, "LED3", configMINIMAL_STACK_SIZE, NULL, 0,( xTaskHandle * ) NULL);
	xTaskCreate( vTaskLED4, "LED4", configMINIMAL_STACK_SIZE, NULL, 0,( xTaskHandle * ) NULL);
	xTaskCreate( ComPcTask, "UsartTask", configMINIMAL_STACK_SIZE, NULL, 1,( xTaskHandle * ) NULL);
	xTaskCreate( ProcessTask, "ProcessTask", configMINIMAL_STACK_SIZE, NULL, 1,( xTaskHandle * ) NULL);
	xTaskCreate( ProcessTaskInj, "ProcessTaskInj", configMINIMAL_STACK_SIZE, NULL, 1,( xTaskHandle * ) NULL);

	vTaskStartScheduler();

	while(1);
}
