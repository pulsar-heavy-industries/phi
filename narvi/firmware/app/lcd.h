/*
    PLAY Embedded - Copyright (C) 2006..2015 Rocco Marco Guglielmi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

/*
    Special thanks to Giovanni Di Sirio for teachings, his moral support and
    friendship. Note that some or every piece of this file could be part of
    the ChibiOS project that is intellectual property of Giovanni Di Sirio.
    Please refer to ChibiOS/RT license before use this file.
	
	For suggestion or Bug report - guglielmir@playembedded.org
 */

/**
 * @file    lcd.h
 * @brief   LCD Complex Driver header.
 *
 * @addtogroup LCD
 * @{
 */

#ifndef _LCD_H_
#define _LCD_H_

#include "hal.h"
#include "userconf.h"

#if USERLIB_USE_LCD || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/


/**
 * @brief   Generic definition of right direction
 */
#define   LCD_RIGHT                     0x04

/**
 * @brief   Generic definition of left direction
 */
#define   LCD_LEFT                      0x00

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @brief   Enables back-light APIs.
 *
 * @note    Enabling this option LCD requires a PWM driver.
 * @note    The default is @p TRUE.
 */
#if !defined(LCD_USE_DIMMABLE_BACKLIGHT) || defined(__DOXYGEN__)
#define LCD_USE_DIMMABLE_BACKLIGHT               TRUE
#endif

/**
 * @brief   Enables 4 BIT mode.
 *
 * @note    Enabling this option LCD uses only D4 to D7 pins
 * @note    The default is @p FALSE.
 */
#if !defined(LCD_USE_4_BIT_MODE) || defined(__DOXYGEN__)
#define LCD_USE_4_BIT_MODE              FALSE
#endif

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if LCD_USE_4_BIT_MODE
#define LINE_DATA_LEN                   4
#define LCD_DATA_LENGHT                 0x00
#else
#define LINE_DATA_LEN                   8
#define LCD_DATA_LENGHT                 0x10
#endif

#if LCD_USE_DIMMABLE_BACKLIGHT && !HAL_USE_PWM
#error "LCD_USE_DIMMABLE_BACKLIGHT requires HAL_USE_PWM"
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @name    lcd data structures and types
 * @{ */

/**
 * @brief   lcd PIN-map.
 * @note    these PINs are required by write/read operation.
 */
typedef struct {
  /**
   * @brief  Register selector PIN
   */
  ioline_t RS;
  /**
   * @brief  Read/Write PIN
   */
  ioline_t RW;
  /**
   * @brief  Enable PIN
   */
  ioline_t E;
  /**
   * @brief  Back-light anode pin
   */
  ioline_t A;
  /**
   * @brief  Data PINs
   */
  ioline_t D[LINE_DATA_LEN];
} lcd_pins_t;

/**
 * @brief  LCD cursor control
 */
typedef enum {
  LCD_CURSOR_OFF = 0x00,
  LCD_CURSOR_ON = 0x02,
} lcd_cursor_t;

/**
 * @brief  LCD blinking control
 */
typedef enum {
  LCD_BLINKING_OFF = 0x00,
  LCD_BLINKING_ON = 0x01,
} lcd_blinking_t;

/**
 * @brief  LCD display settings
 */
typedef enum {
  LCD_SET_FONT_5X8 = 0x00,
  LCD_SET_FONT_5X10 = 0x04
} lcd_set_font_t;

/**
 * @brief  LCD display settings
 */
typedef enum {
  LCD_SET_1LINE = 0x00,
  LCD_SET_2LINES = 0x08
} lcd_set_lines_t;
/** @}  */

/**
 * @brief   Driver state machine possible states.
 */
typedef enum {
  LCD_UNINIT = 0,
  LCD_STOP = 1,
  LCD_ACTIVE = 2,
} lcd_state_t;

/**
 * @brief   LCD configuration structure.
 */
typedef struct {
  /**
   * @brief  LCD cursor control
   */
  lcd_cursor_t cursor;
  /**
   * @brief  LCD blinking control
   */
  lcd_blinking_t blinking;
  /**
   * @brief  LCD font setting
   */
  lcd_set_font_t font;
  /**
   * @brief  LCD lines settings
   */
  lcd_set_lines_t lines;
  /**
   * @brief  LCD PIN-map
   */
  lcd_pins_t const *pinmap;
#if LCD_USE_DIMMABLE_BACKLIGHT
  /**
   * @brief  PWM driver for back-light managing
   */
  PWMDriver *pwmp;
  /**
   * @brief  PWM driver configuration
   */
  PWMConfig const *pwmcfgp;
  /**
   * @brief  PWM channel id
   */
  uint32_t channelid;
#endif
  /**
   * @brief  Initial Back-light percentage (from 0 to 100)
   * @note   If !LCD_USE_DIMMABLE_BACKLIGHT this is just true or false
   */
  uint32_t backlight;
} LCDConfig;

/**
 * @brief   Structure representing an LCD driver.
 */
typedef struct {
  /**
   * @brief Driver state.
   */
  lcd_state_t         state;
  /**
   * @brief  Current Back-light percentage (from 0 to 100)
   *
   * @detail When LCD_USE_DIMMABLE_BACKLIGHT is false, this is considered like boolean
   */
  uint32_t           backlight;
  /**
   * @brief Current configuration data.
   */
  const LCDConfig    *config;
} LCDDriver;

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

extern LCDDriver LCDD1;

#ifdef __cplusplus
extern "C" {
#endif
  void lcdInit(void);
  void lcdObjectInit(LCDDriver *lcdp);
  void lcdStart(LCDDriver *lcdp, const LCDConfig *config);
  void lcdStop(LCDDriver *lcdp);
  void lcdBacklightOn(LCDDriver *lcdp);
  void lcdBacklightOff(LCDDriver *lcdp);
  void lcdClearDisplay(LCDDriver *lcdp);
  void lcdReturnHome(LCDDriver *lcdp);
  void lcdSetAddress(LCDDriver *lcdp, uint8_t add);
  void lcdWriteString(LCDDriver *lcdp, char* string, uint8_t pos);
  void lcdDoDisplayShift(LCDDriver *lcdp, uint8_t dir);
#if LCD_USE_DIMMABLE_BACKLIGHT
  void lcdSetBacklight(LCDDriver *lcdp, uint32_t perc);
  void lcdBacklightFadeOut(LCDDriver *lcdp);
  void lcdBacklightFadeIn(LCDDriver *lcdp);
#endif /* LCD_USE_DIMMABLE_BACKLIGHT */
#ifdef __cplusplus
}
#endif
#endif /* USERLIB_USE_LCD */
#endif /* _LCD_H_ */

/** @} */
