/**
* \file process.h
* \brief process library.
* \author Guillaume H. and Vincent .J
* \version 0.1
* \date 14 June 2017
*
* Library for the Process tasks.
*
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PROCESS
#define __PROCESS

#ifdef __cplusplus
 extern "C" {
#endif

typedef struct
{
	unsigned char data[190];
}xMessage_data_brutTypeDef;

xMessage_data_brutTypeDef xMessage_data_brut;

#define FALSE 	0X00
#define TRUE 	0X01

#define MAX_LENGHT_ROUGH_FRAME	190 //70	//158
#define MAX_LENGHT_PURE_FRAME	190// 59	//130

#define NBR_BITS_OUT_OF_BITSTUFFING		13
#define LENGHT_CRC		15

enum
{
	WAIT,
	SIZE,
	BIT_STUFFING,
	GET_ID,
	CHECK_CRC,
	SEND,
}PROCESS_STATE;

/**
 * \struct PROCESS_MessageUARTTypeDef
 * \brief Object decoded CAN frame with error(s) in bytes.
 *
 * PROCESS_MessageTypeDef is the entire description of the CAN frame decoded.
 * Errors are being shown by a 1 value, 0 value is used if there are not error.
 */
typedef struct
{
	 char ID[4];			/*!< Specifies the identification number.
                                    This parameter is a table of 4 bytes and can be set
                                    with 11 bits (standard mode) or 29 bits (extended mode) therefore the max value is 4 bytes*/
	 char DLC;   				/*!< Specifies the lenght of the data who are transmitted.
	 	              	  	  	  	This parameter is a byte with only 4 bits usefull. */
	 char DATA[8];			/*!< Specifies the data who are transmitted.
                                    This parameter is a table of 8 bytes and can be set
                                    with the data or by 0 if it is empty,
                                    if there are error(s) it can be set with the data or by 0 value because it depended the type of error */
	 char RTR;				/*!< Specifies the type of data
	 	                            This parameter is a byte and can be set by 0 (represents some datas) or by 1 (represent a request)*/
	 char FormError;		/*!< Specifies the Form error
                                    This parameter is a byte and can be set by 0 if the error is false or by 1 if the error is true.
                                    A FORM ERROR has to be detected when a fixed-form bit field contains one or more illegal bits.
                                    */
	 char BitStuffError;	/*!< Specifies the Stuff error
                                    This parameter is a byte and can be set by 0 if the error is false or by 1 if the error is true.
                                    A STUFF ERROR has to be detected at the bit time of the 6th consecutive equal bit level in a message field
                                    that should be decoded by the method of bit stuffing.
                                    */
	 char CRCError;		/*!< Specifies the CRC error
                                    This parameter is a byte and can be set by 0 if the error is false or by 1 if the error is true.
                                    The receivers calculate the CRC in the same way as the transmitter. A CRC ERROR has to be detected,
                                    if the calculated result is not the same as that received in the CRC sequence.
                                    */
	 char ACKError;		/*!< Specifies the ACK error
                                    This parameter is a byte and can be set by 0 if the error is false or by 1 if the error is true.
                                    An ACKNOLEDGMENT ERROR has to be detected by the transmitter whenever it does not monitor a 'dominant' bit
                                    during the ACK SLOT. In the software, we are a sniffer therefore we can detected too this error.
                                    */
	 char RECNumber;		/*!< Specifies the REC counter error
                                    This parameter is a byte and can be set  between 0 and 255.
                                    The RECEIVE ERROR COUNT is the image of the CAN bus state.
                                    */

 } Process_Message;

void ProcessTask(void *pvParameters);
void PROCESS_INIT(Process_Message *PROCESS_MessageUART,uint8_t *_pureFrame,uint8_t *_roughFrame);

uint8_t unstuffed_bit(uint8_t*, uint8_t*, int);
uint8_t extract_id_dlc(Process_Message*, uint8_t*, uint8_t*, int*);
uint8_t check_crc(uint8_t*);

void concatenateBits_Id(uint8_t*, uint8_t*);
//uint8_t *concatenateBits_Id(uint8_t*);
void concatenateBits_Data(uint8_t*, uint8_t*);
void concatenateBits_DataCRC(uint8_t*, uint8_t*);
void concatenateBits_CRC(uint8_t*, uint16_t*);
uint16_t calcul_crc(uint8_t*);

#ifdef __cplusplus
}
#endif

#endif /* __PROCESS */
