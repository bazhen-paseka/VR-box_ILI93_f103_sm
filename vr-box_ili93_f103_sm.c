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
	extern DMA_HandleTypeDef		hdma_usart2_rx ;

	Debug_struct	Debug_ch = { 0 }	;
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
	#define RX_BUFFER_SIZE		0xFF

	uint8_t				rx_circular_buffer[RX_BUFFER_SIZE] ;
	HAL_StatusTypeDef	status_res	= { 0 } 	;
	RingBuffer_DMA		rx_buffer	= { 0 }		;
	DMA_HandleTypeDef 	*hdma_rx_handler		;

	static	uint8_t 	length_u8			= 0 ;
			uint32_t 	pointer_u32			= 0 ;
			uint8_t		lcd_position_u8		= 0 ;
			uint8_t 	previous_row_u8		= 0 ;
			uint8_t 	previous_col_u8		= 0 ;
			int 		cursor_int 			= 0 ;
/*
**************************************************************************
*                        LOCAL FUNCTION PROTOTYPES
**************************************************************************
*/
	void Debug_print(	char				*_string 	) ;
	void Debug_init	(	UART_HandleTypeDef	*_huart		) ;
	void DrawCipher	(	uint8_t 			 _row_u8		,
						uint8_t				 _col_u8		,
						uint16_t 			 _color_cipher	,
						uint16_t 			 _color_rect	,
						uint16_t 			 _color_fill 	) ;
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
	HAL_Delay(100) ;
	LCD_FillScreen(ILI92_WHITE);
	for (int r = 0; r<4; r++) {
		for (int c = 0; c<4; c++) {
			DrawCipher(r, c, ILI92_GREEN, ILI92_MAGENTA, ILI92_WHITE);
		}
	}

	RingBuffer_DMA_Init ( &rx_buffer, 	&hdma_usart2_rx , rx_circular_buffer, RX_BUFFER_SIZE) ;  	// Start UART receive
	status_res = HAL_UART_Receive_DMA( 	&huart2, rx_circular_buffer, RX_BUFFER_SIZE ) ;  	// how many bytes in buffer
}
//***************************************************************************

void VRbox_Main (void) {
	#define	DEBUG_RC_SIZE		30
	char DebugStr[DEBUG_RC_SIZE] = { 0 } ;

//	sprintf(DebugStr," cntr %04u\r\n", (int)pointer_u32++) ;
//	HAL_UART_Transmit(&huart2, (uint8_t *)DebugStr, strlen(DebugStr), 100) ;

	//LCD_SetCursor(cursor_int, 100);

	while (1) {
		uint8_t DebugRC[DEBUG_RC_SIZE] = { 0 };
		uint32_t 	rx_count = RingBuffer_DMA_Count ( &rx_buffer ) ;
		while ( rx_count-- ) {
			DebugRC[length_u8] = RingBuffer_DMA_GetByte ( &rx_buffer ) ;
			length_u8++ ;
		}

		if (length_u8 >= 5 ) {

			sprintf(DebugStr, "%c%c%c%c%c", DebugRC[0], DebugRC[1], DebugRC[2], DebugRC[3], DebugRC[4] ) ;
			Debug_print( DebugStr ) ;

			DrawCipher(previous_row_u8, previous_col_u8, ILI92_GREEN, ILI92_MAGENTA, ILI92_WHITE  );

			uint8_t row_u8 = DebugRC[1]-0x30 ;
			uint8_t col_u8 = DebugRC[2]-0x30 ;

			if ( DebugRC[0] == 0x31 ) {
				DrawCipher(row_u8, col_u8, ILI92_MAGENTA, ILI92_LIGHTGRAY, ILI92_LIGHTGRAY );
			}
			if ( DebugRC[0] == 0x30 ) {
				DrawCipher(row_u8, col_u8, ILI92_WHITE, ILI92_BLACK, ILI92_BLACK );
			}

			previous_row_u8 = row_u8;
			previous_col_u8 = col_u8;
			length_u8 = 0 ;
		}
		if ( DebugRC[length_u8] == '\n' ) {
			length_u8 = 0 ;
		}
		HAL_Delay(10) ;
	}
}
//***************************************************************************

void DrawCipher (uint8_t _row_u8, uint8_t _col_u8, uint16_t _color_cipher, uint16_t _color_rect, uint16_t _color_fill ) {

	#define X_CENTER		160
	#define Y_CENTER		120

	#define	RECT_SIZE		25
	#define RECT_OFFSET		 1

	#define	FILL_SIZE		23
	#define FILL_OFFSET		 1

	#define	CURSOR_OFFSET	 7

	const 	char 	keyboard_char[4][4]	= { { '1', '2', '3', 'A' },
											{ '4', '5', '6', 'B' },
											{ '7', '8', '9', 'C' },
											{ '*', '0', '#', 'D' } } ;

	const uint32_t x_start_u32 = ( X_CENTER - 2*(RECT_OFFSET + RECT_SIZE + RECT_OFFSET) ) ; 	// = 106
	const uint32_t y_start_u32 = ( Y_CENTER - 2*(RECT_OFFSET + RECT_SIZE + RECT_OFFSET) ) ;		// =  66

	uint32_t x_coord_u32 = x_start_u32 + RECT_OFFSET + _col_u8 * ( RECT_SIZE + RECT_OFFSET ) + LCD_OFFSET - lcd_position_u8*2*(LCD_OFFSET) ;
	uint32_t y_coord_u32 = y_start_u32 + RECT_OFFSET + _row_u8 * ( RECT_SIZE + RECT_OFFSET ) ;

	LCD_DrawRect  ( x_coord_u32					, y_coord_u32              		, RECT_SIZE, RECT_SIZE, _color_rect ) ;
	LCD_FillRect  ( x_coord_u32 + FILL_OFFSET	, y_coord_u32 + FILL_OFFSET		, FILL_SIZE, FILL_SIZE, _color_fill ) ;
	LCD_SetCursor ( x_coord_u32 + CURSOR_OFFSET	, y_coord_u32 + CURSOR_OFFSET	) ;
	LCD_SetTextColor(_color_cipher, _color_fill);
	LCD_Printf("%c", keyboard_char[ _row_u8][ _col_u8]);
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

