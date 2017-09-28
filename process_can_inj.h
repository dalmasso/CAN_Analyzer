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
#ifndef __PROCESS_INJ
#define __PROCESS_INJ

#ifdef __cplusplus
 extern "C" {
#endif


 /**
  * \struct PROCESS_message_inj
  * \brief Object content the errors to apply.
  *
  * PROCESS_message_inj is the entire description of the CAN frame decoded.
  * Errors are being shown by a 1 value, 0 value is used if there are not error.
  */
  typedef struct
  {
 	 uint8_t ID[4];			/*!< Specifies the identification number.
                                     This parameter is a table of 4 bytes and can be set
                                     with 11 bits (standard mode) or 29 bits (extended mode) therefore the max value is 4 bytes
                                     This parameter is optional when the parameter Apply_Blackout_error is set to 1*/
 	 uint8_t DATA[8];		/*!< Specifies the data who are transmitted.
 	                                    This parameter is a table of 8 bytes and can be set
 	                                    with the data or by 0 if it is empty*/
 	 uint8_t RTR_value;		/*!< Specifies the type of FRAME.
 	 	                                When this parameter is set to 0, we are in DATA mode
              	  	  	  	  	  	  	When this parameter is set to 1, we are in REQUEST mode. There are no DATA*/
 	 uint8_t Apply_RTR_error;/*!< Specifies that an RTR error is to apply.
 	 	 	 	 	 	 	 	 	 	When this parameter is set to 0, RTR takes normal value (1 for request when no data and 0 when DATA
              	  	  	  	  	  	  	When this parameter is set to 1, the RTR value will be false*/
 	 uint8_t Apply_R0_error;/*!< Specifies that an R0 error is to apply.
 	 	 	 	 	 	 	 	 	 	When this parameter is set to 0, R0 = 0
              	  	  	  	  	  	  	When this parameter is set to 1, R0 = 1*/
 	 uint8_t DLC_value;		/*!< Specifies the DLC value.
 	                                    This parameter is the number of the DATA.
 	                                    If they are NO DATA, it's the number of DATA wait (request mode)*/
 	 uint8_t Apply_DLC_error;/*!< Specifies that an DLC error is to apply.
              	  	  	  	  	  	  	When this parameter is set to 1, DLC_value will be modified to be false*/
 	 uint8_t Apply_CRC_error;/*!< Specifies that a CRC error is to apply.
              	  	  	  	  	  	  	When this parameter is set to 1, CRC will be false*/
 	 uint8_t Apply_STUFF_error;/*!< Specifies that an STUFF error is to apply.
              	  	  	  	  	  	  	When this parameter is set to 1, BIT STUFFING will be false*/
 	 uint8_t Apply_Blackout_error;/*!< Specifies that an BLACKOUT error is to apply.
              	  	  	  	  	  	  	When this parameter is set to 1, the transmitter will be applied 0 during a frame*/

  } PROCESS_message_inj;



#define FALSE 	0X00
#define TRUE 	0X01

/* defined first in process.h
#define MAX_LENGHT_ROUGH_FRAME	190
#define MAX_LENGHT_PURE_FRAME	130*/

enum
{
	WAIT_,
	CREATE_PUR_FRAME,
	BLACKOUT_ERROR,
	RTR_ERROR,
	CRC_ERROR,
	R0_ERROR,
	DLC_ERROR,

	SEND_FRAME,

}PROCESS_STATE_INJ;

typedef struct
{
	uint8_t NB_DATA;	//Lenght of DATA
	uint8_t A_SOF;
	uint8_t A_ID;
	uint8_t A_RTR;
	uint8_t A_R0;
	uint8_t A_R1;
	uint8_t A_DLC;
	uint8_t A_DATA;
	uint8_t A_CRC;
	uint8_t A_CRC_DEL;
	uint8_t A_ACK;
	uint8_t A_DEL;
	uint8_t A_EOT;
	uint8_t A_INTER_T;
	uint8_t A_OUR_INTER_T;
}PROCESS_INJ_var;

void ProcessTaskInj(void *pvParameters);
void PROCESS_INJ_Scan_queue(PROCESS_INJ_var *,  PROCESS_message_inj *);
uint16_t *MakeCRC_INJ(uint8_t*);
uint16_t concatenateBits(uint8_t *);
void ADD_bitsStuffing(uint8_t*, PROCESS_INJ_var*);
void ADD_bitsStuffing_error(uint8_t*, PROCESS_INJ_var*);

#ifdef __cplusplus
}
#endif

#endif /* __PROCESS_INJ */
