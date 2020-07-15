/**
* \version 1.0
* \author bazhen.levkovets
* \date 2020
*************************************************************************************
* \copyright	Bazhen Levkovets
* \copyright	Brovary
* \copyright	Ukraine
*************************************************************************************
*/

	// 		GPIO to data bus
	// D0 -> PA9
	// D1 -> PA7		//	PC7 BAZHEN CHANGE TO PA7 and use #define NEW_BIT1
	// D2 -> PA10
	// D3 -> PB3
	// D4 -> PB5
	// D5 -> PB4
	// D6 -> PB10
	// D7 -> PA8

	// 		GPIO to control bus
	// RST	->	PB1  	// PC1 BAZHEN CHANGE TO PB1	 and use #define NEW_RST
	// CS	->	PB0		on board RED-LED
	// RS	->	PA4		(CD)
	// WR	->	PA1
	// RD	->	PA0
/*
**************************************************************************
*							INCLUDE FILES
**************************************************************************
*/

	#include "vr-box_ili93_f103_sm.h"

/*
**************************************************************************
*							LOCAL DEFINES
**************************************************************************
*/


/*
**************************************************************************
*							LOCAL CONSTANTS
**************************************************************************
*/


/*
**************************************************************************
*						    LOCAL DATA TYPES
**************************************************************************
*/


/*
**************************************************************************
*							  LOCAL TABLES
**************************************************************************
*/

/*
**************************************************************************
*								 MACRO'S
**************************************************************************
*/


/*
**************************************************************************
*						 LOCAL GLOBAL VARIABLES
**************************************************************************
*/

	uint32_t pointer_u32 = 0 ;

/*
**************************************************************************
*                        LOCAL FUNCTION PROTOTYPES
**************************************************************************
*/

/*
**************************************************************************
*                           GLOBAL FUNCTIONS
**************************************************************************
*/

void VRbox_Init (void) {

	#define	DEBUG_STRING_SIZE		300
	char DebugStr[DEBUG_STRING_SIZE];
	sprintf(DebugStr," START\r\n") ;
	HAL_UART_Transmit(&huart2, (uint8_t *)DebugStr, strlen(DebugStr), 100) ;

	#define STRING_LEFT  ( (uint32_t) 0x7466654C )
	#define STRING_RIGHT ( (uint32_t) 0x74676952 )

	sprintf(DebugStr," Flash read...   ") ;
	HAL_UART_Transmit(&huart2, (uint8_t *)DebugStr, strlen(DebugStr), 100) ;

	uint32_t flash_word_u32 = Flash_Read(MY_FLASH_PAGE_ADDR);
	sprintf(DebugStr," 0x%x; \n", (int)flash_word_u32) ;
	HAL_UART_Transmit(&huart2, (uint8_t *)DebugStr, strlen(DebugStr), 100) ;
	//LCD_Printf(" Rotation: \"%s\"; \n ", (char *)&flash_word_u32);
	sprintf(DebugStr," Rotation: \"%s\"; \n ", (char *)&flash_word_u32) ;
	HAL_UART_Transmit(&huart2, (uint8_t *)DebugStr, strlen(DebugStr), 100) ;

	LCD_Init();
	switch (flash_word_u32)	{
		 case 	STRING_LEFT :	LCD_SetRotation(3);		break;
		 case 	STRING_RIGHT:	LCD_SetRotation(1);		break;
		 default:				LCD_SetRotation(1);		break;
	}

	//LCD_Init();
	//LCD_SetRotation(1);
	LCD_FillScreen(ILI92_WHITE);
	LCD_SetTextColor(ILI92_GREEN, ILI92_WHITE);
	LCD_Printf("\n START 'VRGC-056th'\n ");
	LCD_Printf("for_debug USART2 on PA2 115200/8-N-1 \n");
	LCD_Printf(" Rotation: \"%s\"; \n ", (char *)&flash_word_u32);
}
//***************************************************************************

void VRbox_Main (void) {
	#define	DEBUG_STRING_SIZE		300
	char DebugStr[DEBUG_STRING_SIZE];
	sprintf(DebugStr," cntr %04u\r\n", (int)pointer_u32++) ;
	HAL_UART_Transmit(&huart2, (uint8_t *)DebugStr, strlen(DebugStr), 100) ;

	LCD_SetCursor(150, 100);
	LCD_Printf("%04d", pointer_u32 );

	uint8_t DebugRC[DEBUG_STRING_SIZE];
	HAL_UART_Receive(&huart2, (uint8_t *)DebugRC, 1, 100) ;
	sprintf(DebugStr," char:  %c\r\n", (int)DebugRC) ;
	HAL_UART_Transmit(&huart2, (uint8_t *)DebugStr, strlen(DebugStr), 100) ;
	HAL_Delay(1000) ;
}
//***************************************************************************


//***************************************************************************



/*
**************************************************************************
*                           LOCAL FUNCTIONS
**************************************************************************
*/
