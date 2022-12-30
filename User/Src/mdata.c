//
// FI 3JG Projekt 4 
// Inhalt: Berechnung der Messwerte, Plot der Messwerte
//
// Hardware: STM32F429I-Discovery  Board
// IDE:	MDK 4.72a
//
// Datum: 14-02-2017 scmi
// Initial Version

// Includes ------------------------------------------------------------------
#include "mdata.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "stm32f429i_discovery_lcd.h"

extern volatile int32_t SysTickCnt;

// Defines ------------------------------------------------------------------
// ...

// Functions
static int GenerateRandomNumber (int lowRange, int highRange);

// Berechnung der Messwerte
/**
 * @brief Generate sine-signal data and store it in parameters data1 and data2
 *
 * This function generates sine-wave signals with pseudo-random values for
 * amplitude, offset, frequency and phase shift. The resulting data is stored
 * in arrays data1 and data2. 
 *
 * @param data1 data of sine signal 1
 * @param data2 data of sine signal 2
 * @param nrOfData size of arrays data1 and data2.
 * @param deltaTimeMs in this variable the delay between 2 consecutive data points is returned
 * 			(pass by reference)
 *
 * @return number of generated data points
 */
int GetMeasurementData (float* data1, float* data2, int nrOfData, float* deltaTimeMs) {

	float offset_V1, offset_V2;
	float amplitude_V1, amplitude_V2;
	int frequenz_hz;
	float phi0_rad;
	int tgesamt_ms;
	float dt_ms;
	float t_ms;
	float omega;
	int i;

	// Zufallszahlengenerator neu initialisieren
	srand(HAL_GetTick());
	// Bereich f�r Offset1 data1 -10 ... +10
	if (SET_OFFSET_TO_ZERO) {
		offset_V1 = 0;
	} else {
		offset_V1 = 10 - GenerateRandomNumber (1,20);
	}
	// Bereich f�r Amplitude data1 0.01 ... 10
	amplitude_V1 = GenerateRandomNumber (1,100) / (float)10.0;

	// Bereich f�r Offset2 data2 -10 ... +10
	if (SET_OFFSET_TO_ZERO) {
		offset_V2 = 0;
	} else {
		offset_V2 = 10 - GenerateRandomNumber (1,20);
	}
	// Bereich f�r Amplitude data2 0.01 ... 10
	amplitude_V2 = GenerateRandomNumber (1,100) / (float)10.0;

	// Bereich f�r Frequenz 1 ... 150 Hz
	frequenz_hz = 50; // * GenerateRandomNumber (1,3);
	tgesamt_ms = 40;
	// Bereich f�r Phasenverschiebung 0 ... PI
	phi0_rad = GenerateRandomNumber (1,2000) / (float)636.6198;
	phi0_rad = phi0_rad - 1.5708f;

	dt_ms = (float)tgesamt_ms / (nrOfData-1);
	omega = 6.283 * frequenz_hz;

	for (i=0; i<nrOfData; i++ ) {
		t_ms = dt_ms * i;
		data1[i] = offset_V1 + amplitude_V1 * sin (((double)omega*(double)t_ms*0.001));
		data2[i] = offset_V2 + amplitude_V2 * sin (((double)omega*(double)t_ms*0.001+(double)phi0_rad));
	}

	*deltaTimeMs = dt_ms;

	return nrOfData;

}

/**
 * @brief Uses rand() to generate a pseudo random number in the range of lowRange and highRange
 *
 * @param lowRange low limit of range for pseudo random number
 * @param highRange high limit of range for pseudo random number
 *
 * @return pseudo random number
 */
static int GenerateRandomNumber (int lowRange, int highRange) {

	int number;

	number = lowRange + (rand() % (highRange - lowRange + 1));

	return number;
}

/**
 * @brief Plots up to three data arrays to the screen
 *
 * @param nrMData Number of arrays to plot, eg: value 1 means only data in array
 * data1 is plotted, value 3 results in all 3 data arrays to be plotted.
 * @param data1 Data points of signal 1
 * @param data2 Data points of signal 2
 * @param data3 Data points of signal 3
 * @param nrOfData size of data arrays
 */
void PlotData (int nrMData, float* data1, float* data2, float* data3,
		int nrOfData) {

	int i;
	int plotSizeX, plotSizeY;
	int plotOffsetX, plotOffsetY;
	int nrOfRasterX, nrOfRasterY;
	float skalaX, skalaY;
	float rasterX, rasterY;
	float maxh;
	float voltsPerDiv;
	int xCoord1, yCoord1, xCoord2, yCoord2;

	plotSizeX = 220;
	plotSizeY = 140;
	plotOffsetX = 10;
	plotOffsetY = 180;
	nrOfRasterY = 8;
	nrOfRasterX = 10;

	// Ausgabefl�che l�schen
	LCD_SetColors(LCD_COLOR_BLACK, LCD_COLOR_BLACK);
	LCD_FillRect(plotOffsetX, plotOffsetY - plotSizeY , plotSizeX, plotSizeY);

	// Ausgabe am Display
	// Rechteck ausgeben
	LCD_SetColors(LCD_COLOR_YELLOW, LCD_COLOR_BLACK);
	LCD_DrawRect(plotOffsetX, plotOffsetY - plotSizeY , plotSizeX, plotSizeY);

	// Raster f�r y(t)
	rasterY = (float)plotSizeY / (float) nrOfRasterY;
	xCoord1 = plotOffsetX;
	xCoord2 = plotOffsetX + plotSizeX;

	for (i=1; i<nrOfRasterY; i++) {
		yCoord1 = plotOffsetY - i*rasterY;
		yCoord2 = yCoord1;

		LCD_DrawLine(xCoord1, yCoord1, xCoord2, yCoord2);
	}

	//    Nulllinie darstellen (Zeitachse)
	yCoord1 = (float)plotOffsetY - (float)plotSizeY/(float)2.0;
	yCoord2 = yCoord1;
	LCD_DrawLine(xCoord1, yCoord1+1, xCoord2, yCoord2+1);
	LCD_DrawLine(xCoord1, yCoord1, xCoord2, yCoord2);
	LCD_DrawLine(xCoord1, yCoord1-1, xCoord2, yCoord2-1);

	// Raster f�r t
	rasterX = (float)plotSizeX / (float) nrOfRasterX;
	yCoord1 = plotOffsetY;
	yCoord2 = plotOffsetY - plotSizeY;

	for (i=1; i < nrOfRasterX; i++) {
		xCoord1 = plotOffsetX + i * rasterX;
		xCoord2 = xCoord1;

		// 3) Zeichnen mit LCD_DrawLine (...)
		LCD_DrawLine(xCoord1, yCoord1, xCoord2, yCoord2);
	}

	//    Nulllinie darstellen (y(t)-Achse)
	xCoord1 = (float)plotOffsetX + (float)plotSizeX/(float)2.0;
	xCoord2 = xCoord1;

	LCD_DrawLine(xCoord1-1, yCoord1, xCoord2-1, yCoord2);
	LCD_DrawLine(xCoord1, yCoord1, xCoord2, yCoord2);
	LCD_DrawLine(xCoord1+1, yCoord1, xCoord2+1, yCoord2);

	// Ermittlung V/DIV
	//    Min/Max bestimmen
	maxh = 0;
	for (i = 0; i < nrOfData; i++)
	{
		if (maxh < data1[i])
			maxh = data1[i];
		if (data2 != NULL && maxh < data2[i])
			maxh = data2[i];
		if (data3 != NULL && maxh < data3[i])
			maxh = data3[i];
	}

	//    V/DIV bestimmen (Bereich: 1mV .. 100 V)
	//    Achtung: keine Bereichspr�fung
	voltsPerDiv = 100.0;

	while ( (voltsPerDiv * (float)nrOfRasterY/(float)2.0) > (float) maxh) {
		voltsPerDiv = voltsPerDiv / (float)10.0;
	}
	voltsPerDiv = voltsPerDiv * (float)10.0;

	if ( (voltsPerDiv /(float)5.0 * (float)nrOfRasterY/(float)2.0) > (float) maxh) {
		voltsPerDiv = voltsPerDiv / (float)5.0;
	} else if ( (voltsPerDiv /(float)2.0 * (float)nrOfRasterY/(float)2.0) > (float) maxh) {
		voltsPerDiv = voltsPerDiv / (float)2.0;
	}

	// 1) Skalierungsfaktor f�r Darstellung berechnen
	skalaX = (float)plotSizeX / nrOfData/*(float)datatGesamtMs*/;
	skalaY = (float)plotSizeY/(float)(voltsPerDiv*nrOfRasterY);

	// 2) Koordinaten in der Darstellung mit Skalierungsfaktor berechnen

	if (nrMData >= 1) {
		LCD_SetColors(LCD_COLOR_BLUE, LCD_COLOR_BLACK);

		for (i=0;i<(nrOfData-1); i++) {
			xCoord1 = plotOffsetX + i/** dataDeltaMs*/* skalaX;
			yCoord1 = plotOffsetY - plotSizeY / 2.0 - data1[i] * skalaY;
			xCoord2 = plotOffsetX + (i+1)/** dataDeltaMs*/* skalaX;
			yCoord2 = plotOffsetY - plotSizeY / 2.0 - data1[i+1] * skalaY;

			// 3) Zeichnen mit LCD_DrawLine (...)
			LCD_DrawLine(xCoord1, yCoord1, xCoord2, yCoord2);
			LCD_DrawLine(xCoord1, yCoord1-2, xCoord2, yCoord2-2);
			LCD_DrawLine(xCoord1, yCoord1-1, xCoord2, yCoord2-1);
			LCD_DrawLine(xCoord1, yCoord1+1, xCoord2, yCoord2+1);
			LCD_DrawLine(xCoord1, yCoord1+2, xCoord2, yCoord2+2);
		}
	}

	if (nrMData >= 2) {

		LCD_SetColors(LCD_COLOR_RED, LCD_COLOR_BLACK);

		for (i=0;i<(nrOfData-1); i++) {
			xCoord1 = plotOffsetX + i/** dataDeltaMs*/* skalaX;
			yCoord1 = plotOffsetY - plotSizeY / 2.0 - data2[i] * skalaY;
			xCoord2 = plotOffsetX + (i+1)/** dataDeltaMs*/* skalaX;
			yCoord2 = plotOffsetY - plotSizeY / 2.0 - data2[i+1] * skalaY;

			// 3) Zeichnen mit LCD_DrawLine (...)
			LCD_DrawLine(xCoord1, yCoord1, xCoord2, yCoord2);
			LCD_DrawLine(xCoord1, yCoord1-2, xCoord2, yCoord2-2);
			LCD_DrawLine(xCoord1, yCoord1-1, xCoord2, yCoord2-1);
			LCD_DrawLine(xCoord1, yCoord1+1, xCoord2, yCoord2+1);
			LCD_DrawLine(xCoord1, yCoord1+2, xCoord2, yCoord2+2);
		}
	}

	if (nrMData >= 3) {

		LCD_SetColors(LCD_COLOR_MAGENTA, LCD_COLOR_BLACK);

		for (i=0;i<(nrOfData-1); i++) {
			xCoord1 = plotOffsetX + i/** dataDeltaMs*/* skalaX;
			yCoord1 = plotOffsetY - plotSizeY / 2.0 - data3[i] * skalaY;
			xCoord2 = plotOffsetX + (i+1)/** dataDeltaMs*/* skalaX;
			yCoord2 = plotOffsetY - plotSizeY / 2.0 - data3[i+1] * skalaY;

			// 3) Zeichnen mit LCD_DrawLine (...)
			LCD_DrawLine(xCoord1, yCoord1, xCoord2, yCoord2);
			LCD_DrawLine(xCoord1, yCoord1-2, xCoord2, yCoord2-2);
			LCD_DrawLine(xCoord1, yCoord1-1, xCoord2, yCoord2-1);
			LCD_DrawLine(xCoord1, yCoord1+1, xCoord2, yCoord2+1);
			LCD_DrawLine(xCoord1, yCoord1+2, xCoord2, yCoord2+2);
		}
	}

	LCD_SetColors(LCD_COLOR_YELLOW, LCD_COLOR_BLACK);
	LCD_SetFont(&Font8);
	LCD_SetPrintPosition(23,0);
	printf("  Volts/DIV :%5.2f V", voltsPerDiv);

	return;

}

