/**
* \version 1.0
* \author Bazhen Levkovets
* \date 2020
*************************************************************************************
* \copyright	Bazhen Levkovets
* \copyright	Brovary
* \copyright	Ukraine
*************************************************************************************
*/

#ifndef VRBOX_ILI93_F103_SM_INCLUDED_H_
#define VRBOX_ILI93_F103_SM_INCLUDED_H_

/*
**************************************************************************
*								INCLUDE FILES
**************************************************************************
*/
	#include <string.h>
	#include <stdio.h>

	#include "usart.h"
	#include "main.h"
	#include "dma.h"
	#include "usart.h"

	#include "vrbox_local_config.h"
	#include "lcd.h"
	#include "flash_stm32f103_hal_sm.h"
	#include "ringbuffer_dma.h"
	#include "stm32f1xx_hal_dma.h"

/*
**************************************************************************
*								    DEFINES                     
**************************************************************************
*/

/*
**************************************************************************
*								   DATA TYPES
**************************************************************************
*/
	typedef struct	{
		UART_HandleTypeDef	*uart_debug	;
	} 		Debug_struct				;

	typedef struct	{
		DMA_HandleTypeDef	*dma_usart	;
	} 		VRbox_struct				;

//***********************************************************

/*
**************************************************************************
*								GLOBAL VARIABLES
**************************************************************************
*/

/*
**************************************************************************
*									 MACRO'S                     
**************************************************************************
*/

/*
**************************************************************************
*                              FUNCTION PROTOTYPES
**************************************************************************
*/

	void VRbox_Init (void) ;
	void VRbox_Main (void) ;


#endif /* VRBOX_ILI93_F103_SM_INCLUDED_H_ */
