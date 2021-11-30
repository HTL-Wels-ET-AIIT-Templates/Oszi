//
// FI3JG Projekt 4b 
// Inhalt: Leistungsschwingung
//
// Hardware: STM32F429I-Discovery  Board
// IDE:	MDK 4.72a
//
// Datum: 14-02-2017 breb
// Initial Version

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <limits.h>

#include "stm32f4xx.h"
#include "stm32f429i_discovery.h"
#include "stm32f429i_discovery_lcd.h"
#include "mdata.h"

/* Defines ------------------------------------------------------------------*/
#define NR_OF_DATA 256

/* Strukturen -----------------------------------------------------------------*/
// TODO: Struktur deklarieren fuer berechnete Ergebnisse

/* Variables ------------------------------------------------------------------*/
volatile int32_t SysTickCnt=0;

/* Functions -------------------------------------------------------------------*/
void SysTick_Handler(void) {	// wird durch den System-Timer alle 10 ms aufgerufen
	// siehe Funktion: SysTick_Config()
	SysTickCnt++;
}

static int GetUserButtonPressed () {
	return (GPIOA->IDR & 0x0001);
}	

int main(void) {

	//ToDo lokale Variablen anlegen

	RCC_ClocksTypeDef 	Clocks;

	RCC_GetClocksFreq(&Clocks);
	SysTick_Config( Clocks.HCLK_Frequency/100 - 1 );	// 100 Hz ( T=10ms)
	// Inline-Funktion siehe core_cm4.h (ARM-Grundmodul)

	LCD_Init();					// LCD initiatization
	LCD_LayerInit(); 		// LCD Layer initiatization
	LTDC_Cmd(ENABLE);  	// Enable the LTDC 
	LCD_SetLayer(LCD_FOREGROUND_LAYER);	// Set LCD foreground layer

	LCD_SetFont(&Font8x12);					// line: 0...39	, column: 0...29
	LCD_Clear(LCD_COLOR_BLUE);
	LCD_SetColors(LCD_COLOR_YELLOW, LCD_COLOR_BLUE); // TextColor,BackColor
	LCD_SetPrintPosition(0,9);  // line 1, column 0
	printf("HTBLA-WELS");
	LCD_SetPrintPosition(1,7);  // line 2, column 0	
	printf("Fischergasse 30");
	LCD_SetPrintPosition(2,9);  // line 3, column 0	
	printf("A-4600 Wels");

	LCD_SetFont(&Font8x8);					// line: 0...39	, column: 0...29
	LCD_SetPrintPosition(39,0);			// line 39, column 0
	LCD_SetColors(LCD_COLOR_MAGENTA, LCD_COLOR_BLUE); // TextColor,BackColor
	printf("(c) Bernhard Breinbauer");
	PlotData(0, NULL, NULL, NULL, 0); 

	while (1) {

		if (GetUserButtonPressed()) {
			LCD_Clear(LCD_COLOR_BLUE);

			//ToDo: Messablauf ausf�hren



		}
	} // while
} // main


