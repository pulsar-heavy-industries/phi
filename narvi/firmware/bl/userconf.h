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

/**
 * @file    userconf.h
 * @brief   Userlib configuration header.
 * @details Userlib configuration file, this file allows to enable or disable the
 *          various device drivers from your application. You may also use
 *          this file in order to override the device drivers default settings.
 *
 * @addtogroup USER_CONF
 * @{
 */

#ifndef _USERCONF_H_
#define _USERCONF_H_

/**
 * @brief   Enables the LCD subsystem.
 */
#if !defined(USERLIB_USE_LCD) || defined(__DOXYGEN__)
#define USERLIB_USE_LCD             TRUE
#endif

/*===========================================================================*/
/* LCD driver related settings.                                              */
/*===========================================================================*/

/**
 * @brief   Enables 4 BIT mode.
 * @note    Enabling this option LCD uses only D4 to D7 pins
 */
#define LCD_USE_4_BIT_MODE          TRUE

/**
 * @brief   Enables backlight APIs.
 * @note    Enabling this option LCD requires a PWM driver
 */
#define LCD_USE_DIMMABLE_BACKLIGHT  FALSE

#endif /* _USERCONF_H_ */

/** @} */
