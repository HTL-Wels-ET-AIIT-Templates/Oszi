/**
  ******************************************************************************
  * @file    stm32f429i_discovery_lcd.c
  * @author  MCD Application Team
  * @brief   This file includes the LCD driver for ILI9341 Liquid Crystal 
  *          Display Modules of STM32F429I-Discovery kit (MB1075).
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2017 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* File Info : -----------------------------------------------------------------
                                   User NOTES
1. How To use this driver:
--------------------------
   - This driver is used to drive directly an LCD TFT using LTDC controller.
   - This driver select dynamically the mounted LCD, ILI9341 240x320 LCD mounted 
     on MB1075B discovery board, and use the adequate timing and setting for 
	 the specified LCD using device ID of the ILI9341 mounted on MB1075B discovery board           

2. Driver description:
---------------------
  + Initialization steps :
     o Initialize the LCD using the LCD_Init() function.
     o Apply the Layer configuration using LCD_LayerDefaultInit() function    
     o Select the LCD layer to be used using LCD_SelectLayer() function.
     o Enable the LCD display using LCD_DisplayOn() function.

  + Options
     o Configure and enable the color keying functionality using LCD_SetColorKeying()
       function.
     o Modify in the fly the transparency and/or the frame buffer address
       using the following functions :
       - LCD_SetTransparency()
       - LCD_SetLayerAddress() 
  
  + Display on LCD
      o Clear the hole LCD using LCD_Clear() function or only one specified string
        line using LCD_ClearStringLine() function.
      o Display a character on the specified line and column using LCD_DisplayChar()
        function or a complete string line using LCD_DisplayStringAtLine() function.
      o Display a string line on the specified position (x,y in pixel) and align mode
        using LCD_DisplayStringAtLine() function.          
      o Draw and fill a basic shapes (dot, line, rectangle, circle, ellipse, .. bitmap) 
        on LCD using the available set of functions     
 
------------------------------------------------------------------------------*/

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>

#include "stm32f429i_discovery_lcd.h"
#include "../../../Utilities/Fonts/fonts.h"
#include "../../../Utilities/Fonts/font24.c"
#include "../../../Utilities/Fonts/font20.c"
#include "../../../Utilities/Fonts/font16.c"
#include "../../../Utilities/Fonts/font12.c"
#include "../../../Utilities/Fonts/font8.c"

/** @addtogroup BSP
  * @{
  */ 

/** @addtogroup STM32F429I_DISCOVERY
  * @{
  */
    
/** @defgroup STM32F429I_DISCOVERY_LCD STM32F429I DISCOVERY LCD
  * @brief This file includes the LCD driver for (ILI9341) 
  * @{
  */ 

/** @defgroup STM32F429I_DISCOVERY_LCD_Private_TypesDefinitions STM32F429I DISCOVERY LCD Private TypesDefinitions
  * @{
  */ 
/**
  * @}
  */ 

/** @defgroup STM32F429I_DISCOVERY_LCD_Private_Defines STM32F429I DISCOVERY LCD Private Defines
  * @{
  */
#define POLY_X(Z)              ((int32_t)((Points + Z)->X))
#define POLY_Y(Z)              ((int32_t)((Points + Z)->Y))
/**
  * @}
  */ 

/** @defgroup STM32F429I_DISCOVERY_LCD_Private_Macros STM32F429I DISCOVERY LCD Private Macros
  * @{
  */
#define ABS(X)  ((X) > 0 ? (X) : -(X))
/**
  * @}
  */ 
  
/** @defgroup STM32F429I_DISCOVERY_LCD_Private_Variables STM32F429I DISCOVERY LCD Private Variables
  * @{
  */ 
LTDC_HandleTypeDef  LtdcHandler;
static DMA2D_HandleTypeDef Dma2dHandler;
static RCC_PeriphCLKInitTypeDef  PeriphClkInitStruct;

/* Default LCD configuration with LCD Layer 1 */
static uint32_t ActiveLayer = 0;
static LCD_DrawPropTypeDef DrawProp[MAX_LAYER_NUMBER];
LCD_DrvTypeDef  *LcdDrv;
/**
  * @}
  */ 

/** @defgroup STM32F429I_DISCOVERY_LCD_Private_FunctionPrototypes STM32F429I DISCOVERY LCD Private FunctionPrototypes
  * @{
  */ 
static void DrawChar(uint16_t Xpos, uint16_t Ypos, const uint8_t *c);
static void FillBuffer(uint32_t LayerIndex, void *pDst, uint32_t xSize, uint32_t ySize, uint32_t OffLine, uint32_t ColorIndex);
static void ConvertLineToARGB8888(void *pSrc, void *pDst, uint32_t xSize, uint32_t ColorMode);
/**
  * @}
  */ 

/** @defgroup STM32F429I_DISCOVERY_LCD_Private_Functions STM32F429I DISCOVERY LCD Private Functions
  * @{
  */ 

/**
  * @brief  Initializes the LCD.
  * @retval LCD state
  */
uint8_t LCD_Init(void)
{ 
  /* On STM32F429I-DISCO, it is not possible to read ILI9341 ID because */
  /* PIN EXTC is not connected to VDD and then LCD_READ_ID4 is not accessible. */
  /* In this case, ReadID function is bypassed.*/  
  /*if(ili9341_drv.ReadID() == ILI9341_ID)*/

    /* LTDC Configuration ----------------------------------------------------*/
    LtdcHandler.Instance = LTDC;
    
    /* Timing configuration  (Typical configuration from ILI9341 datasheet)
          HSYNC=10 (9+1)
          HBP=20 (29-10+1)
          ActiveW=240 (269-20-10+1)
          HFP=10 (279-240-20-10+1)
    
          VSYNC=2 (1+1)
          VBP=2 (3-2+1)
          ActiveH=320 (323-2-2+1)
          VFP=4 (327-320-2-2+1)
      */
    
    /* Configure horizontal synchronization width */
    LtdcHandler.Init.HorizontalSync = ILI9341_HSYNC;
    /* Configure vertical synchronization height */
    LtdcHandler.Init.VerticalSync = ILI9341_VSYNC;
    /* Configure accumulated horizontal back porch */
    LtdcHandler.Init.AccumulatedHBP = ILI9341_HBP;
    /* Configure accumulated vertical back porch */
    LtdcHandler.Init.AccumulatedVBP = ILI9341_VBP;
    /* Configure accumulated active width */
    LtdcHandler.Init.AccumulatedActiveW = 269;
    /* Configure accumulated active height */
    LtdcHandler.Init.AccumulatedActiveH = 323;
    /* Configure total width */
    LtdcHandler.Init.TotalWidth = 279;
    /* Configure total height */
    LtdcHandler.Init.TotalHeigh = 327;
    
    /* Configure R,G,B component values for LCD background color */
    LtdcHandler.Init.Backcolor.Red= 0;
    LtdcHandler.Init.Backcolor.Blue= 0;
    LtdcHandler.Init.Backcolor.Green= 0;
    
    /* LCD clock configuration */
    /* PLLSAI_VCO Input = HSE_VALUE/PLL_M = 1 Mhz */
    /* PLLSAI_VCO Output = PLLSAI_VCO Input * PLLSAIN = 192 Mhz */
    /* PLLLCDCLK = PLLSAI_VCO Output/PLLSAIR = 192/4 = 48 Mhz */
    /* LTDC clock frequency = PLLLCDCLK / LTDC_PLLSAI_DIVR_8 = 48/4 = 6Mhz */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC;
    PeriphClkInitStruct.PLLSAI.PLLSAIN = 192;
    PeriphClkInitStruct.PLLSAI.PLLSAIR = 4;
    PeriphClkInitStruct.PLLSAIDivR = RCC_PLLSAIDIVR_8;
    HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct); 
    
    /* Polarity */
    LtdcHandler.Init.HSPolarity = LTDC_HSPOLARITY_AL;
    LtdcHandler.Init.VSPolarity = LTDC_VSPOLARITY_AL;
    LtdcHandler.Init.DEPolarity = LTDC_DEPOLARITY_AL;
    LtdcHandler.Init.PCPolarity = LTDC_PCPOLARITY_IPC;
    
    LCD_MspInit();
    HAL_LTDC_Init(&LtdcHandler); 
    
    /* Select the device */
    LcdDrv = &ili9341_drv;

    /* LCD Init */	 
    LcdDrv->Init();

    /* Initialize the SDRAM */
    BSP_SDRAM_Init();

    /* Initialize the font */
    LCD_SetFont(&LCD_DEFAULT_FONT);

	LCD_LayerDefaultInit(1, LCD_FRAME_BUFFER_LAYER1);
	/* Set Foreground Layer */
	LCD_SelectLayer(1);
	/* Clear the LCD */
	LCD_Clear(LCD_COLOR_WHITE);
	LCD_SetColorKeying(1, LCD_COLOR_WHITE);
	LCD_SetLayerVisible(1, DISABLE);

	/* Layer1 Init */
	LCD_LayerDefaultInit(0, LCD_FRAME_BUFFER_LAYER0);

	/* Set Foreground Layer */
	LCD_SelectLayer(0);
	LCD_Clear(LCD_COLOR_WHITE);

	/* Enable The LCD */
	LCD_DisplayOn();
	// no buffering
	setvbuf(stdout, NULL, _IONBF, 0);

  return LCD_OK;
}  

/**
  * @brief  Gets the LCD X size.  
  * @retval The used LCD X size
  */
uint32_t LCD_GetXSize(void)
{
  return LcdDrv->GetLcdPixelWidth();
}

/**
  * @brief  Gets the LCD Y size.  
  * @retval The used LCD Y size
  */
uint32_t LCD_GetYSize(void)
{
  return LcdDrv->GetLcdPixelHeight();
}

/**
  * @brief  Initializes the LCD layers.
  * @param  LayerIndex: the layer foreground or background. 
  * @param  FB_Address: the layer frame buffer.
  */
void LCD_LayerDefaultInit(uint16_t LayerIndex, uint32_t FB_Address)
{     
  LCD_LayerCfgTypeDef   Layercfg;

 /* Layer Init */
  Layercfg.WindowX0 = 0;
  Layercfg.WindowX1 = LCD_GetXSize();
  Layercfg.WindowY0 = 0;
  Layercfg.WindowY1 = LCD_GetYSize(); 
  Layercfg.PixelFormat = LTDC_PIXEL_FORMAT_ARGB8888;
  Layercfg.FBStartAdress = FB_Address;
  Layercfg.Alpha = 255;
  Layercfg.Alpha0 = 0;
  Layercfg.Backcolor.Blue = 0;
  Layercfg.Backcolor.Green = 0;
  Layercfg.Backcolor.Red = 0;
  Layercfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_PAxCA;
  Layercfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_PAxCA;
  Layercfg.ImageWidth = LCD_GetXSize();
  Layercfg.ImageHeight = LCD_GetYSize();
  
  HAL_LTDC_ConfigLayer(&LtdcHandler, &Layercfg, LayerIndex); 

  DrawProp[LayerIndex].BackColor = LCD_COLOR_WHITE;
  DrawProp[LayerIndex].pFont     = &Font24;
  DrawProp[LayerIndex].TextColor = LCD_COLOR_BLACK; 

  /* Dithering activation */
  HAL_LTDC_EnableDither(&LtdcHandler);
}

/**
  * @brief  Selects the LCD Layer.
  * @param  LayerIndex: the Layer foreground or background.
  */
void LCD_SelectLayer(uint32_t LayerIndex)
{
  ActiveLayer = LayerIndex;
}

/**
  * @brief  Sets a LCD Layer visible.
  * @param  LayerIndex: the visible Layer.
  * @param  state: new state of the specified layer.
  *    This parameter can be: ENABLE or DISABLE.  
  */
void LCD_SetLayerVisible(uint32_t LayerIndex, FunctionalState state)
{
  if(state == ENABLE)
  {
    __HAL_LTDC_LAYER_ENABLE(&LtdcHandler, LayerIndex);
  }
  else
  {
    __HAL_LTDC_LAYER_DISABLE(&LtdcHandler, LayerIndex);
  }
  __HAL_LTDC_RELOAD_CONFIG(&LtdcHandler);
}

/**
  * @brief  Sets an LCD Layer visible without reloading.
  * @param  LayerIndex: Visible Layer
  * @param  State: New state of the specified layer
  *          This parameter can be one of the following values:
  *            @arg  ENABLE
  *            @arg  DISABLE 
  * @retval None
  */
void LCD_SetLayerVisible_NoReload(uint32_t LayerIndex, FunctionalState State)
{
  if(State == ENABLE)
  {
    __HAL_LTDC_LAYER_ENABLE(&LtdcHandler, LayerIndex);
  }
  else
  {
    __HAL_LTDC_LAYER_DISABLE(&LtdcHandler, LayerIndex);
  }
  /* Do not Sets the Reload  */
}

/**
  * @brief  Configures the Transparency.
  * @param  LayerIndex: the Layer foreground or background.
  * @param  Transparency: the Transparency, 
  *    This parameter must range from 0x00 to 0xFF.
  */
void LCD_SetTransparency(uint32_t LayerIndex, uint8_t Transparency)
{     
  HAL_LTDC_SetAlpha(&LtdcHandler, Transparency, LayerIndex);
}

/**
  * @brief  Configures the transparency without reloading.
  * @param  LayerIndex: Layer foreground or background.
  * @param  Transparency: Transparency
  *           This parameter must be a number between Min_Data = 0x00 and Max_Data = 0xFF 
  * @retval None
  */
void LCD_SetTransparency_NoReload(uint32_t LayerIndex, uint8_t Transparency)
{    
  HAL_LTDC_SetAlpha_NoReload(&LtdcHandler, Transparency, LayerIndex);
}

/**
  * @brief  Sets a LCD layer frame buffer address.
  * @param  LayerIndex: specifies the Layer foreground or background
  * @param  Address: new LCD frame buffer value      
  */
void LCD_SetLayerAddress(uint32_t LayerIndex, uint32_t Address)
{     
  HAL_LTDC_SetAddress(&LtdcHandler, Address, LayerIndex);
}

/**
  * @brief  Sets an LCD layer frame buffer address without reloading.
  * @param  LayerIndex: Layer foreground or background
  * @param  Address: New LCD frame buffer value      
  * @retval None
  */
void LCD_SetLayerAddress_NoReload(uint32_t LayerIndex, uint32_t Address)
{
  HAL_LTDC_SetAddress_NoReload(&LtdcHandler, Address, LayerIndex);
}

/**
  * @brief  Sets the Display window.
  * @param  LayerIndex: layer index
  * @param  Xpos: LCD X position
  * @param  Ypos: LCD Y position
  * @param  Width: LCD window width
  * @param  Height: LCD window height  
  */
void LCD_SetLayerWindow(uint16_t LayerIndex, uint16_t Xpos, uint16_t Ypos, uint16_t Width, uint16_t Height)
{
  /* reconfigure the layer size */
  HAL_LTDC_SetWindowSize(&LtdcHandler, Width, Height, LayerIndex);
  
  /* reconfigure the layer position */
  HAL_LTDC_SetWindowPosition(&LtdcHandler, Xpos, Ypos, LayerIndex);
}

/**
  * @brief  Sets display window without reloading.
  * @param  LayerIndex: Layer index
  * @param  Xpos: LCD X position
  * @param  Ypos: LCD Y position
  * @param  Width: LCD window width
  * @param  Height: LCD window height  
  * @retval None
  */
void LCD_SetLayerWindow_NoReload(uint16_t LayerIndex, uint16_t Xpos, uint16_t Ypos, uint16_t Width, uint16_t Height)
{
  /* Reconfigure the layer size */
  HAL_LTDC_SetWindowSize_NoReload(&LtdcHandler, Width, Height, LayerIndex);
  
  /* Reconfigure the layer position */
  HAL_LTDC_SetWindowPosition_NoReload(&LtdcHandler, Xpos, Ypos, LayerIndex); 
}

/**
  * @brief  Configures and sets the color Keying.
  * @param  LayerIndex: the Layer foreground or background
  * @param  RGBValue: the Color reference
  */
void LCD_SetColorKeying(uint32_t LayerIndex, uint32_t RGBValue)
{  
  /* Configure and Enable the color Keying for LCD Layer */
  HAL_LTDC_ConfigColorKeying(&LtdcHandler, RGBValue, LayerIndex);
  HAL_LTDC_EnableColorKeying(&LtdcHandler, LayerIndex);
}

/**
  * @brief  Configures and sets the color keying without reloading.
  * @param  LayerIndex: Layer foreground or background
  * @param  RGBValue: Color reference
  * @retval None
  */
void LCD_SetColorKeying_NoReload(uint32_t LayerIndex, uint32_t RGBValue)
{  
  /* Configure and Enable the color Keying for LCD Layer */
  HAL_LTDC_ConfigColorKeying_NoReload(&LtdcHandler, RGBValue, LayerIndex);
  HAL_LTDC_EnableColorKeying_NoReload(&LtdcHandler, LayerIndex);
}

/**
  * @brief  Disables the color Keying.
  * @param  LayerIndex: the Layer foreground or background
  */
void LCD_ResetColorKeying(uint32_t LayerIndex)
{
  /* Disable the color Keying for LCD Layer */
  HAL_LTDC_DisableColorKeying(&LtdcHandler, LayerIndex);
}

/**
  * @brief  Disables the color keying without reloading.
  * @param  LayerIndex: Layer foreground or background
  * @retval None
  */
void LCD_ResetColorKeying_NoReload(uint32_t LayerIndex)
{   
  /* Disable the color Keying for LCD Layer */
  HAL_LTDC_DisableColorKeying_NoReload(&LtdcHandler, LayerIndex);
}

/**
  * @brief  Disables the color keying without reloading.
  * @param  ReloadType: can be one of the following values
  *         - LCD_RELOAD_IMMEDIATE
  *         - LCD_RELOAD_VERTICAL_BLANKING
  * @retval None
  */
void LCD_Relaod(uint32_t ReloadType)
{
  HAL_LTDC_Relaod (&LtdcHandler, ReloadType);
}

/**
  * @brief  Gets the LCD Text color.
  * @retval Text color
  */
uint32_t LCD_GetTextColor(void)
{
  return DrawProp[ActiveLayer].TextColor;
}

/**
  * @brief  Gets the LCD Background color. 
  * @retval Background color  
  */
uint32_t LCD_GetBackColor(void)
{
  return DrawProp[ActiveLayer].BackColor;
}

/**
  * @brief  Sets the Text color.
  * @param  Color: the Text color code ARGB(8-8-8-8)
  */
void LCD_SetTextColor(uint32_t Color)
{
  DrawProp[ActiveLayer].TextColor = Color;
}

/**
  * @brief  Sets the Background color.
  * @param  Color: the layer Background color code ARGB(8-8-8-8)
  */
void LCD_SetBackColor(uint32_t Color)
{
  DrawProp[ActiveLayer].BackColor = Color;
}

/**
  * @brief  Sets the Text Font.
  * @param  pFonts: the layer font to be used
  */
void LCD_SetFont(sFONT *pFonts)
{
  DrawProp[ActiveLayer].pFont = pFonts;
}

/**
  * @brief  Gets the Text Font.
  * @retval Layer font
  */
sFONT *LCD_GetFont(void)
{
  return DrawProp[ActiveLayer].pFont;
}

/**
  * @brief  Reads Pixel.
  * @param  Xpos: the X position
  * @param  Ypos: the Y position 
  * @retval RGB pixel color
  */
uint32_t LCD_ReadPixel(uint16_t Xpos, uint16_t Ypos)
{
  uint32_t ret = 0;
  
  if (Xpos > LCD_GetXSize() || Ypos > LCD_GetYSize()) {
	return 0;
  }

  if(LtdcHandler.LayerCfg[ActiveLayer].PixelFormat == LTDC_PIXEL_FORMAT_ARGB8888)
  {
    /* Read data value from SDRAM memory */
    ret = *(__IO uint32_t*) (LtdcHandler.LayerCfg[ActiveLayer].FBStartAdress + (4*(Ypos*LCD_GetXSize() + Xpos)));
  }
  else if(LtdcHandler.LayerCfg[ActiveLayer].PixelFormat == LTDC_PIXEL_FORMAT_RGB888)
  {
    /* Read data value from SDRAM memory */
    ret = (*(__IO uint32_t*) (LtdcHandler.LayerCfg[ActiveLayer].FBStartAdress + (4*(Ypos*LCD_GetXSize() + Xpos))) & 0x00FFFFFF);
  }
  else if((LtdcHandler.LayerCfg[ActiveLayer].PixelFormat == LTDC_PIXEL_FORMAT_RGB565) || \
          (LtdcHandler.LayerCfg[ActiveLayer].PixelFormat == LTDC_PIXEL_FORMAT_ARGB4444) || \
          (LtdcHandler.LayerCfg[ActiveLayer].PixelFormat == LTDC_PIXEL_FORMAT_AL88))  
  {
    /* Read data value from SDRAM memory */
    ret = *(__IO uint16_t*) (LtdcHandler.LayerCfg[ActiveLayer].FBStartAdress + (2*(Ypos*LCD_GetXSize() + Xpos)));    
  }
  else
  {
    /* Read data value from SDRAM memory */
    ret = *(__IO uint8_t*) (LtdcHandler.LayerCfg[ActiveLayer].FBStartAdress + (2*(Ypos*LCD_GetXSize() + Xpos)));    
  }

  return ret;
}

/**
  * @brief  Clears the hole LCD.
  * @param  Color: the color of the background
  */
void LCD_Clear(uint32_t Color)
{ 
  /* Clear the LCD */ 
  FillBuffer(ActiveLayer, (uint32_t *)(LtdcHandler.LayerCfg[ActiveLayer].FBStartAdress), LCD_GetXSize(), LCD_GetYSize(), 0, Color);
}

/**
  * @brief  Clears the selected line.
  * @param  Line: the line to be cleared
  */
void LCD_ClearStringLine(uint32_t Line)
{
  uint32_t colorbackup = DrawProp[ActiveLayer].TextColor;
  DrawProp[ActiveLayer].TextColor = DrawProp[ActiveLayer].BackColor;

  /* Draw rectangle with background color */
  LCD_FillRect(0, (Line * DrawProp[ActiveLayer].pFont->Height), LCD_GetXSize(), DrawProp[ActiveLayer].pFont->Height);
  
  DrawProp[ActiveLayer].TextColor = colorbackup;
  LCD_SetTextColor(DrawProp[ActiveLayer].TextColor);  
}

/**
  * @brief  Displays one character.
  * @param  Xpos: start column address
  * @param  Ypos: the Line where to display the character shape
  * @param  Ascii: character ascii code, must be between 0x20 and 0x7E
  */
void LCD_DisplayChar(uint16_t Xpos, uint16_t Ypos, uint8_t Ascii)
{
  DrawChar(Xpos, Ypos, &DrawProp[ActiveLayer].pFont->table[(Ascii-' ') *\
              DrawProp[ActiveLayer].pFont->Height * ((DrawProp[ActiveLayer].pFont->Width + 7) / 8)]);
}

/**
  * @brief  Displays a maximum of 60 char on the LCD.
  * @param  X: pointer to x position (in pixel)
  * @param  Y: pointer to y position (in pixel)    
  * @param  pText: pointer to string to display on LCD
  * @param  mode: The display mode
  *    This parameter can be one of the following values:
  *                @arg CENTER_MODE 
  *                @arg RIGHT_MODE
  *                @arg LEFT_MODE   
  */
void LCD_DisplayStringAt(uint16_t X, uint16_t Y, char *pText, Text_AlignModeTypdef mode)
{
  uint16_t refcolumn = 1, i = 0;
  uint32_t size = 0, xsize = 0; 
  char  *ptr = pText;
  
  /* Get the text size */
  while (*ptr++) size ++ ;
  
  /* Characters number per line */
  xsize = (LCD_GetXSize()/DrawProp[ActiveLayer].pFont->Width);
  
  switch (mode)
  {
  case CENTER_MODE:
    {
      refcolumn = X+ ((xsize - size)* DrawProp[ActiveLayer].pFont->Width) / 2;
      break;
    }
  case LEFT_MODE:
    {
      refcolumn = X;
      break;
    }
  case RIGHT_MODE:
    {
      refcolumn = X + ((xsize - size)*DrawProp[ActiveLayer].pFont->Width);
      break;
    }
  default:
    {
      refcolumn = X;
      break;
    }
  }

  /* Send the string character by character on LCD */
  while ((*pText != 0) & (((LCD_GetXSize() - (i*DrawProp[ActiveLayer].pFont->Width)) & 0xFFFF) >= DrawProp[ActiveLayer].pFont->Width))
  {
    /* Display one character on LCD */
    LCD_DisplayChar(refcolumn, Y, *pText);
    /* Decrement the column position by 16 */
    refcolumn += DrawProp[ActiveLayer].pFont->Width;
    /* Point on the next character */
    pText++;
    i++;
  }  
}

/**
  * @brief  Displays a maximum of 20 char on the LCD.
  * @param  Line: the Line where to display the character shape
  * @param  ptr: pointer to string to display on LCD
  */
void LCD_DisplayStringAtLine(uint16_t Line, char *ptr)
{
  LCD_DisplayStringAt(0, LINE(Line), ptr, LEFT_MODE);
}

/**
  * @brief  Displays a maximum of 20 char on the LCD.
  * @param  Line: the Line where to display the character shape
  * @param  ptr: pointer to string to display on LCD
  */
void LCD_DisplayStringAtLineMode(uint16_t Line, char *ptr, Text_AlignModeTypdef mode)
{
  LCD_DisplayStringAt(0, LINE(Line), ptr, mode);
}

/**
  * @brief  Displays an horizontal line.
  * @param  Xpos: the X position
  * @param  Ypos: the Y position
  * @param  Length: line length
  */
void LCD_DrawHLine(uint16_t Xpos, uint16_t Ypos, uint16_t Length)
{
  uint32_t xaddress = 0;

  if (Xpos > LCD_GetXSize() || Ypos > LCD_GetYSize()) {
	return;
  }
  if (Xpos + Length > LCD_GetXSize()) {
	  Length = LCD_GetXSize() - Xpos;
  }
  
  /* Get the line address */
  xaddress = (LtdcHandler.LayerCfg[ActiveLayer].FBStartAdress) + 4*(LCD_GetXSize()*Ypos + Xpos);

  /* Write line */
  FillBuffer(ActiveLayer, (uint32_t *)xaddress, Length, 1, 0, DrawProp[ActiveLayer].TextColor);
}

/**
  * @brief  Displays a vertical line.
  * @param  Xpos: the X position
  * @param  Ypos: the Y position
  * @param  Length: line length
  */
void LCD_DrawVLine(uint16_t Xpos, uint16_t Ypos, uint16_t Length)
{
  uint32_t xaddress = 0;
  
  if (Xpos > LCD_GetXSize() || Ypos > LCD_GetYSize()) {
	return;
  }
  if (Ypos + Length > LCD_GetYSize()) {
	  Length = LCD_GetYSize() - Ypos;
  }
  /* Get the line address */
  xaddress = (LtdcHandler.LayerCfg[ActiveLayer].FBStartAdress) + 4*(LCD_GetXSize()*Ypos + Xpos);
  
  /* Write line */
  FillBuffer(ActiveLayer, (uint32_t *)xaddress, 1, Length, (LCD_GetXSize() - 1), DrawProp[ActiveLayer].TextColor);
}

/**
  * @brief  Displays an uni-line (between two points).
  * @param  X1: the point 1 X position
  * @param  Y1: the point 1 Y position
  * @param  X2: the point 2 X position
  * @param  Y2: the point 2 Y position
  */
void LCD_DrawLine(uint16_t X1, uint16_t Y1, uint16_t X2, uint16_t Y2)
{
  int16_t deltax = 0, deltay = 0, x = 0, y = 0, xinc1 = 0, xinc2 = 0, 
  yinc1 = 0, yinc2 = 0, den = 0, num = 0, numadd = 0, numpixels = 0, 
  curpixel = 0;
  
  deltax = ABS(X2 - X1);        /* The difference between the x's */
  deltay = ABS(Y2 - Y1);        /* The difference between the y's */
  x = X1;                       /* Start x off at the first pixel */
  y = Y1;                       /* Start y off at the first pixel */
  
  if (X2 >= X1)                 /* The x-values are increasing */
  {
    xinc1 = 1;
    xinc2 = 1;
  }
  else                          /* The x-values are decreasing */
  {
    xinc1 = -1;
    xinc2 = -1;
  }
  
  if (Y2 >= Y1)                 /* The y-values are increasing */
  {
    yinc1 = 1;
    yinc2 = 1;
  }
  else                          /* The y-values are decreasing */
  {
    yinc1 = -1;
    yinc2 = -1;
  }
  
  if (deltax >= deltay)         /* There is at least one x-value for every y-value */
  {
    xinc1 = 0;                  /* Don't change the x when numerator >= denominator */
    yinc2 = 0;                  /* Don't change the y for every iteration */
    den = deltax;
    num = deltax / 2;
    numadd = deltay;
    numpixels = deltax;         /* There are more x-values than y-values */
  }
  else                          /* There is at least one y-value for every x-value */
  {
    xinc2 = 0;                  /* Don't change the x for every iteration */
    yinc1 = 0;                  /* Don't change the y when numerator >= denominator */
    den = deltay;
    num = deltay / 2;
    numadd = deltax;
    numpixels = deltay;         /* There are more y-values than x-values */
  }
  
  for (curpixel = 0; curpixel <= numpixels; curpixel++)
  {
    LCD_DrawPixel(x, y, DrawProp[ActiveLayer].TextColor);   /* Draw the current pixel */
    num += numadd;                            /* Increase the numerator by the top of the fraction */
    if (num >= den)                           /* Check if numerator >= denominator */
    {
      num -= den;                             /* Calculate the new numerator value */
      x += xinc1;                             /* Change the x as appropriate */
      y += yinc1;                             /* Change the y as appropriate */
    }
    x += xinc2;                               /* Change the x as appropriate */
    y += yinc2;                               /* Change the y as appropriate */
  }
}

/**
  * @brief  Displays a rectangle.
  * @param  Xpos: the X position
  * @param  Ypos: the Y position
  * @param  Height: display rectangle height
  * @param  Width: display rectangle width
  */
void LCD_DrawRect(uint16_t Xpos, uint16_t Ypos, uint16_t Width, uint16_t Height)
{
  /* Draw horizontal lines */
  LCD_DrawHLine(Xpos, Ypos, Width);
  LCD_DrawHLine(Xpos, (Ypos+ Height), Width);
  
  /* Draw vertical lines */
  LCD_DrawVLine(Xpos, Ypos, Height);
  LCD_DrawVLine((Xpos + Width), Ypos, Height);
}

/**
  * @brief  Displays a circle.
  * @param  Xpos: the X position
  * @param  Ypos: the Y position
  * @param  Radius: the circle radius
  */
void LCD_DrawCircle(uint16_t Xpos, uint16_t Ypos, uint16_t Radius)
{
  int32_t  d;/* Decision Variable */ 
  uint32_t  curx;/* Current X Value */
  uint32_t  cury;/* Current Y Value */ 
  
  d = 3 - (Radius << 1);
  curx = 0;
  cury = Radius;
  
  while (curx <= cury)
  {
    LCD_DrawPixel((Xpos + curx), (Ypos - cury), DrawProp[ActiveLayer].TextColor);
    LCD_DrawPixel((Xpos - curx), (Ypos - cury), DrawProp[ActiveLayer].TextColor);
    LCD_DrawPixel((Xpos + cury), (Ypos - curx), DrawProp[ActiveLayer].TextColor);
    LCD_DrawPixel((Xpos - cury), (Ypos - curx), DrawProp[ActiveLayer].TextColor);
    LCD_DrawPixel((Xpos + curx), (Ypos + cury), DrawProp[ActiveLayer].TextColor);
    LCD_DrawPixel((Xpos - curx), (Ypos + cury), DrawProp[ActiveLayer].TextColor);
    LCD_DrawPixel((Xpos + cury), (Ypos + curx), DrawProp[ActiveLayer].TextColor);
    LCD_DrawPixel((Xpos - cury), (Ypos + curx), DrawProp[ActiveLayer].TextColor);   

    if (d < 0)
    { 
      d += (curx << 2) + 6;
    }
    else
    {
      d += ((curx - cury) << 2) + 10;
      cury--;
    }
    curx++;
  } 
}

/**
  * @brief  Displays an poly-line (between many points).
  * @param  Points: pointer to the points array
  * @param  PointCount: Number of points
  */
void LCD_DrawPolygon(pPoint Points, uint16_t PointCount)
{
  int16_t x = 0, y = 0;

  if(PointCount < 2)
  {
    return;
  }

  LCD_DrawLine(Points->X, Points->Y, (Points+PointCount-1)->X, (Points+PointCount-1)->Y);
  
  while(--PointCount)
  {
    x = Points->X;
    y = Points->Y;
    Points++;
    LCD_DrawLine(x, y, Points->X, Points->Y);
  }
}

/**
  * @brief  Displays an Ellipse.
  * @param  Xpos: the X position
  * @param  Ypos: the Y position
  * @param  XRadius: the X radius of ellipse
  * @param  YRadius: the Y radius of ellipse
  */
void LCD_DrawEllipse(int Xpos, int Ypos, int XRadius, int YRadius)
{
  int x = 0, y = -YRadius, err = 2-2*XRadius, e2;
  float k = 0, rad1 = 0, rad2 = 0;
  
  rad1 = XRadius;
  rad2 = YRadius;
  
  k = (float)(rad2/rad1);
  
  do { 
    LCD_DrawPixel((Xpos-(uint16_t)(x/k)), (Ypos+y), DrawProp[ActiveLayer].TextColor);
    LCD_DrawPixel((Xpos+(uint16_t)(x/k)), (Ypos+y), DrawProp[ActiveLayer].TextColor);
    LCD_DrawPixel((Xpos+(uint16_t)(x/k)), (Ypos-y), DrawProp[ActiveLayer].TextColor);
    LCD_DrawPixel((Xpos-(uint16_t)(x/k)), (Ypos-y), DrawProp[ActiveLayer].TextColor);      
    
    e2 = err;
    if (e2 <= x) {
      err += ++x*2+1;
      if (-y == x && e2 <= y) e2 = 0;
    }
    if (e2 > y) err += ++y*2+1;
  }
  while (y <= 0);
}

/**
  * @brief  Displays a bitmap picture loaded in the internal Flash (32 bpp).
  * @param  X: the bmp x position in the LCD
  * @param  Y: the bmp Y position in the LCD
  * @param  pBmp: Bmp picture address in the internal Flash
  */
void LCD_DrawBitmap(uint32_t X, uint32_t Y, uint8_t *pBmp)
{
  uint32_t index = 0, width = 0, height = 0, bitpixel = 0;
  uint32_t address;
  uint32_t inputcolormode = 0;
  
  /* Get bitmap data address offset */
  index = pBmp[10] + (pBmp[11] << 8) + (pBmp[12] << 16)  + (pBmp[13] << 24);

  /* Read bitmap width */
  width = pBmp[18] + (pBmp[19] << 8) + (pBmp[20] << 16)  + (pBmp[21] << 24);

  /* Read bitmap height */
  height = pBmp[22] + (pBmp[23] << 8) + (pBmp[24] << 16)  + (pBmp[25] << 24);

  /* Read bit/pixel */
  bitpixel = pBmp[28] + (pBmp[29] << 8);   
 
  /* Set Address */
  address = LtdcHandler.LayerCfg[ActiveLayer].FBStartAdress + (((LCD_GetXSize()*Y) + X)*(4));

  /* Get the Layer pixel format */    
  if ((bitpixel/8) == 4)
  {
    inputcolormode = CM_ARGB8888;
  }
  else if ((bitpixel/8) == 2)
  {
    inputcolormode = CM_RGB565;
  }
  else
  {
    inputcolormode = CM_RGB888;
  }
 
  /* bypass the bitmap header */
  pBmp += (index + (width * (height - 1) * (bitpixel/8)));

  /* Convert picture to ARGB8888 pixel format */
  for(index=0; index < height; index++)
  {
  /* Pixel format conversion */
  ConvertLineToARGB8888((uint32_t *)pBmp, (uint32_t *)address, width, inputcolormode);

  /* Increment the source and destination buffers */
  address+=  ((LCD_GetXSize() - width + width)*4);
  pBmp -= width*(bitpixel/8);
  }
}

/**
  * @brief  Displays a full rectangle.
  * @param  Xpos: the X position
  * @param  Ypos: the Y position
  * @param  Height: rectangle height
  * @param  Width: rectangle width
  */
void LCD_FillRect(uint16_t Xpos, uint16_t Ypos, uint16_t Width, uint16_t Height)
{
  uint32_t xaddress = 0;

  if (Xpos > LCD_GetXSize() || Ypos > LCD_GetYSize()) {
	return;
  }
  if (Xpos + Width > LCD_GetXSize()) {
	  Width = LCD_GetXSize() - Xpos;
  }
  if (Ypos + Height > LCD_GetYSize()) {
	  Height = LCD_GetYSize() - Ypos;
  }
  /* Set the text color */
  LCD_SetTextColor(DrawProp[ActiveLayer].TextColor);

  /* Get the rectangle start address */
  xaddress = (LtdcHandler.LayerCfg[ActiveLayer].FBStartAdress) + 4*(LCD_GetXSize()*Ypos + Xpos);

  /* Fill the rectangle */
  FillBuffer(ActiveLayer, (uint32_t *)xaddress, Width, Height, (LCD_GetXSize() - Width), DrawProp[ActiveLayer].TextColor);
}

/**
  * @brief  Displays a full circle.
  * @param  Xpos: the X position
  * @param  Ypos: the Y position
  * @param  Radius: the circle radius
  */
void LCD_FillCircle(uint16_t Xpos, uint16_t Ypos, uint16_t Radius)
{
  int32_t  d;    /* Decision Variable */ 
  uint32_t  curx;/* Current X Value */
  uint32_t  cury;/* Current Y Value */ 
  
  d = 3 - (Radius << 1);

  curx = 0;
  cury = Radius;
  
  LCD_SetTextColor(DrawProp[ActiveLayer].TextColor);

  while (curx <= cury)
  {
    if(cury > 0) 
    {
      LCD_DrawHLine(Xpos - cury, Ypos + curx, 2*cury);
      LCD_DrawHLine(Xpos - cury, Ypos - curx, 2*cury);
    }

    if(curx > 0) 
    {
      LCD_DrawHLine(Xpos - curx, Ypos - cury, 2*curx);
      LCD_DrawHLine(Xpos - curx, Ypos + cury, 2*curx);
    }
    if (d < 0)
    { 
      d += (curx << 2) + 6;
    }
    else
    {
      d += ((curx - cury) << 2) + 10;
      cury--;
    }
    curx++;
  }

  LCD_SetTextColor(DrawProp[ActiveLayer].TextColor);
  LCD_DrawCircle(Xpos, Ypos, Radius);
}

/**
  * @brief  Fill triangle.
  * @param  X1: the point 1 x position
  * @param  Y1: the point 1 y position
  * @param  X2: the point 2 x position
  * @param  Y2: the point 2 y position
  * @param  X3: the point 3 x position
  * @param  Y3: the point 3 y position
  */
void LCD_FillTriangle(uint16_t X1, uint16_t X2, uint16_t X3, uint16_t Y1, uint16_t Y2, uint16_t Y3)
{ 
  int16_t deltax = 0, deltay = 0, x = 0, y = 0, xinc1 = 0, xinc2 = 0, 
  yinc1 = 0, yinc2 = 0, den = 0, num = 0, numadd = 0, numpixels = 0, 
  curpixel = 0;
  
  deltax = ABS(X2 - X1);        /* The difference between the x's */
  deltay = ABS(Y2 - Y1);        /* The difference between the y's */
  x = X1;                       /* Start x off at the first pixel */
  y = Y1;                       /* Start y off at the first pixel */
  
  if (X2 >= X1)                 /* The x-values are increasing */
  {
    xinc1 = 1;
    xinc2 = 1;
  }
  else                          /* The x-values are decreasing */
  {
    xinc1 = -1;
    xinc2 = -1;
  }
  
  if (Y2 >= Y1)                 /* The y-values are increasing */
  {
    yinc1 = 1;
    yinc2 = 1;
  }
  else                          /* The y-values are decreasing */
  {
    yinc1 = -1;
    yinc2 = -1;
  }
  
  if (deltax >= deltay)         /* There is at least one x-value for every y-value */
  {
    xinc1 = 0;                  /* Don't change the x when numerator >= denominator */
    yinc2 = 0;                  /* Don't change the y for every iteration */
    den = deltax;
    num = deltax / 2;
    numadd = deltay;
    numpixels = deltax;         /* There are more x-values than y-values */
  }
  else                          /* There is at least one y-value for every x-value */
  {
    xinc2 = 0;                  /* Don't change the x for every iteration */
    yinc1 = 0;                  /* Don't change the y when numerator >= denominator */
    den = deltay;
    num = deltay / 2;
    numadd = deltax;
    numpixels = deltay;         /* There are more y-values than x-values */
  }
  
  for (curpixel = 0; curpixel <= numpixels; curpixel++)
  {
    LCD_DrawLine(x, y, X3, Y3);
    
    num += numadd;              /* Increase the numerator by the top of the fraction */
    if (num >= den)             /* Check if numerator >= denominator */
    {
      num -= den;               /* Calculate the new numerator value */
      x += xinc1;               /* Change the x as appropriate */
      y += yinc1;               /* Change the y as appropriate */
    }
    x += xinc2;                 /* Change the x as appropriate */
    y += yinc2;                 /* Change the y as appropriate */
  } 
}

/**
  * @brief  Displays a full poly-line (between many points).
  * @param  Points: pointer to the points array
  * @param  PointCount: Number of points
  */
void LCD_FillPolygon(pPoint Points, uint16_t PointCount)
{
  
  int16_t x = 0, y = 0, x2 = 0, y2 = 0, xcenter = 0, ycenter = 0, xfirst = 0, yfirst = 0, pixelx = 0, pixely = 0, counter = 0;
  uint16_t  imageleft = 0, imageright = 0, imagetop = 0, imagebottom = 0;  

  imageleft = imageright = Points->X;
  imagetop= imagebottom = Points->Y;

  for(counter = 1; counter < PointCount; counter++)
  {
    pixelx = POLY_X(counter);
    if(pixelx < imageleft)
    {
      imageleft = pixelx;
    }
    if(pixelx > imageright)
    {
      imageright = pixelx;
    }

    pixely = POLY_Y(counter);
    if(pixely < imagetop)
    { 
      imagetop = pixely;
    }
    if(pixely > imagebottom)
    {
      imagebottom = pixely;
    }
  }  

  if(PointCount < 2)
  {
    return;
  }

  xcenter = (imageleft + imageright)/2;
  ycenter = (imagebottom + imagetop)/2;
 
  xfirst = Points->X;
  yfirst = Points->Y;

  while(--PointCount)
  {
    x = Points->X;
    y = Points->Y;
    Points++;
    x2 = Points->X;
    y2 = Points->Y;    
  
    LCD_FillTriangle(x, x2, xcenter, y, y2, ycenter);
    LCD_FillTriangle(x, xcenter, x2, y, ycenter, y2);
    LCD_FillTriangle(xcenter, x2, x, ycenter, y2, y);   
  }
  
  LCD_FillTriangle(xfirst, x2, xcenter, yfirst, y2, ycenter);
  LCD_FillTriangle(xfirst, xcenter, x2, yfirst, ycenter, y2);
  LCD_FillTriangle(xcenter, x2, xfirst, ycenter, y2, yfirst);   
}

/**
  * @brief  Draw a full ellipse.
  * @param  Xpos: the X position
  * @param  Ypos: the Y position
  * @param  XRadius: X radius of ellipse
  * @param  YRadius: Y radius of ellipse. 
  */
void LCD_FillEllipse(int Xpos, int Ypos, int XRadius, int YRadius)
{
  int x = 0, y = -YRadius, err = 2-2*XRadius, e2;
  float K = 0, rad1 = 0, rad2 = 0;
  
  rad1 = XRadius;
  rad2 = YRadius;
  K = (float)(rad2/rad1);
  
  do 
  { 
    LCD_DrawHLine((Xpos-(uint16_t)(x/K)), (Ypos+y), (2*(uint16_t)(x/K) + 1));
    LCD_DrawHLine((Xpos-(uint16_t)(x/K)), (Ypos-y), (2*(uint16_t)(x/K) + 1));
    
    e2 = err;
    if (e2 <= x) 
    {
      err += ++x*2+1;
      if (-y == x && e2 <= y) e2 = 0;
    }
    if (e2 > y) err += ++y*2+1;
  }
  while (y <= 0);
}

/**
  * @brief  Enables the Display.
  */
void LCD_DisplayOn(void)
{
  if(LcdDrv->DisplayOn != NULL)
  {
    LcdDrv->DisplayOn();
  }
}

/**
  * @brief  Disables the Display.
  */
void LCD_DisplayOff(void)
{
  if(LcdDrv->DisplayOff != NULL)
  {
    LcdDrv->DisplayOff();
  }
}

/*******************************************************************************
                       LTDC and DMA2D BSP Routines
*******************************************************************************/

/**
  * @brief  Initializes the LTDC MSP.
  */
__weak void LCD_MspInit(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  
  /* Enable the LTDC and DMA2D Clock */
  __HAL_RCC_LTDC_CLK_ENABLE();
  __HAL_RCC_DMA2D_CLK_ENABLE(); 
  
  /* Enable GPIOs clock */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();

  /* GPIOs Configuration */
  /*
   +------------------------+-----------------------+----------------------------+
   +                       LCD pins assignment                                   +
   +------------------------+-----------------------+----------------------------+
   |  LCD_TFT R2 <-> PC.10  |  LCD_TFT G2 <-> PA.06 |  LCD_TFT B2 <-> PD.06      |
   |  LCD_TFT R3 <-> PB.00  |  LCD_TFT G3 <-> PG.10 |  LCD_TFT B3 <-> PG.11      |
   |  LCD_TFT R4 <-> PA.11  |  LCD_TFT G4 <-> PB.10 |  LCD_TFT B4 <-> PG.12      |
   |  LCD_TFT R5 <-> PA.12  |  LCD_TFT G5 <-> PB.11 |  LCD_TFT B5 <-> PA.03      |
   |  LCD_TFT R6 <-> PB.01  |  LCD_TFT G6 <-> PC.07 |  LCD_TFT B6 <-> PB.08      |
   |  LCD_TFT R7 <-> PG.06  |  LCD_TFT G7 <-> PD.03 |  LCD_TFT B7 <-> PB.09      |
   -------------------------------------------------------------------------------
            |  LCD_TFT HSYNC <-> PC.06  | LCDTFT VSYNC <->  PA.04 |
            |  LCD_TFT CLK   <-> PG.07  | LCD_TFT DE   <->  PF.10 |
             -----------------------------------------------------
  */

  /* GPIOA configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_6 |
                           GPIO_PIN_11 | GPIO_PIN_12;
  GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStructure.Pull = GPIO_NOPULL;
  GPIO_InitStructure.Speed = GPIO_SPEED_FAST;
  GPIO_InitStructure.Alternate= GPIO_AF14_LTDC;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

 /* GPIOB configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_8 | \
                           GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

 /* GPIOC configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_10;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);

 /* GPIOD configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_3 | GPIO_PIN_6;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);
  
 /* GPIOF configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_10;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStructure);     

 /* GPIOG configuration */  
  GPIO_InitStructure.Pin = GPIO_PIN_6 | GPIO_PIN_7 | \
                           GPIO_PIN_11;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStructure);
 
  /* GPIOB configuration */  
  GPIO_InitStructure.Pin = GPIO_PIN_0 | GPIO_PIN_1;
  GPIO_InitStructure.Alternate= GPIO_AF9_LTDC;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

  /* GPIOG configuration */  
  GPIO_InitStructure.Pin = GPIO_PIN_10 | GPIO_PIN_12;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStructure);
}

/*******************************************************************************
                            Static Functions
*******************************************************************************/

/**
  * @brief  Writes Pixel.
  * @param  Xpos: the X position
  * @param  Ypos: the Y position
  * @param  RGB_Code: the pixel color in ARGB mode (8-8-8-8)  
  */
void LCD_DrawPixel(uint16_t Xpos, uint16_t Ypos, uint32_t RGB_Code)
{
	if (Xpos > LCD_GetXSize() || Ypos > LCD_GetYSize()) {
		return;
	}
  /* Write data value to all SDRAM memory */
  *(__IO uint32_t*) (LtdcHandler.LayerCfg[ActiveLayer].FBStartAdress + (4*(Ypos*LCD_GetXSize() + Xpos))) = RGB_Code;
}

/**
  * @brief  Draws a character on LCD.
  * @param  Xpos: the Line where to display the character shape
  * @param  Ypos: start column address
  * @param  c: pointer to the character data
  */
static void DrawChar(uint16_t Xpos, uint16_t Ypos, const uint8_t *c)
{
  uint32_t i = 0, j = 0;
  uint16_t height, width;
  uint8_t offset;
  uint8_t *pchar;
  uint32_t line=0;

  height = DrawProp[ActiveLayer].pFont->Height;
  width  = DrawProp[ActiveLayer].pFont->Width;

  offset = 8 *((width + 7)/8) -  width ;

  for(i = 0; i < height; i++)
  {
    pchar = ((uint8_t *)c + (width + 7)/8 * i);

    switch(((width + 7)/8))
    {
    case 1:
      line =  pchar[0];      
      break;
      
    case 2:
      line =  (pchar[0]<< 8) | pchar[1];
      break;

    case 3:
    default:
      line =  (pchar[0]<< 16) | (pchar[1]<< 8) | pchar[2];      
      break;
    }

    for (j = 0; j < width; j++)
    {
      if(line & (1 << (width- j + offset- 1))) 
      {
        LCD_DrawPixel((Xpos + j), Ypos, DrawProp[ActiveLayer].TextColor);
      }
      else
      {
        LCD_DrawPixel((Xpos + j), Ypos, DrawProp[ActiveLayer].BackColor);
      } 
    }
    Ypos++;
  }
}

/**
  * @brief  Fills buffer.
  * @param  LayerIndex: layer index
  * @param  pDst: output color
  * @param  xSize: buffer width
  * @param  ySize: buffer height
  * @param  OffLine: offset
  * @param  ColorIndex: color Index  
  */
static void FillBuffer(uint32_t LayerIndex, void * pDst, uint32_t xSize, uint32_t ySize, uint32_t OffLine, uint32_t ColorIndex) 
{
  
  /* Register to memory mode with ARGB8888 as color Mode */ 
  Dma2dHandler.Init.Mode         = DMA2D_R2M;
  Dma2dHandler.Init.ColorMode    = DMA2D_ARGB8888;
  Dma2dHandler.Init.OutputOffset = OffLine;      
  
  Dma2dHandler.Instance = DMA2D; 
  
  /* DMA2D Initialization */
  if(HAL_DMA2D_Init(&Dma2dHandler) == HAL_OK) 
  {
    if(HAL_DMA2D_ConfigLayer(&Dma2dHandler, LayerIndex) == HAL_OK) 
    {
      if (HAL_DMA2D_Start(&Dma2dHandler, ColorIndex, (uint32_t)pDst, xSize, ySize) == HAL_OK)
      {
        /* Polling For DMA transfer */  
        HAL_DMA2D_PollForTransfer(&Dma2dHandler, 10);
      }
    }
  } 
}

/**
  * @brief  Converts Line to ARGB8888 pixel format.
  * @param  pSrc: pointer to source buffer
  * @param  pDst: output color
  * @param  xSize: buffer width
  * @param  ColorMode: input color mode   
  */
static void ConvertLineToARGB8888(void * pSrc, void * pDst, uint32_t xSize, uint32_t ColorMode)
{    
  /* Configure the DMA2D Mode, Color Mode and output offset */
  Dma2dHandler.Init.Mode         = DMA2D_M2M_PFC;
  Dma2dHandler.Init.ColorMode    = DMA2D_ARGB8888;
  Dma2dHandler.Init.OutputOffset = 0;     
  
  /* Foreground Configuration */
  Dma2dHandler.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
  Dma2dHandler.LayerCfg[1].InputAlpha = 0xFF;
  Dma2dHandler.LayerCfg[1].InputColorMode = ColorMode;
  Dma2dHandler.LayerCfg[1].InputOffset = 0;
  
  Dma2dHandler.Instance = DMA2D; 
  
  /* DMA2D Initialization */
  if(HAL_DMA2D_Init(&Dma2dHandler) == HAL_OK) 
  {
    if(HAL_DMA2D_ConfigLayer(&Dma2dHandler, 1) == HAL_OK) 
    {
      if (HAL_DMA2D_Start(&Dma2dHandler, (uint32_t)pSrc, (uint32_t)pDst, xSize, 1) == HAL_OK)
      {
        /* Polling For DMA transfer */  
        HAL_DMA2D_PollForTransfer(&Dma2dHandler, 10);
      }
    }
  } 
}

/**
  * @brief  Sets the LCD Text and Background colors.
  * @param  TextColor: specifies the Text Color.
  * @param  BackColor: specifies the Background Color.
  * @retval None
  */
void LCD_SetColors(uint32_t TextColor, uint32_t BackColor)
{
  LCD_SetTextColor(TextColor);
  LCD_SetBackColor(BackColor);
}

static unsigned int  Line=0;
static unsigned int  Column=0;
#define COLUMN(x) ((x) * (((sFONT *)LCD_GetFont())->Width))


#define MAX_LINE		 ((320/(((sFONT *)LCD_GetFont())->Height))-1)
#define MAX_COLUMN	 ((240/(((sFONT *)LCD_GetFont())->Width))-1)

void LCD_SetPrintPosition(unsigned int ln, unsigned int col)
{
	Line 		= ( ln <= MAX_LINE )? ln : MAX_LINE;
	Column 	= ( col <= MAX_COLUMN ) ? col : MAX_COLUMN;
}

int __io_putchar(int ch)
{
 fputc(ch, stdout);
 return ch;
}

int _write(int file,char *ptr, int len)
{
 int DataIdx;
 for(DataIdx= 0; DataIdx< len; DataIdx++)
 {
 __io_putchar(*ptr++);
 }
return len;
}

int fputc(int ch, FILE *f)
{
	if( (char)ch == '\n' )
	{
		if( Line < MAX_LINE )
		{
			Line++;
		}
		Column = 0;
		return(0);
	}
	if( (char)ch == '\r' )
	{
		Column = 0;
		return(0);
	}


	LCD_DisplayChar(COLUMN(Column), LINE(Line), ch);


	if( Column >= MAX_COLUMN &&	Line < MAX_LINE )
	{
		Line++;
		Column=0;
	}
	else
	{
		Column++;
	}

	return (0);
}


/**
  * @}
  */ 

/**
  * @}
  */ 

/**
  * @}
  */ 

/**
  * @}
  */ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
