#include "global.h"

//Initialize GPIO and USART1
void USART1_setup(void)
{
	GPIO_InitTypeDef GPIO_InitStruct; // this is for the GPIO pins used as TX and RX
	USART_InitTypeDef USART_InitStruct; // this is for the USART1 initilization
	NVIC_InitTypeDef NVIC_InitStructure; // this is used to configure the NVIC (nested vector interrupt controller)

	// enable APB2 peripheral clock for USART1
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	// enable the peripheral clock for the pins used by
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

	/* The RX and TX pins are now connected to their AF
	 * so that the USART1 can take over control of the
	 * pins
	 */
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_USART1);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_USART1);

	// This sequence sets up the TX and RX pins
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7; // Pins 6 (TX) and 7 (RX) are used
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF; 			// the pins are configured as alternate function so the USART peripheral has access to them
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;		// this defines the IO speed and has nothing to do with the baudrate!
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;			// this defines the output type as push pull mode (as opposed to open drain)
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;			// this activates the pullup resistors on the IO pins
	GPIO_Init(GPIOB, &GPIO_InitStruct);					// now all the values are passed to the GPIO_Init() function which sets the GPIO registers

	/* Now the USART_InitStruct is used to define the
	 * properties of USART1
	 */
	USART_InitStruct.USART_BaudRate = 115200;				// the baudrate is set to the value we passed into this init function
	USART_InitStruct.USART_WordLength = USART_WordLength_8b;// we want the data frame size to be 8 bits (standard)
	USART_InitStruct.USART_StopBits = USART_StopBits_1;		// we want 1 stop bit (standard)
	USART_InitStruct.USART_Parity = USART_Parity_No;		// we don't want a parity bit (standard)
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // we don't want flow control (standard)
	USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx; // we want to enable the transmitter and the receiver
	USART_Init(USART1, &USART_InitStruct);					// again all the properties are passed to the USART_Init function which takes care of all the bit setting

	/* Here the USART1 receive interrupt is enabled
	 * and the interrupt controller is configured
	 * to jump to the USART1_IRQHandler() function
	 * if the USART1 receive interrupt occurs
	 */
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE); // enable the USART1 receive interrupt
	USART_ITConfig(USART1, USART_IT_TXE, DISABLE); // enable the USART1 transmit interrupt

	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;		 // we want to configure the USART1 interrupts
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;// this sets the priority group of the USART1 interrupts
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		 // this sets the subpriority inside the group
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			 // the USART1 interrupts are globally enabled
	NVIC_Init(&NVIC_InitStructure);							 // the properties are passed to the NVIC_Init function which takes care of the low level stuff

	// finally this enables the complete USART1 peripheral
	USART_Cmd(USART1, ENABLE);
}

uint8_t USART1_puts(char *string)
{
	// Take semaphore
	if(xSemaphoreTake(TxMutexSemaphore, 1000/portTICK_RATE_MS) == pdFAIL)
	{
		return 0;
	}

	while(*string)
	{
		// wait until data register is empty
		while(!USART_GetFlagStatus(USART1, USART_FLAG_TC));
		USART_SendData(USART1, *string++);
	}

	xSemaphoreGive(TxMutexSemaphore); // Return semaphore

	return 1;
}


uint8_t USART1_putHEX(char *string, uint8_t size)
{
	uint8_t i=0;

	// Take semaphore
	if(xSemaphoreTake(TxMutexSemaphore, 1000/portTICK_RATE_MS) == pdFAIL)
	{
		return 0;
	}

	for(i=0;i<size;i++)
	{
		// wait until data register is empty
		while(!USART_GetFlagStatus(USART1, USART_FLAG_TC));
		USART_SendData(USART1, string[i]);
	}

	xSemaphoreGive(TxMutexSemaphore); // Return semaphore

	return 1;
}

uint8_t USART1_struct(Process_Message can)
{
	USART1_puts("ID");
	USART1_putHEX(can.ID,4);
	USART1_puts("DT");
	USART1_putHEX(can.DATA, 8);
	USART1_puts("EF");
	USART1_putHEX(&can.FormError, 1);
	USART1_puts("EB");
	USART1_putHEX(&can.BitStuffError, 1);
	USART1_puts("EC");
	USART1_putHEX(&can.CRCError, 1);
	USART1_puts("EA");
	USART1_putHEX(&can.ACKError, 1);

	return 1;
}

// this is the interrupt request handler (IRQ) for ALL USART1 interrupts
void USART1_IRQHandler(void)
{
	static uint8_t string_from_pc[SIZE_STRING_PC];
	static uint8_t cnt = 0; // this counter is used to determine the string length
	uint8_t temp = 0;
	BaseType_t xHigherPriorityTaskWoken;

	// check if the USART1 receive interrupt flag was set
//	if( USART_GetITStatus(USART1, USART_IT_RXNE) )
//	{
		temp = USART1->DR;
		if( (temp != '\n') & (cnt < SIZE_STRING_PC) )
		{
			string_from_pc[cnt] = temp;
			cnt++;
		}
		else
		{
			string_from_pc[cnt] = '\0';
			cnt = 0;
			xHigherPriorityTaskWoken = pdFALSE;
			xQueueSendToBackFromISR( RxQueue, ( void * ) string_from_pc, &xHigherPriorityTaskWoken );
			portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
		}
//	}
	// check if the USART1 transmit interrupt flag was set
//	if( USART_GetITStatus(USART3, USART_IT_TXE) )
//	{
//	}
}

//Task For Sending Data Via USART
void ComPcTask(void *pvParameters)
{
	uint8_t string_from_pc[30] = "IDAAAART0DTBBBBBBBBDL8ER0";
//	uint8_t tab[190] = {0};
//	char message;
//	Process_Message pxMessage = { 	{0x10, 0x11, 0x12, 0x13},
//									0,
//									{0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08},
//									0,
//									0,
//									0,
//									0,
//									1,
//									0	};
//	pxMessage = & Process_Message;

	while(1)
	{
		if( xQueueSendToBack( RxQueue, ( void * ) string_from_pc, ( TickType_t ) 10 ) != pdPASS  );
//		USART1_struct(pxMessage);
		vTaskDelay(5000);

//		if( xQueueReceive( RxQueue, &message, ( TickType_t ) 100 ) == pdFAIL)
//		{}
//		else
//		{
//			USART1_puts(&message);
//			pxMessage.FormError = 1;
//			xQueueSend( ProcessQueue, ( void * ) &pxMessage, ( TickType_t ) 0 );
//		}

//		if( xQueueReceive( xQueueISRToRead, &message, ( TickType_t ) 100 ) == pdFAIL)
//		{}
//		else
//		{
//			USART_SendData(USART1, message);
//		}
//		if( xQueueReceive( xQueueISRToRead, tab, ( TickType_t ) 100 ) == pdFAIL)
//		{}
//		else
//		{
//			USART1_puts(tab);
//		}

		vTaskDelay(100);
	}
}
