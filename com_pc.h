#include "process_can.h"

void ComPcTask(void *pvParameters);
void USART1_setup(void);
uint8_t USART1_puts(char *string);
void USART1_IRQHandler(void);
uint8_t USART1_putHEX(char *string, uint8_t size);
uint8_t USART1_struct(Process_Message can);
