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
	extern DMA_HandleTypeDef		hdma_usart2_rx ;

/*
**************************************************************************
*							LOCAL DEFINES
**************************************************************************
*/

	#define LCD_OFFSET		(20)
	#define STRING_LEFT  ( (uint32_t) 0x7466654C )
	#define STRING_RIGHT ( (uint32_t) 0x74676952 )

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

	Debug_struct 			Debug_ch				= { 0 }	;

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
	#define RX_BUFFER_SIZE 			0xFF
	uint8_t				rx_circular_buffer[RX_BUFFER_SIZE] ;
	HAL_StatusTypeDef	status_res	= { 0 } ;
	RingBuffer_DMA		rx_buffer	= { 0 }	;
	//DMA_HandleTypeDef * hdma_rx_handler			;
	int length_int = 0 ;
	uint8_t lcd_position_u8 = 0;
	int cursor_int = 0;
	uint8_t previous_drav_cipher_u8 = 0;

/*
**************************************************************************
*                        LOCAL FUNCTION PROTOTYPES
**************************************************************************
*/

	void Debug_print( char * _string ) ;
	void Debug_init( UART_HandleTypeDef * _huart ) ;

	void DrawCipher (uint8_t _cipher, uint16_t _color_cipher, uint16_t _color_rect, uint16_t _color_fill ) ;

/*
**************************************************************************
*                           GLOBAL FUNCTIONS
**************************************************************************
*/

void VRbox_Init (void) {

	Debug_init( &huart2 ) ;

	#define	DEBUG_STRING_SIZE		300
	char debugString[DEBUG_STRING_SIZE];
	sprintf(debugString," START\r\n") ;
	Debug_print( debugString ) ;

	sprintf(debugString," Flash read..  ") ;
	Debug_print( debugString ) ;

	uint32_t flash_word_u32 = Flash_Read(MY_FLASH_PAGE_ADDR);
	sprintf(debugString," 0x%x; \r\n", (int)flash_word_u32) ;
	Debug_print( debugString ) ;

	snprintf(debugString, 10, "Rot: %s", (char *)&flash_word_u32) ;
	Debug_print( debugString ) ;

	LCD_Init();
	switch (flash_word_u32)	{
		 case 	STRING_LEFT :	{ LCD_SetRotation(3) ; lcd_position_u8 = 0 ; }	break;		//	 left: + LCD_OFFSET
		 case 	STRING_RIGHT:	{ LCD_SetRotation(1) ; lcd_position_u8 = 1 ; }	break;		//	right: - LCD_OFFSET
		 default:				{ LCD_SetRotation(1) ; lcd_position_u8 = 0 ; }	break;
	}

	cursor_int = 100 + (LCD_OFFSET) - ( lcd_position_u8 * 2 * (LCD_OFFSET) ) ;

	LCD_FillScreen(ILI92_WHITE);
	LCD_SetTextColor(ILI92_GREEN, ILI92_WHITE);
	LCD_Printf("\n START 'VRGC-056th'\n ");
	LCD_Printf("for_debug: USART2 on PA2 115200/8-N-1 \n");

	int		soft_version_arr_int[3] = {0} ;
	soft_version_arr_int[0] 	= ((SOFT_VERSION) / 100) %10 ;
	soft_version_arr_int[1] 	= ((SOFT_VERSION) /  10) %10 ;
	soft_version_arr_int[2] 	= ((SOFT_VERSION)      ) %10 ;
	sprintf (	debugString						,
				" 2020-June-21 v%d.%d.%d. \r\n"	,
				soft_version_arr_int[0]			,
				soft_version_arr_int[1]			,
				soft_version_arr_int[2]			) ;
	Debug_print( debugString ) ;
	LCD_Printf( "%s" , debugString ) ;

	snprintf(debugString, 11, " Rot: %s", (char *)&flash_word_u32) ;
	LCD_Printf(debugString);
	HAL_Delay(500) ;
	LCD_FillScreen(ILI92_WHITE);
	for (int i = 0x00; i<=0x0F; i++) {
		DrawCipher(i, ILI92_GREEN, ILI92_MAGENTA, ILI92_WHITE);
		HAL_Delay(200) ;
	}

	RingBuffer_DMA_Init ( &rx_buffer, &hdma_usart2_rx, rx_circular_buffer, RX_BUFFER_SIZE) ;  	// Start UART receive
	status_res = HAL_UART_Receive_DMA(	&huart2, rx_circular_buffer, RX_BUFFER_SIZE ) ;  	// how many bytes in buffer
}
//***************************************************************************

void VRbox_Main (void) {
	#define	DEBUG_STRING_SIZE		300
	char DebugStr[DEBUG_STRING_SIZE];
	sprintf(DebugStr," cntr %04u\r\n", (int)pointer_u32++) ;
	HAL_UART_Transmit(&huart2, (uint8_t *)DebugStr, strlen(DebugStr), 100) ;

	LCD_SetCursor(cursor_int, 100);

	while (1) {
		uint8_t DebugRC[DEBUG_STRING_SIZE] = { 0 };
		uint32_t 	rx_count = RingBuffer_DMA_Count ( &rx_buffer ) ;
		while ( rx_count-- ) {
			DebugRC[length_int] = RingBuffer_DMA_GetByte ( &rx_buffer ) ;
			length_int++ ;
		}

		if (length_int >0 ) {
			snprintf(DebugStr, length_int+1, "%c", (int)DebugRC[0]) ;
			Debug_print( DebugStr ) ;

			DrawCipher(previous_drav_cipher_u8, ILI92_GREEN, ILI92_MAGENTA, ILI92_WHITE  );

			uint8_t current_drav_cipher_u8 = 0;

			if ((DebugRC[0] >= 0x30 ) && (DebugRC[0] <= 0x39)) {
				current_drav_cipher_u8 = DebugRC[0]-0x30;
			} else if (DebugRC[0] > 0x60) {
				current_drav_cipher_u8 = DebugRC[0]-0x57;
			}

			if ( DebugRC[1] == 0x32 ) {
				DrawCipher(current_drav_cipher_u8, ILI92_MAGENTA, ILI92_LIGHTGRAY, ILI92_LIGHTGRAY );
			}
			if ( DebugRC[1] == 0x31 ) {
				DrawCipher(current_drav_cipher_u8, ILI92_WHITE, ILI92_BLACK, ILI92_BLACK );
			}

			previous_drav_cipher_u8 = current_drav_cipher_u8;
		}
		length_int = 0 ;
		HAL_Delay(50) ;
	}
}

//***************************************************************************

void DrawCipher (uint8_t _cipher, uint16_t _color_cipher, uint16_t _color_rect, uint16_t _color_fill ) {
	LCD_SetTextColor(_color_cipher, _color_fill);


	#define X_CENTER		160
	#define Y_CENTER		120

	#define X_START			(160-27-27) = 106
	#define Y_START			(120-27-27)	=  66

	#define	RECT_SIZE		25
	#define RECT_OFFSET		 1

	#define	FILL_SIZE		23
	#define FILL_OFFSET		 1

	#define	CURSOR_OFFSET	 7

	switch (_cipher) {
		case 0x00: {
			LCD_DrawRect((134+(LCD_OFFSET)-(lcd_position_u8*2*(LCD_OFFSET))), 148, 25, 25, _color_rect);
			LCD_FillRect((134+1+(LCD_OFFSET)-(lcd_position_u8*2*(LCD_OFFSET))), 148+1, 23, 23, _color_fill);
			LCD_SetCursor((7+134+(LCD_OFFSET)-(lcd_position_u8*2*(LCD_OFFSET))), 7+148 );
			LCD_Printf("0");
		} break;

		case 0x01: {
			LCD_DrawRect((107+(LCD_OFFSET)-(lcd_position_u8*2*(LCD_OFFSET))), 67, 25, 25, _color_rect);
			LCD_FillRect((107+1+(LCD_OFFSET)-(lcd_position_u8*2*(LCD_OFFSET))), 67+1, 23, 23, _color_fill);
			LCD_SetCursor((7+107+(LCD_OFFSET)-(lcd_position_u8*2*(LCD_OFFSET))), 7+67 );
			LCD_Printf("1");
		} break;

		case 0x02: {
			LCD_DrawRect((134+(LCD_OFFSET)-(lcd_position_u8*2*(LCD_OFFSET))), 67, 25, 25, _color_rect);
			LCD_FillRect((134+1+(LCD_OFFSET)-(lcd_position_u8*2*(LCD_OFFSET))), 67+1, 23, 23, _color_fill);
			LCD_SetCursor((7+134+(LCD_OFFSET)-(lcd_position_u8*2*(LCD_OFFSET))), 7+67 );
			LCD_Printf("2");
		} break;

		case 0x03: {
			LCD_DrawRect((161+(LCD_OFFSET)-(lcd_position_u8*2*(LCD_OFFSET))), 67, 25, 25, _color_rect);
			LCD_FillRect((161+1+(LCD_OFFSET)-(lcd_position_u8*2*(LCD_OFFSET))), 67+1, 23, 23, _color_fill);
			LCD_SetCursor((7+161+(LCD_OFFSET)-(lcd_position_u8*2*(LCD_OFFSET))), 7+67 );
			LCD_Printf("3");
		} break;

		case 0x04: {
			LCD_DrawRect((107+(LCD_OFFSET)-(lcd_position_u8*2*(LCD_OFFSET))), 94, 25, 25, _color_rect);
			LCD_FillRect((107+1+(LCD_OFFSET)-(lcd_position_u8*2*(LCD_OFFSET))), 94+1, 23, 23, _color_fill);
			LCD_SetCursor((7+107+(LCD_OFFSET)-(lcd_position_u8*2*(LCD_OFFSET))), 7+94 );
			LCD_Printf("4");
		} break;

		case 0x05: {
			LCD_DrawRect((134+(LCD_OFFSET)-(lcd_position_u8*2*(LCD_OFFSET))), 94, 25, 25, _color_rect);
			LCD_FillRect((134+1+(LCD_OFFSET)-(lcd_position_u8*2*(LCD_OFFSET))), 94+1, 23, 23, _color_fill);
			LCD_SetCursor((7+134+(LCD_OFFSET)-(lcd_position_u8*2*(LCD_OFFSET))), 7+94 );
			LCD_Printf("5");
		} break;

		case 0x06: {
			LCD_DrawRect((161+(LCD_OFFSET)-(lcd_position_u8*2*(LCD_OFFSET))), 94, 25, 25, _color_rect);
			LCD_FillRect((161+1+(LCD_OFFSET)-(lcd_position_u8*2*(LCD_OFFSET))), 94+1, 23, 23, _color_fill);
			LCD_SetCursor((7+161+(LCD_OFFSET)-(lcd_position_u8*2*(LCD_OFFSET))), 7+94 );
			LCD_Printf("6");
		} break;

		case 0x07: {
			LCD_DrawRect((107+(LCD_OFFSET)-(lcd_position_u8*2*(LCD_OFFSET))), 121, 25, 25, _color_rect);
			LCD_FillRect((107+1+(LCD_OFFSET)-(lcd_position_u8*2*(LCD_OFFSET))), 121+1, 23, 23, _color_fill);
			LCD_SetCursor((7+107+(LCD_OFFSET)-(lcd_position_u8*2*(LCD_OFFSET))), 7+121 );
			LCD_Printf("7");
		} break;

		case 0x08: {
			LCD_DrawRect((134+(LCD_OFFSET)-(lcd_position_u8*2*(LCD_OFFSET))), 121, 25, 25, _color_rect);
			LCD_FillRect((134+1+(LCD_OFFSET)-(lcd_position_u8*2*(LCD_OFFSET))), 121+1, 23, 23, _color_fill);
			LCD_SetCursor((7+134+(LCD_OFFSET)-(lcd_position_u8*2*(LCD_OFFSET))), 7+121 );
			LCD_Printf("8");
		} break;

		case 0x09: {
			LCD_DrawRect((161+(LCD_OFFSET)-(lcd_position_u8*2*(LCD_OFFSET))), 121, 25, 25, _color_rect);
			LCD_FillRect((161+1+(LCD_OFFSET)-(lcd_position_u8*2*(LCD_OFFSET))), 121+1, 23, 23, _color_fill);
			LCD_SetCursor((7+161+(LCD_OFFSET)-(lcd_position_u8*2*(LCD_OFFSET))), 7+121 );
			LCD_Printf("9");
		} break;

		case 0x0A: {
			LCD_DrawRect((188+(LCD_OFFSET)-(lcd_position_u8*2*(LCD_OFFSET))), 67, 25, 25, _color_rect);
			LCD_FillRect((188+1+(LCD_OFFSET)-(lcd_position_u8*2*(LCD_OFFSET))), 67+1, 23, 23, _color_fill);
			LCD_SetCursor((7+188+(LCD_OFFSET)-(lcd_position_u8*2*(LCD_OFFSET))), 7+67 );
			LCD_Printf("A");
		} break;

		case 0x0B: {
			LCD_DrawRect((188+(LCD_OFFSET)-(lcd_position_u8*2*(LCD_OFFSET))), 94, 25, 25, _color_rect);
			LCD_FillRect((188+1+(LCD_OFFSET)-(lcd_position_u8*2*(LCD_OFFSET))), 94+1, 23, 23, _color_fill);
			LCD_SetCursor((7+188+(LCD_OFFSET)-(lcd_position_u8*2*(LCD_OFFSET))), 7+94 );
			LCD_Printf("B");
		} break;

		case 0x0C: {
			LCD_DrawRect((188+(LCD_OFFSET)-(lcd_position_u8*2*(LCD_OFFSET))), 121, 25, 25, _color_rect);
			LCD_FillRect((188+1+(LCD_OFFSET)-(lcd_position_u8*2*(LCD_OFFSET))), 121+1, 23, 23, _color_fill);
			LCD_SetCursor((7+188+(LCD_OFFSET)-(lcd_position_u8*2*(LCD_OFFSET))), 7+121 );
			LCD_Printf("C");
		} break;

		case 0x0D: {
			LCD_DrawRect((188+(LCD_OFFSET)-(lcd_position_u8*2*(LCD_OFFSET))), 148, 25, 25, _color_rect);
			LCD_FillRect((188+1+(LCD_OFFSET)-(lcd_position_u8*2*(LCD_OFFSET))), 148+1, 23, 23, _color_fill);
			LCD_SetCursor((7+188+(LCD_OFFSET)-(lcd_position_u8*2*(LCD_OFFSET))), 7+148 );
			LCD_Printf("D");
		} break;

		case 0x0E: {
			LCD_DrawRect((107+(LCD_OFFSET)-(lcd_position_u8*2*(LCD_OFFSET))), 148, 25, 25, _color_rect);
			LCD_FillRect((107+1+(LCD_OFFSET)-(lcd_position_u8*2*(LCD_OFFSET))), 148+1, 23, 23, _color_fill);
			LCD_SetCursor((7+107+(LCD_OFFSET)-(lcd_position_u8*2*(LCD_OFFSET))), 7+148 );
			LCD_Printf("*");
		} break;

		case 0x0F: {
			LCD_DrawRect((161+(LCD_OFFSET)-(lcd_position_u8*2*(LCD_OFFSET))), 148, 25, 25, _color_rect);
			LCD_FillRect((161+1+(LCD_OFFSET)-(lcd_position_u8*2*(LCD_OFFSET))), 148+1, 23, 23, _color_fill);
			LCD_SetCursor((7+161+(LCD_OFFSET)-(lcd_position_u8*2*(LCD_OFFSET))), 7+148 );
			LCD_Printf("#");
		} break;

		default :	{
			LCD_DrawRect((134+(LCD_OFFSET)-(lcd_position_u8*2*(LCD_OFFSET))), 94, 25, 25, _color_rect);
			LCD_FillRect((134+1+(LCD_OFFSET)-(lcd_position_u8*2*(LCD_OFFSET))), 94+1, 23, 23, _color_fill);
			LCD_SetCursor((7+134+(LCD_OFFSET)-(lcd_position_u8*2*(LCD_OFFSET))), 7+94 );
			LCD_Printf("X");
		}	break;
	}
}
//***************************************************************************

/*
**************************************************************************
*                           LOCAL FUNCTIONS
**************************************************************************
*/

void Debug_print( char * _string ) {
	size_t len_szt = strlen( _string );
	if (len_szt > DEBUG_STRING_SIZE) {
		len_szt = DEBUG_STRING_SIZE ;
	}
	HAL_UART_Transmit(	Debug_ch.uart_debug		,
						(uint8_t * ) _string	,
						len_szt					,
						100						) ;
}
//***********************************************************

void Debug_init( UART_HandleTypeDef * _huart ) {
	Debug_ch.uart_debug = _huart ;
}
//*********************************************************

