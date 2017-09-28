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

//Task For Sending Data Via USART
//void ProcessTask(void *pvParameters)
//{
//	struct xMessage pxRxedMessage;
//	while(1)
//	{
//		if( xQueueReceive( ProcessQueue, &( pxRxedMessage ), portMAX_DELAY) == pdFAIL)
//		{
//		}
//		else
//		{
//			vTaskDelay(10);
//		}
//		vTaskDelay(100);
//	}
//}

/**
 * \task vTaskPROCESS
 * \brief Object decoded CAN frame with error(s) in string characters.
 *
 * PROCESS_MessageTypeDef is the entire description of the CAN frame decoded.
 * Errors are been showed by a '1' characters, '0' value is used if there are not error and by default.
 */

void ProcessTask(void *pvParameters)
{
	Process_Message L_PROCESS_MessageUART;

	int idxCRC=0;

	uint8_t pureFrame[MAX_LENGHT_ROUGH_FRAME];//frame without bitstuffing
	uint8_t roughFrame[MAX_LENGHT_ROUGH_FRAME];//todo change it back to roughFrame

	PROCESS_INIT(&L_PROCESS_MessageUART, pureFrame, roughFrame);

	int sizeFrame=0;

	while(1)
	{
		switch (PROCESS_STATE)
		{
			case WAIT :
				//receive one frame from the queue
				if( xQueueReceive( xQueueISRToRead,roughFrame,portMAX_DELAY ) == pdPASS )
				{
//					PROCESS_INIT(&L_PROCESS_MessageUART, pureFrame, roughFrame);
					PROCESS_STATE = SIZE; //TODO uncommented
				}
				else
				{
					PROCESS_STATE = WAIT;
				}
				break;
			case SIZE :
				sizeFrame = strlen((char*)roughFrame);
				//Size frame < sizeMaxValid ?
				if(sizeFrame<MAX_LENGHT_ROUGH_FRAME){
					PROCESS_STATE = BIT_STUFFING;
				}
				else
				{
					L_PROCESS_MessageUART.FormError = TRUE;
					PROCESS_STATE = WAIT;
				}
				break;
			case BIT_STUFFING :
				//delete bit stuffing
				L_PROCESS_MessageUART.BitStuffError = unstuffed_bit(roughFrame, pureFrame, sizeFrame);
				PROCESS_STATE = (L_PROCESS_MessageUART.BitStuffError == 0) ? GET_ID : WAIT;
				break;

			case GET_ID :
				L_PROCESS_MessageUART.FormError = extract_id_dlc(&L_PROCESS_MessageUART, roughFrame, pureFrame, &idxCRC);
				PROCESS_STATE = (L_PROCESS_MessageUART.FormError == 0) ? CHECK_CRC : WAIT;
				break;

			case CHECK_CRC :
				L_PROCESS_MessageUART.CRCError = check_crc(pureFrame);
				PROCESS_STATE = SEND;
				break;

			case SEND :
				USART1_struct(L_PROCESS_MessageUART);
				PROCESS_STATE = WAIT;
				break;

			default :
				break;
		}
	}
}

void PROCESS_INIT(Process_Message *PROCESS_MessageUART,uint8_t *_pureFrame,uint8_t *_roughFrame){
	int i=0;

	//_PROCESS_MessageUART init by 0
	for(i=0; i<4; i++){
		PROCESS_MessageUART->ID[i]= 0;
	}
	for(i=0; i<8; i++){
		PROCESS_MessageUART->DATA[i]= 0;
	}
	PROCESS_MessageUART->RTR = 0;
	PROCESS_MessageUART->DLC = 0;
	PROCESS_MessageUART->FormError = 0;
	PROCESS_MessageUART->BitStuffError = 0;
	PROCESS_MessageUART->CRCError = 0;
	PROCESS_MessageUART->ACKError = 0;
	PROCESS_MessageUART->RECNumber = 0;

	//_pureFrame init by 0
	for(i=0; i<MAX_LENGHT_ROUGH_FRAME; i++){
		_pureFrame[i] = 0;
	}

	//_roughFrame init by 0
	for(i=0; i<MAX_LENGHT_ROUGH_FRAME; i++){
		_roughFrame[i] = 0;
	}
}

uint8_t unstuffed_bit(uint8_t *roughFrame, uint8_t *pureFrame, int roughFrameSize){
	int cnt=0;//counter for bit de-stuff
	int i=0;
	char chBitStuffing = '0';
	int next=0;
	uint8_t errorType = 0;

	//0 0 0 0 0 1 0 1 1
	//0 0 0 0

	for (i = 0; i < roughFrameSize-NBR_BITS_OUT_OF_BITSTUFFING; i++) {//remove bit stuffing
		//manage the bit state change
		if( cnt < 5){
			pureFrame[next] = roughFrame[i];
			next++;
			cnt++;
			if(roughFrame[i] != chBitStuffing){
				cnt=1;
				chBitStuffing = (chBitStuffing=='1') ? '0' : '1';
			}
		}else{
			if(roughFrame[i] == chBitStuffing){
				errorType=1;
				i = roughFrameSize+NBR_BITS_OUT_OF_BITSTUFFING; //leave the for loop
			}else{
				cnt=0;
			}
		}
	}
	return errorType;
}

uint8_t extract_id_dlc(Process_Message *L_PROCESS_MessageUART, uint8_t *roughFrame, uint8_t *pureFrame, int *idxCRC){
	int i=0;
	uint8_t errorType = 0;
	int sizePureFrame = strlen((char*)pureFrame);
	uint8_t sizeDataBlock = 0;
	uint8_t dataArray[64];
	uint8_t idArray[32];
	int tempSize =0;

	uint8_t idArrayCtn[4] = {0,0,0,0};
	uint8_t dataArrayCtn[8] = {0,0,0,0,0,0,0,0};

	for(i=0; i<32; i++){
		idArray[i] = '0';
	}
	for(i=0; i<64; i++){
		dataArray[i] = '0';
	}

	//calcul DLC if ID=11bits
	for(i=0;i<4;i++){
		sizeDataBlock += ((pureFrame[15+i]-48) << (3-i));
	}

	tempSize = 8 + 11 + 8*(int)sizeDataBlock +15;

	if(tempSize==sizePureFrame && pureFrame[13]=='0' && pureFrame[14]=='0'){//test id is 11bits
		memcpy(&idArray[21], &pureFrame[1], 11);//isolate ID
		memcpy(&dataArray[64-(8*(int)sizeDataBlock)], &pureFrame[19], 8*(int)sizeDataBlock);//isolate DATA


		concatenateBits_Id(idArray, idArrayCtn);
		memcpy(L_PROCESS_MessageUART->ID, idArrayCtn, 4);//save ID

		concatenateBits_Data(dataArray,dataArrayCtn);
		memcpy(L_PROCESS_MessageUART->DATA,dataArrayCtn , 8);//save DATA
		L_PROCESS_MessageUART->DLC = sizeDataBlock;
		L_PROCESS_MessageUART->RTR = pureFrame[12]-48;
		*idxCRC=19+sizeDataBlock;
	}else{
		//calcul DLC if ID=29bits
		for(i=0;i<4;i++){
			sizeDataBlock += (pureFrame[33+i] << (3-i) );
		}

		if(tempSize==sizePureFrame && pureFrame[31]=='0' && pureFrame[32]=='0'){//test id is 29bits
			memcpy(&idArray[3], &pureFrame[1], 29);//isolate ID
			memcpy(&dataArray[64-(8*(int)sizeDataBlock)], &pureFrame[19], 8*(int)sizeDataBlock);//isolate DATA

//			memcpy(L_PROCESS_MessageUART->ID, concatenateBits_Id(idArray), 4);//save ID
//			memcpy(L_PROCESS_MessageUART->DATA, concatenateBits_Data(dataArray), 8);//save DATA
			L_PROCESS_MessageUART->DLC = sizeDataBlock;
			L_PROCESS_MessageUART->RTR = pureFrame[30]-48;
			*idxCRC=37+sizeDataBlock;
		}else{
			errorType= 1;//form error
		}
	}
	return errorType;
}

void concatenateBits_Id(uint8_t *bits, uint8_t *ctnBytes){
	int i=0;
	int j=0;
	int sizeStr=4;//sizeof(bits);
	//static uint8_t ctnBytes[]={0,0,0,0};

	for(i=0; i<sizeStr; i++){
		for(j=0;j<8;j++){
			ctnBytes[i] += ((bits[(i*8)+j]-48) << (7-j));
		}
	}
	//return ctnBytes;
}

void concatenateBits_Data(uint8_t *bits, uint8_t *ctnBytes){
	int i=0;
	int j=0;
	int sizeStr=8;//sizeof(bits);
	//static uint8_t ctnBytes[]={0,0,0,0,0,0,0,0};

	for(i=0; i<sizeStr; i++){
		for(j=0;j<8;j++){
			ctnBytes[i] += ((bits[(i*8)+j]-48) << (7-j));
		}
	}
	//return ctnBytes;
}

void concatenateBits_CRC(uint8_t *bits, uint16_t* ctnBytes){
	int i=0;
	int j=0;
	int sizeStr=15;//sizeof(bits);
	//static uint16_t ctnBytes=0;

	for(i=0; i<sizeStr; i++){
		ctnBytes += ((bits[i]-48) << (14-i));
	}
	//return ctnBytes;
}

uint8_t check_crc(uint8_t *bitString)
{
	int i=0;
	int j=0;
	uint8_t errorType = 0;
	uint16_t crcReceived[16] = {0};

	int sizeFrameClc = (int)strlen(bitString)-LENGHT_CRC;
	uint8_t crc_received[]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	memcpy(crc_received, &bitString[sizeFrameClc],LENGHT_CRC);
	concatenateBits_CRC(crc_received,crcReceived);
	//uint16_t crcReceived = concatenateBits_CRC(crc_received);
	uint16_t crc=0;

	uint8_t dataCrcArray[MAX_LENGHT_PURE_FRAME];
	for(i=0; i<MAX_LENGHT_PURE_FRAME; i++){
		dataCrcArray[i] = '\0';
	}
	memcpy(dataCrcArray, bitString,sizeFrameClc);

	/*for(i=0;i<sizeFrameClc;i++){
		uint8_t crc_next;
		crc_next = (dataCrcArray[i]-48) ^ ((crc >> 14) & 1);
		(crc) <<= 1;
		if (crc_next == 1) {
			(crc) = (crc ^ 0xc599) & 0x7fff;
		}
		//CRC_Count++;
	}*/

	crc = calcul_crc(dataCrcArray);	//works as an octo-crc

	if(crc!=crcReceived){
		errorType=1;
	}


	return errorType;
}

uint16_t calcul_crc(uint8_t *data)
{
	int i, j;
	uint16_t crc;
	crc = 0;

	uint8_t tempByte[25];
	for(i=0; i<25; i++){
		tempByte[i] = 0;
	}

	int sizeFrame = strlen(data);//sizeof(data);
	int sizeFrameCtn = sizeFrame/8;
	int nbrReste = sizeFrame%8;
	int nbrZerosToAdd = 0;

	//test for bit-string
	if(nbrReste > 0){
		sizeFrameCtn = sizeFrameCtn +1;
		nbrZerosToAdd = (sizeFrameCtn*8)-sizeFrame;
	}

	for (i = 0; i < sizeFrameCtn; i++) {
		j = ( (i==0) ? nbrZerosToAdd : 0 );
		for (j; j < 8; j++) {
			int indexBit = i*8+j-nbrZerosToAdd;
			if(data[indexBit]-'0' == 1){
				tempByte[i] += (1 << (7-j) );
			}
		}
	}

	for (i = 0; i < sizeFrameCtn; i++) {
		crc ^= ((uint16_t)tempByte[i] << 7);

		for (j = 0; j < 8; j++) {
			crc <<= 1;
			if (crc & 0x8000) {
				crc ^= 0xc599;
			}
		}
		crc = crc & 0x7fff;
	}

	return crc;
}
