/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * Description of project
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "main.h"
#include "stm32f429i_discovery_lcd.h"
#include "stm32f429i_discovery_ts.h"
#include "ts_calibration.h"
#include "mdata.h"

/* Private define ------------------------------------------------------------*/
#define NR_OF_DATA 256

/* Private includes ----------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
static int GetUserButtonPressed(void);
static int GetTouchState (int *xCoord, int *yCoord);

/**
 * @brief This function handles System tick timer.
 */
void SysTick_Handler(void)
{
	HAL_IncTick();
}

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {

	//ToDo declare local variables

	/* MCU Configuration--------------------------------------------------------*/
	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();
	/* Configure the system clock */
	SystemClock_Config();

	/* Initialize LCD and touch screen */
	LCD_Init();
	TS_Init(LCD_GetXSize(), LCD_GetYSize());
	/* touch screen calibration */
	//	TS_Calibration();

	LCD_SetFont(&Font12);					// line: 0...39	, column: 0...29
	LCD_Clear(LCD_COLOR_BLACK);
	LCD_SetColors(LCD_COLOR_YELLOW, LCD_COLOR_BLACK); // TextColor,BackColor
	LCD_SetPrintPosition(0,13);  // line 1, column 0
	printf("HTBLA-WELS");
	LCD_SetPrintPosition(1,11);  // line 2, column 0
	printf("Fischergasse 30");
	LCD_SetPrintPosition(2,13);  // line 3, column 0
	printf("A-4600 Wels");

	LCD_SetFont(&Font8);
	LCD_SetColors(LCD_COLOR_MAGENTA, LCD_COLOR_BLACK); // TextColor, BackColor
	LCD_DisplayStringAtLineMode(39, "copyright xyz", CENTER_MODE);

	PlotData(0, NULL, NULL, NULL, 0); 

	//ToDo: Initialisierung

	while (1) {

		if (GetUserButtonPressed()) {
			LCD_Clear(LCD_COLOR_BLACK);

			//ToDo: measure and display data

		}
	} // while
} // main

/**
 * Check if User Button has been pressed
 * @param none
 * @return 1 if user button input (PA0) is high
 */
static int GetUserButtonPressed(void) {
	return (GPIOA->IDR & 0x0001);
}

/**
 * Check if touch interface has been used
 * @param xCoord x coordinate of touch event in pixels
 * @param yCoord y coordinate of touch event in pixels
 * @return 1 if touch event has been detected
 */
static int GetTouchState (int* xCoord, int* yCoord) {
	void    BSP_TS_GetState(TS_StateTypeDef *TsState);
	TS_StateTypeDef TsState;
	int touchclick = 0;

	TS_GetState(&TsState);
	if (TsState.TouchDetected) {
		*xCoord = TsState.X;
		*yCoord = TsState.Y;
		touchclick = 1;
		if (TS_IsCalibrationDone()) {
			*xCoord = TS_Calibration_GetX(*xCoord);
			*yCoord = TS_Calibration_GetY(*yCoord);
		}
	}

	return touchclick;
}


