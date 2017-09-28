/**
* \file process.c
* \brief process library.
* \author Guillaume H. and Vincent .J
* \version 0.1
* \date 14 June 2017
*
* Contented the code for the Process tasks.
*
*/

#include "global.h"


/**
 * \task vTaskPROCESS_F_INJ
 * \brief Object coded CAN frame with error(s).
 *
 * The frame is created with a table of 0 or 1 in term of the PROCESS_message_inj table received by UART.
 */

void ProcessTaskInj(void *pvParameters) {
	PROCESS_message_inj L_PROCESS_message_inj;
	PROCESS_INJ_var L_PROCESS_INJ_var;

	uint8_t message[30];

	int index = 0,i,j;
	uint16_t CRC_value=0;

	int etat = 0;
	int cnt = 0;

	uint8_t Frame[MAX_LENGHT_ROUGH_FRAME];
	PROCESS_STATE_INJ = WAIT;
	while(1)
	{

		switch (PROCESS_STATE_INJ){
			case WAIT :
				if( xQueueReceive( RxQueue, message, ( TickType_t ) 100 ) == pdFAIL)
				{
					PROCESS_STATE_INJ = WAIT;
				}
				else
				{
					for(cnt = 0; cnt < 30; cnt++)
					{
						switch(etat)
						{
							case 0 :
								if(message[cnt] == 'I' && message[cnt+1] == 'D')
								{
									cnt += 2;
									for(i=0; i<4; i++) L_PROCESS_message_inj.ID[i] = message[i+cnt];
									etat = 1;
								}
								break;

							case 1 :
								if(message[cnt] == 'R' && message[cnt+1] == 'T')
								{
									cnt += 2;
									L_PROCESS_message_inj.RTR_value = message[cnt] - 0x30;
									etat = 2;
								}
								break;

							case 2 :
								if(message[cnt] == 'D' && message[cnt+1] == 'T')
								{
									cnt += 2;
									for(i=0; i<8; i++) L_PROCESS_message_inj.DATA[i] = message[i+cnt];
									etat = 3;
								}
								break;

							case 3 :
								if(message[cnt] == 'D' && message[cnt+1] == 'L')
								{
									cnt += 2;
									L_PROCESS_message_inj.DLC_value = message[cnt] - 0x30;
									etat = 4;
								}
								break;

							case 4 :
								if(message[cnt] == 'E' && message[cnt+1] == 'R')
								{
									cnt += 2;
									L_PROCESS_message_inj.Apply_RTR_error = 0;
									L_PROCESS_message_inj.Apply_R0_error = 0;
									L_PROCESS_message_inj.Apply_DLC_error = 0;
									L_PROCESS_message_inj.Apply_CRC_error = 0;
									L_PROCESS_message_inj.Apply_STUFF_error = 0;
									L_PROCESS_message_inj.Apply_Blackout_error = 0;
									switch(message[cnt])
									{
										case '1' :
											L_PROCESS_message_inj.Apply_DLC_error = 1;
											break;
										case '2' :
											L_PROCESS_message_inj.Apply_RTR_error = 1;
											break;
										case '3' :
											L_PROCESS_message_inj.Apply_CRC_error = 1;
											break;
										case '4' :
											L_PROCESS_message_inj.Apply_STUFF_error = 1;
											break;
										case '5' :
											L_PROCESS_message_inj.Apply_Blackout_error = 1;
											break;
										case '6' :
											L_PROCESS_message_inj.Apply_R0_error = 1;
											break;
										default : break;
									}
									etat = 0;
									PROCESS_STATE_INJ = CREATE_PUR_FRAME;
								}
								break;
							default : break;
						}
					}
				}
				break;
			case CREATE_PUR_FRAME :
				//Scan the queue
				PROCESS_INJ_Scan_queue(&L_PROCESS_INJ_var, &L_PROCESS_message_inj);
				//Integrate SOF
				Frame[L_PROCESS_INJ_var.A_SOF] = 0;
				//Integrate ID
				/*for(i=0; i<8 ;i++)
				{
					//Frame[L_PROCESS_INJ_var.A_ID+i] = (L_PROCESS_message_inj.ID[0]>>(7-i)) & 0x01;
					Frame[L_PROCESS_INJ_var.A_ID+i] = (L_PROCESS_message_inj.ID[0]>>(7-i)) & 0x01;
				}
				for(; i<11;i++)
				{
					Frame[L_PROCESS_INJ_var.A_ID+i] = (L_PROCESS_message_inj.ID[1]>>(15-i)) & 0x01;
				}*/
				for(i=0; i<3 ; i++)
				{
					Frame[L_PROCESS_INJ_var.A_ID+i] = (L_PROCESS_message_inj.ID[1]>>(2-i)) & 0x01;
				}
				for(i=0; i<8 ; i++)
				{
					Frame[L_PROCESS_INJ_var.A_ID+i+3] = (L_PROCESS_message_inj.ID[0]>>(7-i)) & 0x01;
				}
				//Integrate RTR
				Frame[L_PROCESS_INJ_var.A_RTR] = L_PROCESS_message_inj.RTR_value;
				//Integrate RO,R1
				Frame[L_PROCESS_INJ_var.A_R0] = 0;
				Frame[L_PROCESS_INJ_var.A_R1] = 0;
				//Integrate DLC
				for(i=0; i<4 ;i++)
				{
					Frame[L_PROCESS_INJ_var.A_DLC+i] = (L_PROCESS_message_inj.DLC_value>>(3-i)) & 0x01;
				}
				//Integrate DATA
				index = 0;
				for(j=0;j<L_PROCESS_INJ_var.NB_DATA;j++)
				{
					for(i=0; i<8 ;i++)
					{
						Frame[L_PROCESS_INJ_var.A_DATA+index] = (L_PROCESS_message_inj.DATA[j]>>(7-i)) & 0x01;
						index++;
					}
				}
				//Integrate CRC
				//CRC_value = MakeCRC_INJ(Frame); //todo check if code of Guillaume is OK
				for(i=0; i<15 ;i++)
				{
					Frame[L_PROCESS_INJ_var.A_CRC+i] = (CRC_value<<i) && 0x8000;
				}
				//Integrate CRC DELIMITOR
				Frame[L_PROCESS_INJ_var.A_CRC_DEL] = 1;
				//Integrate ACK
				Frame[L_PROCESS_INJ_var.A_ACK] = 1;
				//Integrate DELIMITOR
				Frame[L_PROCESS_INJ_var.A_DEL] = 1;
				//Integrate EOT
				for(i=0; i<7 ;i++)
				{
					Frame[L_PROCESS_INJ_var.A_EOT+i] = 0x01;
				}
				//Integrate INTER TRAME
				for(i=0; i<3 ;i++)
				{
					Frame[L_PROCESS_INJ_var.A_INTER_T+i] = 0x01;
				}
				//Integrate our END of FRAME
				Frame[L_PROCESS_INJ_var.A_OUR_INTER_T] = 0xFF;


				PROCESS_STATE_INJ = BLACKOUT_ERROR;
				break;
			case BLACKOUT_ERROR :
				// Create a blackout
				if(L_PROCESS_message_inj.Apply_Blackout_error == TRUE){
					//create a table of 0 data
					for(index=0; index< MAX_LENGHT_PURE_FRAME;index++)
					{
						Frame[index] = 0;
					}
					//Add our end of frame
					Frame[index] = 0xFF;
					PROCESS_STATE_INJ = SEND_FRAME;
				}
				else {
					PROCESS_STATE_INJ = RTR_ERROR;
				}
				break;
			case RTR_ERROR :
				if(L_PROCESS_message_inj.Apply_RTR_error == TRUE){

					Frame[L_PROCESS_INJ_var.A_RTR] = !L_PROCESS_message_inj.RTR_value;
					PROCESS_STATE_INJ = SEND_FRAME;
				}
				else
				{
					PROCESS_STATE_INJ = CRC_ERROR;
				}
				break;
			case CRC_ERROR :
				if(L_PROCESS_message_inj.Apply_CRC_error == TRUE){
					//disturb CRC value
					Frame[L_PROCESS_INJ_var.A_CRC+1] = !Frame[L_PROCESS_INJ_var.A_CRC+1];
					PROCESS_STATE_INJ = SEND_FRAME;
				}
				else
				{
					PROCESS_STATE_INJ = R0_ERROR;
				}
				break;
			case R0_ERROR :
				if(L_PROCESS_message_inj.Apply_R0_error == TRUE){
					Frame[L_PROCESS_INJ_var.A_R0] = !Frame[L_PROCESS_INJ_var.A_R0];
					PROCESS_STATE_INJ = SEND_FRAME;
				}
				else
				{
					PROCESS_STATE_INJ = DLC_ERROR;
				}
				break;
			case DLC_ERROR :
				if(L_PROCESS_message_inj.Apply_DLC_error == TRUE){
					Frame[L_PROCESS_INJ_var.A_DLC] = !Frame[L_PROCESS_INJ_var.A_DLC];
					PROCESS_STATE_INJ = SEND_FRAME;
				}
				else
				{
					PROCESS_STATE_INJ = SEND_FRAME;
				}
				break;

			case SEND_FRAME :
				//checked if STUFF error is selected
				if(L_PROCESS_message_inj.Apply_STUFF_error == TRUE)
				{
					//ADD bits stuffing in the frame with one error
					ADD_bitsStuffing_error(Frame,&L_PROCESS_INJ_var);
				}
				else
				{
					//ADD bits stuffing in the frame
					ADD_bitsStuffing(Frame,&L_PROCESS_INJ_var);
				}
				//send Frame in the queue xQueueInject
				xQueueSend(xQueueInject, Frame,portMAX_DELAY);
				TIM4_Start();
				PROCESS_STATE_INJ = WAIT;
				break;
			default :
				break;
		}
	}
}

void PROCESS_INJ_Scan_queue(PROCESS_INJ_var *F_PROCESS_INJ_var, PROCESS_message_inj *F_PROCESS_message_inj )
{
	F_PROCESS_INJ_var->A_SOF = 0;
	F_PROCESS_INJ_var->A_ID = F_PROCESS_INJ_var->A_SOF + 1;
	F_PROCESS_INJ_var->A_RTR = F_PROCESS_INJ_var->A_ID + 11;
	F_PROCESS_INJ_var->A_R0 = F_PROCESS_INJ_var->A_RTR + 1;
	F_PROCESS_INJ_var->A_R1 = F_PROCESS_INJ_var->A_R0  + 1;
	F_PROCESS_INJ_var->A_DLC = F_PROCESS_INJ_var->A_R1 + 1;
	F_PROCESS_INJ_var->NB_DATA = F_PROCESS_message_inj->DLC_value;
	if(F_PROCESS_message_inj->RTR_value == TRUE)
	{
		F_PROCESS_INJ_var->A_DATA = 0; //there are no DATA
		F_PROCESS_INJ_var->A_CRC = F_PROCESS_INJ_var->A_DLC  + 4;
	}
	else
	{
		F_PROCESS_INJ_var->A_DATA = F_PROCESS_INJ_var->A_DLC  + 4;
		F_PROCESS_INJ_var->A_CRC = F_PROCESS_INJ_var->A_DATA  + F_PROCESS_message_inj->DLC_value * 8;
	}
	F_PROCESS_INJ_var->A_CRC_DEL = F_PROCESS_INJ_var->A_CRC  + 15;
	F_PROCESS_INJ_var->A_ACK = F_PROCESS_INJ_var->A_CRC_DEL  + 1;
	F_PROCESS_INJ_var->A_DEL = F_PROCESS_INJ_var->A_ACK  + 1;
	F_PROCESS_INJ_var->A_EOT = F_PROCESS_INJ_var->A_DEL  + 1;
	F_PROCESS_INJ_var->A_INTER_T = F_PROCESS_INJ_var->A_EOT  + 7;
	F_PROCESS_INJ_var->A_OUR_INTER_T = F_PROCESS_INJ_var->A_INTER_T + 3;
}


uint16_t *MakeCRC_INJ(uint8_t *bitString)
{
	int i=0;
	int j=0;

	uint8_t bitsStringUn = concatenateBits(bitString);

	int sizeFrameClc = strlen(bitsStringUn)-15;
	uint16_t crc=0;

	uint8_t dataCtn[13]={0,0,0,0,0,0,0,0,0,0,0,0,0};
	uint8_t dataCrcArray[64];
	for(i=0; i<64; i++){
		dataCrcArray[i] = '0';
	}
	memcpy(dataCrcArray, bitsStringUn,sizeFrameClc);
	//memcpy(dataCtn, concatenateBits_DataCRC(dataCrcArray),13);

	int iMax = ((int)sizeFrameClc/8);
	int jMax = sizeFrameClc-(iMax*8)-1;

	for (i = 0; i < iMax+1; i++) {
		crc ^= (uint16_t)dataCtn[i] << 7;
		for (j = 0; j < 8; j++){
			crc <<= 1;
			if (crc & 0x8000){
				crc ^= 0xc599;
			}

			if(i==iMax && j==jMax){
				j=8;
			}
		}
		crc = crc & 0x7fff;
	}

	return crc;
}

uint16_t concatenateBits(uint8_t *bits){
	int i=0;
	int sizeStr=15;//sizeof(bits);
	static uint16_t ctnBytes=0;

	for(i=0; i<sizeStr; i++){
		ctnBytes += (bits[i] << (14-i));
	}
	return ctnBytes;
}

void ADD_bitsStuffing(uint8_t* F_Frame, PROCESS_INJ_var* F_PROCESS_INJ_var)
{
	int compt=0,i,j;
	uint8_t SizeFrame = (F_PROCESS_INJ_var->A_CRC_DEL);
	uint8_t SizeFrameShift  = (F_PROCESS_INJ_var->A_OUR_INTER_T);
	for(i=0; i<(SizeFrame-1); i++)
	{
		//checked if n == n+1
		if(F_Frame[i] == F_Frame[i+1])
		{
			compt++;
		}
		else
		{
			compt=0;
		}

		//checked if compt>=5
		if(compt>=4)
		{
			//shift all data after current index i
			for(j=SizeFrameShift; j>(i+1);j--)
			{
				F_Frame[j+1] = F_Frame[j];
			}
			F_Frame[i+2] = !F_Frame[i+1];
			SizeFrameShift++;
			SizeFrame++;
			compt=0;
		}

	}
}

void ADD_bitsStuffing_error(uint8_t* F_Frame, PROCESS_INJ_var* F_PROCESS_INJ_var)
{
	int compt=0,i,j;
	uint8_t SizeFrame = (F_PROCESS_INJ_var->A_CRC_DEL);
	uint8_t SizeFrameShift  = (F_PROCESS_INJ_var->A_OUR_INTER_T);

	uint8_t error =0;
	for(i=0; i<(SizeFrame-1); i++)
	{
		//checked if n-1 == n
		if(F_Frame[i] == F_Frame[i+1])
		{
			compt++;
		}
		else
		{
			compt=0;
		}

		//checked if compt>=5
		if(compt>=4)
		{
			if(error == 0)
			{
				//nothing is done, it's an opportunity ton add an error ...
				error=1;
				compt=0;
			}
			else
			{
				//shift all data after current index i
				for(j=SizeFrameShift; j>(i+1);j--)
				{
					F_Frame[j+1] = F_Frame[j];
				}
				F_Frame[i+2] = !F_Frame[i+1];
				SizeFrameShift++;
				SizeFrame++;
				compt=0;
			}
		}
	}
}

