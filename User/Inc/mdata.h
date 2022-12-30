//
// FI 3JG Projekt 4b 
// Inhalt: Berechnung der Messwerte, Plot der Messwerte
//
// Hardware: STM32F429I-Discovery  Board
// IDE:	MDK 4.72a
//
// Datum: 14-02-2017 scmi
// Initial Version

#ifndef MDATA_H
#define MDATA_H

/* Defines -------------------------------------------------------------------*/
#define SET_OFFSET_TO_ZERO     1

/* Includes ------------------------------------------------------------------*/

int GetMeasurementData (float* data1,	      	// Array1 f�r Messwerte
		float* data2,       	// Array2 f�r Messwerte
		int nrOfData,	      	// Anzahl der Messwerte
		float* deltaTimeMs);  // Abtastzeit in ms

void PlotData          (int nrMData,          // Anzahl der Messreihen 
		float* data1,         // zu zeichnende Messreihe1
		float* data2,         // zu zeichnende Messreihe2
		float* data3,         // zu zeichnende Messreihe3
		int nrOfData);        // Anzahl der Messwerte
//                        int min, int max);    // Minimum, Maximum 

#endif // MDATA_H
