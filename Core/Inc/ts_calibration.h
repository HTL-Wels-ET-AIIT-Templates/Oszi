/**
 ******************************************************************************
 * @file    ts_calibration.h
 * @brief   Header for touch screen calibration
 */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TS_CALIBRATION_H
#define __TS_CALIBRATION_H

#include <stdint.h>

/* Exported functions ------------------------------------------------------- */
void TS_Calibration (void);
uint16_t TS_Calibration_GetX(uint16_t x);
uint16_t TS_Calibration_GetY(uint16_t y);
uint8_t TS_IsCalibrationDone(void);

#endif /* __TS_CALIBRATION_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
