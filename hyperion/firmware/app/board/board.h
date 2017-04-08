/*
    ChibiOS - Copyright (C) 2006..2016 Giovanni Di Sirio

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
 * This file has been automatically generated using ChibiStudio board
 * generator plugin. Do not edit manually.
 */

#ifndef BOARD_H
#define BOARD_H

/*
 * Setup for STMicroelectronics STM32F4-Discovery board.
 */

/*
 * Board identifier.
 */
#define BOARD_ST_STM32F4_DISCOVERY
#define BOARD_NAME                  "STMicroelectronics STM32F4-Discovery"

/*
 * Board oscillators-related settings.
 * NOTE: LSE not fitted.
 */
#if !defined(STM32_LSECLK)
#define STM32_LSECLK                0U
#endif

#if !defined(STM32_HSECLK)
#define STM32_HSECLK                8000000U
#endif

/*
 * Board voltages.
 * Required for performance limits calculation.
 */
#define STM32_VDD                   300U

/*
 * MCU type as defined in the ST header.
 */
#define STM32F407xx

/*
 * IO pins assignments.
 */
#define GPIOA_FADER                 0U
#define GPIOA_POT1                  1U
#define GPIOA_POT3                  2U
#define GPIOA_POT5                  3U
#define GPIOA_POT6                  4U
#define GPIOA_POT7                  5U
#define GPIOA_POT4                  6U
#define GPIOA_POT2                  7U
#define GPIOA_PIN8                  8U
#define GPIO_PINA9                  9U
#define GPIO_PINA10                 10U
#define GPIO_PINA11                 11U
#define GPIO_PINA12                 12U
#define GPIOA_SWDIO                 13U
#define GPIOA_SWCLK                 14U
#define GPIOA_PIN15                 15U

#define GPIOB_PIN0                  0U
#define GPIOB_PIN1                  1U
#define GPIOB_PIN2                  2U
#define GPIOB_SPI1_SCK              3U
#define GPIOB_SPI1_MISO             4U
#define GPIOB_SPI1_MOSI             5U
#define GPIOB_LCD_CS                6U
#define GPIOB_LCD_DC                7U
#define GPIOB_CAN_RX                8U
#define GPIOB_CAN_TX                9U
#define GPIOB_PIN10                 10U
#define GPIOB_PIN11                 11U
#define GPIOB_PIN12                 12U
#define GPIOB_SPI2_SCK              13U
#define GPIOB_SPI2_MISO				14U
#define GPIOB_SPI2_MOSI             15U

#define GPIOC_PIN0                  0U
#define GPIOC_PIN1                  1U
#define GPIOC_SPI2_RC2              2U
#define GPIOC_SPI2_RC1              3U
#define GPIOC_PIN4                  4U
#define GPIOC_PIN5                  5U
#define GPIOC_PIN6                  6U
#define GPIOC_PIN7                  7U
#define GPIOC_PIN8                  8U
#define GPIOC_PIN9                  9U
#define GPIOC_UART_TX               10U
#define GPIOC_UART_RX               11U
#define GPIOC_LED                   12U
#define GPIOC_PIN13                 13U
#define GPIOC_PIN14                 14U
#define GPIOC_PIN15                 15U

#define GPIOD_PIN0                  0U
#define GPIOD_PIN1                  1U
#define GPIOD_PIN2                  2U
#define GPIOD_PIN3                  3U
#define GPIOD_PIN4                  4U
#define GPIOD_PIN5                  5U
#define GPIOD_PIN6                  6U
#define GPIOD_PIN7                  7U
#define GPIOD_PIN8                  8U
#define GPIOD_PIN9                  9U
#define GPIOD_PIN10                 10U
#define GPIOD_PIN11                 11U
#define GPIOD_PIN12                 12U
#define GPIOD_PIN13                 13U
#define GPIOD_PIN14                 14U
#define GPIOD_PIN15                 15U

#define GPIOE_PIN0                  0U
#define GPIOE_PIN1                  1U
#define GPIOE_PIN2                  2U
#define GPIOE_PIN3                  3U
#define GPIOE_PIN4                  4U
#define GPIOE_PIN5                  5U
#define GPIOE_PIN6                  6U
#define GPIOE_PIN7                  7U
#define GPIOE_PIN8                  8U
#define GPIOE_PIN9                  9U
#define GPIOE_PIN10                 10U
#define GPIOE_PIN11                 11U
#define GPIOE_PIN12                 12U
#define GPIOE_PIN13                 13U
#define GPIOE_PIN14                 14U
#define GPIOE_PIN15                 15U

#define GPIOF_PIN0                  0U
#define GPIOF_PIN1                  1U
#define GPIOF_PIN2                  2U
#define GPIOF_PIN3                  3U
#define GPIOF_PIN4                  4U
#define GPIOF_PIN5                  5U
#define GPIOF_PIN6                  6U
#define GPIOF_PIN7                  7U
#define GPIOF_PIN8                  8U
#define GPIOF_PIN9                  9U
#define GPIOF_PIN10                 10U
#define GPIOF_PIN11                 11U
#define GPIOF_PIN12                 12U
#define GPIOF_PIN13                 13U
#define GPIOF_PIN14                 14U
#define GPIOF_PIN15                 15U

#define GPIOG_PIN0                  0U
#define GPIOG_PIN1                  1U
#define GPIOG_PIN2                  2U
#define GPIOG_PIN3                  3U
#define GPIOG_PIN4                  4U
#define GPIOG_PIN5                  5U
#define GPIOG_PIN6                  6U
#define GPIOG_PIN7                  7U
#define GPIOG_PIN8                  8U
#define GPIOG_PIN9                  9U
#define GPIOG_PIN10                 10U
#define GPIOG_PIN11                 11U
#define GPIOG_PIN12                 12U
#define GPIOG_PIN13                 13U
#define GPIOG_PIN14                 14U
#define GPIOG_PIN15                 15U

#define GPIOH_OSC_IN                0U
#define GPIOH_OSC_OUT               1U
#define GPIOH_PIN2                  2U
#define GPIOH_PIN3                  3U
#define GPIOH_PIN4                  4U
#define GPIOH_PIN5                  5U
#define GPIOH_PIN6                  6U
#define GPIOH_PIN7                  7U
#define GPIOH_PIN8                  8U
#define GPIOH_PIN9                  9U
#define GPIOH_PIN10                 10U
#define GPIOH_PIN11                 11U
#define GPIOH_PIN12                 12U
#define GPIOH_PIN13                 13U
#define GPIOH_PIN14                 14U
#define GPIOH_PIN15                 15U

#define GPIOI_PIN0                  0U
#define GPIOI_PIN1                  1U
#define GPIOI_PIN2                  2U
#define GPIOI_PIN3                  3U
#define GPIOI_PIN4                  4U
#define GPIOI_PIN5                  5U
#define GPIOI_PIN6                  6U
#define GPIOI_PIN7                  7U
#define GPIOI_PIN8                  8U
#define GPIOI_PIN9                  9U
#define GPIOI_PIN10                 10U
#define GPIOI_PIN11                 11U
#define GPIOI_PIN12                 12U
#define GPIOI_PIN13                 13U
#define GPIOI_PIN14                 14U
#define GPIOI_PIN15                 15U


/*
 * I/O ports initial setup, this configuration is established soon after reset
 * in the initialization code.
 * Please refer to the STM32 Reference Manual for details.
 */
#define PIN_MODE_INPUT(n)           (0U << ((n) * 2U))
#define PIN_MODE_OUTPUT(n)          (1U << ((n) * 2U))
#define PIN_MODE_ALTERNATE(n)       (2U << ((n) * 2U))
#define PIN_MODE_ANALOG(n)          (3U << ((n) * 2U))
#define PIN_ODR_LOW(n)              (0U << (n))
#define PIN_ODR_HIGH(n)             (1U << (n))
#define PIN_OTYPE_PUSHPULL(n)       (0U << (n))
#define PIN_OTYPE_OPENDRAIN(n)      (1U << (n))
#define PIN_OSPEED_VERYLOW(n)       (0U << ((n) * 2U))
#define PIN_OSPEED_LOW(n)           (1U << ((n) * 2U))
#define PIN_OSPEED_MEDIUM(n)        (2U << ((n) * 2U))
#define PIN_OSPEED_HIGH(n)          (3U << ((n) * 2U))
#define PIN_PUPDR_FLOATING(n)       (0U << ((n) * 2U))
#define PIN_PUPDR_PULLUP(n)         (1U << ((n) * 2U))
#define PIN_PUPDR_PULLDOWN(n)       (2U << ((n) * 2U))
#define PIN_AFIO_AF(n, v)           ((v) << (((n) % 8U) * 4U))

/*
 * GPIOA setup:
 *
 */
#define VAL_GPIOA_MODER             (PIN_MODE_ANALOG(GPIOA_FADER) |        \
                                     PIN_MODE_ANALOG(GPIOA_POT1) |         \
                                     PIN_MODE_ANALOG(GPIOA_POT3) |         \
                                     PIN_MODE_ANALOG(GPIOA_POT5) |         \
                                     PIN_MODE_ANALOG(GPIOA_POT6) |         \
                                     PIN_MODE_ANALOG(GPIOA_POT7) |         \
                                     PIN_MODE_ANALOG(GPIOA_POT4) |         \
                                     PIN_MODE_ANALOG(GPIOA_POT2) |         \
                                     PIN_MODE_INPUT(GPIOA_PIN8) |          \
                                     PIN_MODE_INPUT(GPIO_PINA9) |          \
                                     PIN_MODE_INPUT(GPIO_PINA10) |         \
                                     PIN_MODE_INPUT(GPIO_PINA11) |         \
                                     PIN_MODE_INPUT(GPIO_PINA12) |         \
                                     PIN_MODE_ALTERNATE(GPIOA_SWDIO) |     \
                                     PIN_MODE_ALTERNATE(GPIOA_SWCLK) |     \
                                     PIN_MODE_INPUT(GPIOA_PIN15))
#define VAL_GPIOA_OTYPER            (PIN_OTYPE_PUSHPULL(GPIOA_FADER) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOA_POT3) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOA_POT1) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOA_POT5) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOA_POT6) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOA_POT7) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOA_POT4) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOA_POT2) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOA_PIN8) |      \
                                     PIN_OTYPE_PUSHPULL(GPIO_PINA9) |      \
                                     PIN_OTYPE_PUSHPULL(GPIO_PINA10) |     \
                                     PIN_OTYPE_PUSHPULL(GPIO_PINA11) |     \
                                     PIN_OTYPE_PUSHPULL(GPIO_PINA12) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOA_SWDIO) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOA_SWCLK) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOA_PIN15))
#define VAL_GPIOA_OSPEEDR           (PIN_OSPEED_HIGH(GPIOA_FADER) |        \
                                     PIN_OSPEED_HIGH(GPIOA_POT1) |         \
                                     PIN_OSPEED_HIGH(GPIOA_POT3) |         \
                                     PIN_OSPEED_HIGH(GPIOA_POT5) |         \
                                     PIN_OSPEED_HIGH(GPIOA_POT6) |         \
                                     PIN_OSPEED_HIGH(GPIOA_POT7) |         \
                                     PIN_OSPEED_HIGH(GPIOA_POT4) |         \
                                     PIN_OSPEED_HIGH(GPIOA_POT2) |         \
                                     PIN_OSPEED_HIGH(GPIOA_PIN8) |         \
                                     PIN_OSPEED_HIGH(GPIO_PINA9) |         \
                                     PIN_OSPEED_HIGH(GPIO_PINA10) |        \
                                     PIN_OSPEED_HIGH(GPIO_PINA11) |        \
                                     PIN_OSPEED_HIGH(GPIO_PINA12) |        \
                                     PIN_OSPEED_HIGH(GPIOA_SWDIO) |        \
                                     PIN_OSPEED_HIGH(GPIOA_SWCLK) |        \
                                     PIN_OSPEED_HIGH(GPIOA_PIN15))
#define VAL_GPIOA_PUPDR             (PIN_PUPDR_PULLUP(GPIOA_FADER) |       \
                                     PIN_PUPDR_PULLUP(GPIOA_POT1) |        \
                                     PIN_PUPDR_PULLUP(GPIOA_POT3) |        \
                                     PIN_PUPDR_PULLUP(GPIOA_POT5) |        \
                                     PIN_PUPDR_PULLUP(GPIOA_POT6) |        \
                                     PIN_PUPDR_PULLUP(GPIOA_POT7) |        \
                                     PIN_PUPDR_PULLUP(GPIOA_POT4) |        \
                                     PIN_PUPDR_PULLUP(GPIOA_POT2) |        \
                                     PIN_PUPDR_FLOATING(GPIOA_PIN8) |      \
                                     PIN_PUPDR_FLOATING(GPIO_PINA9) |      \
                                     PIN_PUPDR_FLOATING(GPIO_PINA10) |     \
                                     PIN_PUPDR_FLOATING(GPIO_PINA11) |     \
                                     PIN_PUPDR_FLOATING(GPIO_PINA12) |     \
                                     PIN_PUPDR_FLOATING(GPIOA_SWDIO) |     \
                                     PIN_PUPDR_FLOATING(GPIOA_SWCLK) |     \
                                     PIN_PUPDR_FLOATING(GPIOA_PIN15))
#define VAL_GPIOA_ODR               (PIN_ODR_HIGH(GPIOA_FADER) |           \
                                     PIN_ODR_HIGH(GPIOA_POT1) |            \
                                     PIN_ODR_HIGH(GPIOA_POT3) |            \
                                     PIN_ODR_HIGH(GPIOA_POT5) |            \
                                     PIN_ODR_HIGH(GPIOA_POT6) |            \
                                     PIN_ODR_HIGH(GPIOA_POT7) |            \
                                     PIN_ODR_HIGH(GPIOA_POT4) |            \
                                     PIN_ODR_HIGH(GPIOA_POT2) |            \
                                     PIN_ODR_HIGH(GPIOA_PIN8) |            \
                                     PIN_ODR_HIGH(GPIO_PINA9) |            \
                                     PIN_ODR_HIGH(GPIO_PINA10) |           \
                                     PIN_ODR_HIGH(GPIO_PINA11) |           \
                                     PIN_ODR_HIGH(GPIO_PINA12) |           \
                                     PIN_ODR_HIGH(GPIOA_SWDIO) |           \
                                     PIN_ODR_HIGH(GPIOA_SWCLK) |           \
                                     PIN_ODR_HIGH(GPIOA_PIN15))
#define VAL_GPIOA_AFRL              (PIN_AFIO_AF(GPIOA_FADER, 0U) |        \
                                     PIN_AFIO_AF(GPIOA_POT1, 0U) |         \
                                     PIN_AFIO_AF(GPIOA_POT3, 0U) |         \
                                     PIN_AFIO_AF(GPIOA_POT5, 0U) |         \
                                     PIN_AFIO_AF(GPIOA_POT6, 0U) |         \
                                     PIN_AFIO_AF(GPIOA_POT7, 0U) |         \
                                     PIN_AFIO_AF(GPIOA_POT4, 0U) |         \
                                     PIN_AFIO_AF(GPIOA_POT2, 0U))
#define VAL_GPIOA_AFRH              (PIN_AFIO_AF(GPIOA_PIN8, 0U) |         \
                                     PIN_AFIO_AF(GPIO_PINA9, 0U) |         \
                                     PIN_AFIO_AF(GPIO_PINA10, 0U) |        \
                                     PIN_AFIO_AF(GPIO_PINA11, 0U) |        \
                                     PIN_AFIO_AF(GPIO_PINA12, 0U) |        \
                                     PIN_AFIO_AF(GPIOA_SWDIO, 0U) |        \
                                     PIN_AFIO_AF(GPIOA_SWCLK, 0U) |        \
                                     PIN_AFIO_AF(GPIOA_PIN15, 0U))

/*
 * GPIOB setup:

 */
#define VAL_GPIOB_MODER             (PIN_MODE_INPUT(GPIOB_PIN0) |          \
                                     PIN_MODE_INPUT(GPIOB_PIN1) |          \
                                     PIN_MODE_INPUT(GPIOB_PIN2) |          \
                                     PIN_MODE_ALTERNATE(GPIOB_SPI1_SCK) |  \
                                     PIN_MODE_ALTERNATE(GPIOB_SPI1_MISO) | \
                                     PIN_MODE_ALTERNATE(GPIOB_SPI1_MOSI) | \
                                     PIN_MODE_OUTPUT(GPIOB_LCD_CS) |       \
                                     PIN_MODE_OUTPUT(GPIOB_LCD_DC) |       \
                                     PIN_MODE_INPUT(GPIOB_CAN_RX) |        \
                                     PIN_MODE_INPUT(GPIOB_CAN_TX) |        \
                                     PIN_MODE_INPUT(GPIOB_PIN10) |         \
                                     PIN_MODE_INPUT(GPIOB_PIN11) |         \
                                     PIN_MODE_INPUT(GPIOB_PIN12) |         \
                                     PIN_MODE_ALTERNATE(GPIOB_SPI2_SCK) |  \
                                     PIN_MODE_ALTERNATE(GPIOB_SPI2_MISO) | \
                                     PIN_MODE_ALTERNATE(GPIOB_SPI2_MOSI))
#define VAL_GPIOB_OTYPER            (PIN_OTYPE_PUSHPULL(GPIOB_PIN0) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOB_PIN1) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOB_PIN2) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOB_SPI1_SCK) |  \
                                     PIN_OTYPE_PUSHPULL(GPIOB_SPI1_MISO) | \
                                     PIN_OTYPE_PUSHPULL(GPIOB_SPI1_MOSI) | \
                                     PIN_OTYPE_PUSHPULL(GPIOB_LCD_CS) |    \
                                     PIN_OTYPE_PUSHPULL(GPIOB_LCD_DC) |    \
                                     PIN_OTYPE_PUSHPULL(GPIOB_CAN_RX) |    \
                                     PIN_OTYPE_PUSHPULL(GPIOB_CAN_TX) |    \
                                     PIN_OTYPE_PUSHPULL(GPIOB_PIN10) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOB_PIN11) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOB_PIN12) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOB_SPI2_SCK) |  \
                                     PIN_OTYPE_PUSHPULL(GPIOB_SPI2_MISO) | \
                                     PIN_OTYPE_PUSHPULL(GPIOB_SPI2_MOSI))
#define VAL_GPIOB_OSPEEDR           (PIN_OSPEED_HIGH(GPIOB_PIN0) |         \
                                     PIN_OSPEED_HIGH(GPIOB_PIN1) |         \
                                     PIN_OSPEED_HIGH(GPIOB_PIN2) |         \
                                     PIN_OSPEED_HIGH(GPIOB_SPI1_SCK) |     \
                                     PIN_OSPEED_HIGH(GPIOB_SPI1_MISO) |    \
                                     PIN_OSPEED_HIGH(GPIOB_SPI1_MOSI) |    \
                                     PIN_OSPEED_HIGH(GPIOB_LCD_CS) |       \
                                     PIN_OSPEED_HIGH(GPIOB_LCD_DC) |       \
                                     PIN_OSPEED_HIGH(GPIOB_CAN_RX) |       \
                                     PIN_OSPEED_HIGH(GPIOB_CAN_TX) |       \
                                     PIN_OSPEED_HIGH(GPIOB_PIN10) |        \
                                     PIN_OSPEED_HIGH(GPIOB_PIN11) |        \
                                     PIN_OSPEED_HIGH(GPIOB_PIN12) |        \
                                     PIN_OSPEED_HIGH(GPIOB_SPI2_SCK) |     \
                                     PIN_OSPEED_HIGH(GPIOB_SPI2_MISO) |    \
                                     PIN_OSPEED_HIGH(GPIOB_SPI2_MOSI))
#define VAL_GPIOB_PUPDR             (PIN_PUPDR_PULLUP(GPIOB_PIN0) |        \
                                     PIN_PUPDR_PULLUP(GPIOB_PIN1) |        \
                                     PIN_PUPDR_PULLUP(GPIOB_PIN2) |        \
                                     PIN_PUPDR_FLOATING(GPIOB_SPI1_SCK) |  \
                                     PIN_PUPDR_PULLUP(GPIOB_SPI1_MISO) |   \
                                     PIN_PUPDR_FLOATING(GPIOB_SPI1_MOSI) | \
                                     PIN_PUPDR_PULLUP(GPIOB_LCD_CS) |      \
                                     PIN_PUPDR_PULLUP(GPIOB_LCD_DC) |      \
                                     PIN_PUPDR_PULLUP(GPIOB_CAN_RX) |      \
                                     PIN_PUPDR_PULLUP(GPIOB_CAN_TX) |      \
                                     PIN_PUPDR_PULLUP(GPIOB_PIN10) |       \
                                     PIN_PUPDR_PULLUP(GPIOB_PIN11) |       \
                                     PIN_PUPDR_PULLUP(GPIOB_PIN12) |       \
                                     PIN_PUPDR_FLOATING(GPIOB_SPI2_SCK) |  \
                                     PIN_PUPDR_PULLUP(GPIOB_SPI2_MISO) |   \
                                     PIN_PUPDR_FLOATING(GPIOB_SPI2_MOSI))
#define VAL_GPIOB_ODR               (PIN_ODR_HIGH(GPIOB_PIN0) |            \
                                     PIN_ODR_HIGH(GPIOB_PIN1) |            \
                                     PIN_ODR_HIGH(GPIOB_PIN2) |            \
                                     PIN_ODR_HIGH(GPIOB_SPI1_SCK) |        \
                                     PIN_ODR_HIGH(GPIOB_SPI1_MISO) |       \
                                     PIN_ODR_HIGH(GPIOB_SPI1_MOSI) |       \
                                     PIN_ODR_HIGH(GPIOB_LCD_CS) |          \
                                     PIN_ODR_HIGH(GPIOB_LCD_DC) |          \
                                     PIN_ODR_HIGH(GPIOB_CAN_RX) |          \
                                     PIN_ODR_HIGH(GPIOB_CAN_TX) |          \
                                     PIN_ODR_HIGH(GPIOB_PIN10) |           \
                                     PIN_ODR_HIGH(GPIOB_PIN11) |           \
                                     PIN_ODR_HIGH(GPIOB_PIN12) |           \
                                     PIN_ODR_HIGH(GPIOB_SPI2_SCK) |        \
                                     PIN_ODR_HIGH(GPIOB_SPI2_MISO) |       \
                                     PIN_ODR_HIGH(GPIOB_SPI2_MOSI))
#define VAL_GPIOB_AFRL              (PIN_AFIO_AF(GPIOB_PIN0, 0U) |         \
                                     PIN_AFIO_AF(GPIOB_PIN1, 0U) |         \
                                     PIN_AFIO_AF(GPIOB_PIN2, 0U) |         \
                                     PIN_AFIO_AF(GPIOB_SPI1_SCK, 5U) |     \
                                     PIN_AFIO_AF(GPIOB_SPI1_MISO, 5U) |    \
                                     PIN_AFIO_AF(GPIOB_SPI1_MOSI, 5U) |    \
                                     PIN_AFIO_AF(GPIOB_LCD_CS, 0U) |       \
                                     PIN_AFIO_AF(GPIOB_LCD_DC, 0U))
#define VAL_GPIOB_AFRH              (PIN_AFIO_AF(GPIOB_CAN_RX, 0U) |       \
                                     PIN_AFIO_AF(GPIOB_CAN_TX, 0U) |       \
                                     PIN_AFIO_AF(GPIOB_PIN10, 0U) |        \
                                     PIN_AFIO_AF(GPIOB_PIN11, 0U) |        \
                                     PIN_AFIO_AF(GPIOB_PIN12, 0U) |        \
                                     PIN_AFIO_AF(GPIOB_SPI2_SCK, 5U) |     \
                                     PIN_AFIO_AF(GPIOB_SPI2_MISO, 5U) |    \
                                     PIN_AFIO_AF(GPIOB_SPI2_MOSI, 5U))

/*
 * GPIOC setup:

 */
#define VAL_GPIOC_MODER             (PIN_MODE_INPUT(GPIOC_PIN0) |          \
                                     PIN_MODE_INPUT(GPIOC_PIN1) |          \
                                     PIN_MODE_OUTPUT(GPIOC_SPI2_RC2) |     \
                                     PIN_MODE_OUTPUT(GPIOC_SPI2_RC1) |     \
                                     PIN_MODE_INPUT(GPIOC_PIN4) |          \
                                     PIN_MODE_INPUT(GPIOC_PIN5) |          \
                                     PIN_MODE_INPUT(GPIOC_PIN6) |          \
                                     PIN_MODE_INPUT(GPIOC_PIN7) |          \
                                     PIN_MODE_INPUT(GPIOC_PIN8) |          \
                                     PIN_MODE_INPUT(GPIOC_PIN9) |          \
                                     PIN_MODE_ALTERNATE(GPIOC_UART_TX) |   \
                                     PIN_MODE_ALTERNATE(GPIOC_UART_RX) |   \
                                     PIN_MODE_OUTPUT(GPIOC_LED) |          \
                                     PIN_MODE_INPUT(GPIOC_PIN13) |         \
                                     PIN_MODE_INPUT(GPIOC_PIN14) |         \
                                     PIN_MODE_INPUT(GPIOC_PIN15))
#define VAL_GPIOC_OTYPER            (PIN_OTYPE_PUSHPULL(GPIOC_PIN0) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN1) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOC_SPI2_RC2) |  \
                                     PIN_OTYPE_PUSHPULL(GPIOC_SPI2_RC1) |  \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN4) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN5) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN6) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN7) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN8) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN9) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOC_UART_TX) |   \
                                     PIN_OTYPE_PUSHPULL(GPIOC_UART_RX) |   \
                                     PIN_OTYPE_PUSHPULL(GPIOC_LED) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN13) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN14) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN15))
#define VAL_GPIOC_OSPEEDR           (PIN_OSPEED_HIGH(GPIOC_PIN0) |         \
                                     PIN_OSPEED_HIGH(GPIOC_PIN1) |         \
                                     PIN_OSPEED_HIGH(GPIOC_SPI2_RC2) |     \
                                     PIN_OSPEED_HIGH(GPIOC_SPI2_RC1) |     \
                                     PIN_OSPEED_HIGH(GPIOC_PIN4) |         \
                                     PIN_OSPEED_HIGH(GPIOC_PIN5) |         \
                                     PIN_OSPEED_HIGH(GPIOC_PIN6) |         \
                                     PIN_OSPEED_HIGH(GPIOC_PIN7) |         \
                                     PIN_OSPEED_HIGH(GPIOC_PIN8) |         \
                                     PIN_OSPEED_HIGH(GPIOC_PIN9) |         \
                                     PIN_OSPEED_HIGH(GPIOC_UART_TX) |      \
                                     PIN_OSPEED_HIGH(GPIOC_UART_RX) |      \
                                     PIN_OSPEED_HIGH(GPIOC_LED) |          \
                                     PIN_OSPEED_HIGH(GPIOC_PIN13) |        \
                                     PIN_OSPEED_HIGH(GPIOC_PIN14) |        \
                                     PIN_OSPEED_HIGH(GPIOC_PIN15))
#define VAL_GPIOC_PUPDR             (PIN_PUPDR_PULLUP(GPIOC_PIN0) |        \
                                     PIN_PUPDR_PULLUP(GPIOC_PIN1) |        \
                                     PIN_PUPDR_PULLUP(GPIOC_SPI2_RC2) |    \
                                     PIN_PUPDR_PULLUP(GPIOC_SPI2_RC1) |    \
                                     PIN_PUPDR_PULLUP(GPIOC_PIN4) |        \
                                     PIN_PUPDR_PULLUP(GPIOC_PIN5) |        \
                                     PIN_PUPDR_PULLUP(GPIOC_PIN6) |        \
                                     PIN_PUPDR_PULLUP(GPIOC_PIN7) |        \
                                     PIN_PUPDR_PULLUP(GPIOC_PIN8) |        \
                                     PIN_PUPDR_PULLUP(GPIOC_PIN9) |        \
                                     PIN_PUPDR_FLOATING(GPIOC_UART_TX) |   \
                                     PIN_PUPDR_FLOATING(GPIOC_UART_RX) |   \
                                     PIN_PUPDR_PULLUP(GPIOC_LED) |         \
                                     PIN_PUPDR_PULLUP(GPIOC_PIN13) |       \
                                     PIN_PUPDR_PULLUP(GPIOC_PIN14) |       \
                                     PIN_PUPDR_PULLUP(GPIOC_PIN15))
#define VAL_GPIOC_ODR               (PIN_ODR_HIGH(GPIOC_PIN0) |            \
                                     PIN_ODR_HIGH(GPIOC_PIN1) |            \
                                     PIN_ODR_HIGH(GPIOC_SPI2_RC2) |        \
                                     PIN_ODR_HIGH(GPIOC_SPI2_RC1) |        \
                                     PIN_ODR_HIGH(GPIOC_PIN4) |            \
                                     PIN_ODR_HIGH(GPIOC_PIN5) |            \
                                     PIN_ODR_HIGH(GPIOC_PIN6) |            \
                                     PIN_ODR_HIGH(GPIOC_PIN7) |            \
                                     PIN_ODR_HIGH(GPIOC_PIN8) |            \
                                     PIN_ODR_HIGH(GPIOC_PIN9) |            \
                                     PIN_ODR_HIGH(GPIOC_UART_TX) |         \
                                     PIN_ODR_HIGH(GPIOC_UART_RX) |         \
                                     PIN_ODR_HIGH(GPIOC_LED) |             \
                                     PIN_ODR_HIGH(GPIOC_PIN13) |           \
                                     PIN_ODR_HIGH(GPIOC_PIN14) |           \
                                     PIN_ODR_HIGH(GPIOC_PIN15))
#define VAL_GPIOC_AFRL              (PIN_AFIO_AF(GPIOC_PIN0, 0U) |         \
                                     PIN_AFIO_AF(GPIOC_PIN1, 0U) |         \
                                     PIN_AFIO_AF(GPIOC_SPI2_RC2, 0U) |     \
                                     PIN_AFIO_AF(GPIOC_SPI2_RC1, 0U) |     \
                                     PIN_AFIO_AF(GPIOC_PIN4, 0U) |         \
                                     PIN_AFIO_AF(GPIOC_PIN5, 0U) |         \
                                     PIN_AFIO_AF(GPIOC_PIN6, 0U) |         \
                                     PIN_AFIO_AF(GPIOC_PIN7, 0U))
#define VAL_GPIOC_AFRH              (PIN_AFIO_AF(GPIOC_PIN8, 0U) |         \
                                     PIN_AFIO_AF(GPIOC_PIN9, 0U) |         \
                                     PIN_AFIO_AF(GPIOC_UART_TX, 7U) |      \
                                     PIN_AFIO_AF(GPIOC_UART_RX, 7U) |      \
                                     PIN_AFIO_AF(GPIOC_LED, 0U) |          \
                                     PIN_AFIO_AF(GPIOC_PIN13, 0U) |        \
                                     PIN_AFIO_AF(GPIOC_PIN14, 0U) |        \
                                     PIN_AFIO_AF(GPIOC_PIN15, 0U))

/*
 * GPIOD setup:
 *
 */
#define VAL_GPIOD_MODER             (PIN_MODE_INPUT(GPIOD_PIN0) |          \
                                     PIN_MODE_INPUT(GPIOD_PIN1) |          \
                                     PIN_MODE_INPUT(GPIOD_PIN2) |          \
                                     PIN_MODE_INPUT(GPIOD_PIN3) |          \
                                     PIN_MODE_INPUT(GPIOD_PIN4) |          \
                                     PIN_MODE_INPUT(GPIOD_PIN5) |          \
                                     PIN_MODE_INPUT(GPIOD_PIN6) |          \
                                     PIN_MODE_INPUT(GPIOD_PIN7) |          \
                                     PIN_MODE_INPUT(GPIOD_PIN8) |          \
                                     PIN_MODE_INPUT(GPIOD_PIN9) |          \
                                     PIN_MODE_INPUT(GPIOD_PIN10) |         \
                                     PIN_MODE_INPUT(GPIOD_PIN11) |         \
                                     PIN_MODE_INPUT(GPIOD_PIN12) |         \
                                     PIN_MODE_INPUT(GPIOD_PIN13) |         \
                                     PIN_MODE_INPUT(GPIOD_PIN14) |         \
                                     PIN_MODE_INPUT(GPIOD_PIN15))
#define VAL_GPIOD_OTYPER            (PIN_OTYPE_PUSHPULL(GPIOD_PIN0) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOD_PIN1) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOD_PIN2) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOD_PIN3) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOD_PIN4) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOD_PIN5) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOD_PIN6) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOD_PIN7) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOD_PIN8) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOD_PIN9) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOD_PIN10) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOD_PIN11) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOD_PIN12) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOD_PIN13) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOD_PIN14) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOD_PIN15))
#define VAL_GPIOD_OSPEEDR           (PIN_OSPEED_HIGH(GPIOD_PIN0) |          \
                                     PIN_OSPEED_HIGH(GPIOD_PIN1) |          \
                                     PIN_OSPEED_HIGH(GPIOD_PIN2) |          \
                                     PIN_OSPEED_HIGH(GPIOD_PIN3) |          \
                                     PIN_OSPEED_HIGH(GPIOD_PIN4) |          \
                                     PIN_OSPEED_HIGH(GPIOD_PIN5) |          \
                                     PIN_OSPEED_HIGH(GPIOD_PIN6) |          \
                                     PIN_OSPEED_HIGH(GPIOD_PIN7) |          \
                                     PIN_OSPEED_HIGH(GPIOD_PIN8) |          \
                                     PIN_OSPEED_HIGH(GPIOD_PIN9) |          \
                                     PIN_OSPEED_HIGH(GPIOD_PIN10) |         \
                                     PIN_OSPEED_HIGH(GPIOD_PIN11) |         \
                                     PIN_OSPEED_HIGH(GPIOD_PIN12) |         \
                                     PIN_OSPEED_HIGH(GPIOD_PIN13) |         \
                                     PIN_OSPEED_HIGH(GPIOD_PIN14) |         \
                                     PIN_OSPEED_HIGH(GPIOD_PIN15))
#define VAL_GPIOD_PUPDR             (PIN_PUPDR_PULLUP(GPIOD_PIN0) |         \
                                     PIN_PUPDR_PULLUP(GPIOD_PIN1) |         \
                                     PIN_PUPDR_PULLUP(GPIOD_PIN2) |         \
                                     PIN_PUPDR_PULLUP(GPIOD_PIN3) |         \
                                     PIN_PUPDR_PULLUP(GPIOD_PIN4) |         \
                                     PIN_PUPDR_PULLUP(GPIOD_PIN5) |         \
                                     PIN_PUPDR_PULLUP(GPIOD_PIN6) |         \
                                     PIN_PUPDR_PULLUP(GPIOD_PIN7) |         \
                                     PIN_PUPDR_PULLUP(GPIOD_PIN8) |         \
                                     PIN_PUPDR_PULLUP(GPIOD_PIN9) |         \
                                     PIN_PUPDR_PULLUP(GPIOD_PIN10) |        \
                                     PIN_PUPDR_PULLUP(GPIOD_PIN11) |        \
                                     PIN_PUPDR_PULLUP(GPIOD_PIN12) |        \
                                     PIN_PUPDR_PULLUP(GPIOD_PIN13) |        \
                                     PIN_PUPDR_PULLUP(GPIOD_PIN14) |        \
                                     PIN_PUPDR_PULLUP(GPIOD_PIN15))
#define VAL_GPIOD_ODR               (PIN_ODR_HIGH(GPIOD_PIN0) |             \
                                     PIN_ODR_HIGH(GPIOD_PIN1) |             \
                                     PIN_ODR_HIGH(GPIOD_PIN2) |             \
                                     PIN_ODR_HIGH(GPIOD_PIN3) |             \
                                     PIN_ODR_HIGH(GPIOD_PIN4) |             \
                                     PIN_ODR_HIGH(GPIOD_PIN5) |             \
                                     PIN_ODR_HIGH(GPIOD_PIN6) |             \
                                     PIN_ODR_HIGH(GPIOD_PIN7) |             \
                                     PIN_ODR_HIGH(GPIOD_PIN8) |             \
                                     PIN_ODR_HIGH(GPIOD_PIN9) |             \
                                     PIN_ODR_HIGH(GPIOD_PIN10) |            \
                                     PIN_ODR_HIGH(GPIOD_PIN11) |            \
                                     PIN_ODR_HIGH(GPIOD_PIN12) |            \
                                     PIN_ODR_HIGH(GPIOD_PIN13) |            \
                                     PIN_ODR_HIGH(GPIOD_PIN14) |            \
                                     PIN_ODR_HIGH(GPIOD_PIN15))
#define VAL_GPIOD_AFRL              (PIN_AFIO_AF(GPIOD_PIN0, 0U) |          \
                                     PIN_AFIO_AF(GPIOD_PIN1, 0U) |          \
                                     PIN_AFIO_AF(GPIOD_PIN2, 0U) |          \
                                     PIN_AFIO_AF(GPIOD_PIN3, 0U) |          \
                                     PIN_AFIO_AF(GPIOD_PIN4, 0U) |          \
                                     PIN_AFIO_AF(GPIOD_PIN5, 0U) |          \
                                     PIN_AFIO_AF(GPIOD_PIN6, 0U) |          \
                                     PIN_AFIO_AF(GPIOD_PIN7, 0U))
#define VAL_GPIOD_AFRH              (PIN_AFIO_AF(GPIOD_PIN8, 0U) |          \
                                     PIN_AFIO_AF(GPIOD_PIN9, 0U) |          \
                                     PIN_AFIO_AF(GPIOD_PIN10, 0U) |         \
                                     PIN_AFIO_AF(GPIOD_PIN11, 0U) |         \
                                     PIN_AFIO_AF(GPIOD_PIN12, 0U) |         \
                                     PIN_AFIO_AF(GPIOD_PIN13, 0U) |         \
                                     PIN_AFIO_AF(GPIOD_PIN14, 0U) |         \
                                     PIN_AFIO_AF(GPIOD_PIN15, 0U))

/*
 * GPIOE setup:
 */
#define VAL_GPIOE_MODER             (PIN_MODE_INPUT(GPIOE_PIN0) |          \
                                     PIN_MODE_INPUT(GPIOE_PIN1) |          \
                                     PIN_MODE_INPUT(GPIOE_PIN2) |          \
                                     PIN_MODE_INPUT(GPIOE_PIN3) |          \
                                     PIN_MODE_INPUT(GPIOE_PIN4) |          \
                                     PIN_MODE_INPUT(GPIOE_PIN5) |          \
                                     PIN_MODE_INPUT(GPIOE_PIN6) |          \
                                     PIN_MODE_INPUT(GPIOE_PIN7) |          \
                                     PIN_MODE_INPUT(GPIOE_PIN8) |          \
                                     PIN_MODE_INPUT(GPIOE_PIN9) |          \
                                     PIN_MODE_INPUT(GPIOE_PIN10) |         \
                                     PIN_MODE_INPUT(GPIOE_PIN11) |         \
                                     PIN_MODE_INPUT(GPIOE_PIN12) |         \
                                     PIN_MODE_INPUT(GPIOE_PIN13) |         \
                                     PIN_MODE_INPUT(GPIOE_PIN14) |         \
                                     PIN_MODE_INPUT(GPIOE_PIN15))
#define VAL_GPIOE_OTYPER            (PIN_OTYPE_PUSHPULL(GPIOE_PIN0) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN1) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN2) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN3) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN4) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN5) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN6) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN7) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN8) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN9) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN10) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN11) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN12) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN13) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN14) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOE_PIN15))
#define VAL_GPIOE_OSPEEDR           (PIN_OSPEED_HIGH(GPIOE_PIN0) |         \
                                     PIN_OSPEED_HIGH(GPIOE_PIN1) |         \
                                     PIN_OSPEED_HIGH(GPIOE_PIN2) |         \
                                     PIN_OSPEED_HIGH(GPIOE_PIN3) |         \
                                     PIN_OSPEED_HIGH(GPIOE_PIN4) |         \
                                     PIN_OSPEED_HIGH(GPIOE_PIN5) |         \
                                     PIN_OSPEED_HIGH(GPIOE_PIN6) |         \
                                     PIN_OSPEED_HIGH(GPIOE_PIN7) |         \
                                     PIN_OSPEED_HIGH(GPIOE_PIN8) |         \
                                     PIN_OSPEED_HIGH(GPIOE_PIN9) |         \
                                     PIN_OSPEED_HIGH(GPIOE_PIN10) |        \
                                     PIN_OSPEED_HIGH(GPIOE_PIN11) |        \
                                     PIN_OSPEED_HIGH(GPIOE_PIN12) |        \
                                     PIN_OSPEED_HIGH(GPIOE_PIN13) |        \
                                     PIN_OSPEED_HIGH(GPIOE_PIN14) |        \
                                     PIN_OSPEED_HIGH(GPIOE_PIN15))
#define VAL_GPIOE_PUPDR             (PIN_PUPDR_PULLUP(GPIOE_PIN0) |        \
                                     PIN_PUPDR_PULLUP(GPIOE_PIN1) |        \
                                     PIN_PUPDR_PULLUP(GPIOE_PIN2) |        \
                                     PIN_PUPDR_PULLUP(GPIOE_PIN3) |        \
                                     PIN_PUPDR_PULLUP(GPIOE_PIN4) |        \
                                     PIN_PUPDR_PULLUP(GPIOE_PIN5) |        \
                                     PIN_PUPDR_PULLUP(GPIOE_PIN6) |        \
                                     PIN_PUPDR_PULLUP(GPIOE_PIN7) |        \
                                     PIN_PUPDR_PULLUP(GPIOE_PIN8) |        \
                                     PIN_PUPDR_PULLUP(GPIOE_PIN9) |        \
                                     PIN_PUPDR_PULLUP(GPIOE_PIN10) |       \
                                     PIN_PUPDR_PULLUP(GPIOE_PIN11) |       \
                                     PIN_PUPDR_PULLUP(GPIOE_PIN12) |       \
                                     PIN_PUPDR_PULLUP(GPIOE_PIN13) |       \
                                     PIN_PUPDR_PULLUP(GPIOE_PIN14) |       \
                                     PIN_PUPDR_PULLUP(GPIOE_PIN15))
#define VAL_GPIOE_ODR               (PIN_ODR_HIGH(GPIOE_PIN0) |            \
                                     PIN_ODR_HIGH(GPIOE_PIN1) |            \
                                     PIN_ODR_HIGH(GPIOE_PIN2) |            \
                                     PIN_ODR_HIGH(GPIOE_PIN3) |            \
                                     PIN_ODR_HIGH(GPIOE_PIN4) |            \
                                     PIN_ODR_HIGH(GPIOE_PIN5) |            \
                                     PIN_ODR_HIGH(GPIOE_PIN6) |            \
                                     PIN_ODR_HIGH(GPIOE_PIN7) |            \
                                     PIN_ODR_HIGH(GPIOE_PIN8) |            \
                                     PIN_ODR_HIGH(GPIOE_PIN9) |            \
                                     PIN_ODR_HIGH(GPIOE_PIN10) |           \
                                     PIN_ODR_HIGH(GPIOE_PIN11) |           \
                                     PIN_ODR_HIGH(GPIOE_PIN12) |           \
                                     PIN_ODR_HIGH(GPIOE_PIN13) |           \
                                     PIN_ODR_HIGH(GPIOE_PIN14) |           \
                                     PIN_ODR_HIGH(GPIOE_PIN15))
#define VAL_GPIOE_AFRL              (PIN_AFIO_AF(GPIOE_PIN0, 0U) |         \
                                     PIN_AFIO_AF(GPIOE_PIN1, 0U) |         \
                                     PIN_AFIO_AF(GPIOE_PIN2, 0U) |         \
                                     PIN_AFIO_AF(GPIOE_PIN3, 0U) |         \
                                     PIN_AFIO_AF(GPIOE_PIN4, 0U) |         \
                                     PIN_AFIO_AF(GPIOE_PIN5, 0U) |         \
                                     PIN_AFIO_AF(GPIOE_PIN6, 0U) |         \
                                     PIN_AFIO_AF(GPIOE_PIN7, 0U))
#define VAL_GPIOE_AFRH              (PIN_AFIO_AF(GPIOE_PIN8, 0U) |         \
                                     PIN_AFIO_AF(GPIOE_PIN9, 0U) |         \
                                     PIN_AFIO_AF(GPIOE_PIN10, 0U) |        \
                                     PIN_AFIO_AF(GPIOE_PIN11, 0U) |        \
                                     PIN_AFIO_AF(GPIOE_PIN12, 0U) |        \
                                     PIN_AFIO_AF(GPIOE_PIN13, 0U) |        \
                                     PIN_AFIO_AF(GPIOE_PIN14, 0U) |        \
                                     PIN_AFIO_AF(GPIOE_PIN15, 0U))

/*
 * GPIOF setup:
 */
#define VAL_GPIOF_MODER             (PIN_MODE_INPUT(GPIOF_PIN0) |           \
                                     PIN_MODE_INPUT(GPIOF_PIN1) |           \
                                     PIN_MODE_INPUT(GPIOF_PIN2) |           \
                                     PIN_MODE_INPUT(GPIOF_PIN3) |           \
                                     PIN_MODE_INPUT(GPIOF_PIN4) |           \
                                     PIN_MODE_INPUT(GPIOF_PIN5) |           \
                                     PIN_MODE_INPUT(GPIOF_PIN6) |           \
                                     PIN_MODE_INPUT(GPIOF_PIN7) |           \
                                     PIN_MODE_INPUT(GPIOF_PIN8) |           \
                                     PIN_MODE_INPUT(GPIOF_PIN9) |           \
                                     PIN_MODE_INPUT(GPIOF_PIN10) |          \
                                     PIN_MODE_INPUT(GPIOF_PIN11) |          \
                                     PIN_MODE_INPUT(GPIOF_PIN12) |          \
                                     PIN_MODE_INPUT(GPIOF_PIN13) |          \
                                     PIN_MODE_INPUT(GPIOF_PIN14) |          \
                                     PIN_MODE_INPUT(GPIOF_PIN15))
#define VAL_GPIOF_OTYPER            (PIN_OTYPE_PUSHPULL(GPIOF_PIN0) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN1) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN2) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN3) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN4) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN5) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN6) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN7) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN8) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN9) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN10) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN11) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN12) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN13) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN14) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN15))
#define VAL_GPIOF_OSPEEDR           (PIN_OSPEED_HIGH(GPIOF_PIN0) |          \
                                     PIN_OSPEED_HIGH(GPIOF_PIN1) |          \
                                     PIN_OSPEED_HIGH(GPIOF_PIN2) |          \
                                     PIN_OSPEED_HIGH(GPIOF_PIN3) |          \
                                     PIN_OSPEED_HIGH(GPIOF_PIN4) |          \
                                     PIN_OSPEED_HIGH(GPIOF_PIN5) |          \
                                     PIN_OSPEED_HIGH(GPIOF_PIN6) |          \
                                     PIN_OSPEED_HIGH(GPIOF_PIN7) |          \
                                     PIN_OSPEED_HIGH(GPIOF_PIN8) |          \
                                     PIN_OSPEED_HIGH(GPIOF_PIN9) |          \
                                     PIN_OSPEED_HIGH(GPIOF_PIN10) |         \
                                     PIN_OSPEED_HIGH(GPIOF_PIN11) |         \
                                     PIN_OSPEED_HIGH(GPIOF_PIN12) |         \
                                     PIN_OSPEED_HIGH(GPIOF_PIN13) |         \
                                     PIN_OSPEED_HIGH(GPIOF_PIN14) |         \
                                     PIN_OSPEED_HIGH(GPIOF_PIN15))
#define VAL_GPIOF_PUPDR             (PIN_PUPDR_FLOATING(GPIOF_PIN0) |       \
                                     PIN_PUPDR_FLOATING(GPIOF_PIN1) |       \
                                     PIN_PUPDR_FLOATING(GPIOF_PIN2) |       \
                                     PIN_PUPDR_FLOATING(GPIOF_PIN3) |       \
                                     PIN_PUPDR_FLOATING(GPIOF_PIN4) |       \
                                     PIN_PUPDR_FLOATING(GPIOF_PIN5) |       \
                                     PIN_PUPDR_FLOATING(GPIOF_PIN6) |       \
                                     PIN_PUPDR_FLOATING(GPIOF_PIN7) |       \
                                     PIN_PUPDR_FLOATING(GPIOF_PIN8) |       \
                                     PIN_PUPDR_FLOATING(GPIOF_PIN9) |       \
                                     PIN_PUPDR_FLOATING(GPIOF_PIN10) |      \
                                     PIN_PUPDR_FLOATING(GPIOF_PIN11) |      \
                                     PIN_PUPDR_FLOATING(GPIOF_PIN12) |      \
                                     PIN_PUPDR_FLOATING(GPIOF_PIN13) |      \
                                     PIN_PUPDR_FLOATING(GPIOF_PIN14) |      \
                                     PIN_PUPDR_FLOATING(GPIOF_PIN15))
#define VAL_GPIOF_ODR               (PIN_ODR_HIGH(GPIOF_PIN0) |             \
                                     PIN_ODR_HIGH(GPIOF_PIN1) |             \
                                     PIN_ODR_HIGH(GPIOF_PIN2) |             \
                                     PIN_ODR_HIGH(GPIOF_PIN3) |             \
                                     PIN_ODR_HIGH(GPIOF_PIN4) |             \
                                     PIN_ODR_HIGH(GPIOF_PIN5) |             \
                                     PIN_ODR_HIGH(GPIOF_PIN6) |             \
                                     PIN_ODR_HIGH(GPIOF_PIN7) |             \
                                     PIN_ODR_HIGH(GPIOF_PIN8) |             \
                                     PIN_ODR_HIGH(GPIOF_PIN9) |             \
                                     PIN_ODR_HIGH(GPIOF_PIN10) |            \
                                     PIN_ODR_HIGH(GPIOF_PIN11) |            \
                                     PIN_ODR_HIGH(GPIOF_PIN12) |            \
                                     PIN_ODR_HIGH(GPIOF_PIN13) |            \
                                     PIN_ODR_HIGH(GPIOF_PIN14) |            \
                                     PIN_ODR_HIGH(GPIOF_PIN15))
#define VAL_GPIOF_AFRL              (PIN_AFIO_AF(GPIOF_PIN0, 0U) |          \
                                     PIN_AFIO_AF(GPIOF_PIN1, 0U) |          \
                                     PIN_AFIO_AF(GPIOF_PIN2, 0U) |          \
                                     PIN_AFIO_AF(GPIOF_PIN3, 0U) |          \
                                     PIN_AFIO_AF(GPIOF_PIN4, 0U) |          \
                                     PIN_AFIO_AF(GPIOF_PIN5, 0U) |          \
                                     PIN_AFIO_AF(GPIOF_PIN6, 0U) |          \
                                     PIN_AFIO_AF(GPIOF_PIN7, 0U))
#define VAL_GPIOF_AFRH              (PIN_AFIO_AF(GPIOF_PIN8, 0U) |          \
                                     PIN_AFIO_AF(GPIOF_PIN9, 0U) |          \
                                     PIN_AFIO_AF(GPIOF_PIN10, 0U) |         \
                                     PIN_AFIO_AF(GPIOF_PIN11, 0U) |         \
                                     PIN_AFIO_AF(GPIOF_PIN12, 0U) |         \
                                     PIN_AFIO_AF(GPIOF_PIN13, 0U) |         \
                                     PIN_AFIO_AF(GPIOF_PIN14, 0U) |         \
                                     PIN_AFIO_AF(GPIOF_PIN15, 0U))

/*
 * GPIOG setup:
 */
#define VAL_GPIOG_MODER             (PIN_MODE_INPUT(GPIOG_PIN0) |           \
                                     PIN_MODE_INPUT(GPIOG_PIN1) |           \
                                     PIN_MODE_INPUT(GPIOG_PIN2) |           \
                                     PIN_MODE_INPUT(GPIOG_PIN3) |           \
                                     PIN_MODE_INPUT(GPIOG_PIN4) |           \
                                     PIN_MODE_INPUT(GPIOG_PIN5) |           \
                                     PIN_MODE_INPUT(GPIOG_PIN6) |           \
                                     PIN_MODE_INPUT(GPIOG_PIN7) |           \
                                     PIN_MODE_INPUT(GPIOG_PIN8) |           \
                                     PIN_MODE_INPUT(GPIOG_PIN9) |           \
                                     PIN_MODE_INPUT(GPIOG_PIN10) |          \
                                     PIN_MODE_INPUT(GPIOG_PIN11) |          \
                                     PIN_MODE_INPUT(GPIOG_PIN12) |          \
                                     PIN_MODE_INPUT(GPIOG_PIN13) |          \
                                     PIN_MODE_INPUT(GPIOG_PIN14) |          \
                                     PIN_MODE_INPUT(GPIOG_PIN15))
#define VAL_GPIOG_OTYPER            (PIN_OTYPE_PUSHPULL(GPIOG_PIN0) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOG_PIN1) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOG_PIN2) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOG_PIN3) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOG_PIN4) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOG_PIN5) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOG_PIN6) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOG_PIN7) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOG_PIN8) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOG_PIN9) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOG_PIN10) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOG_PIN11) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOG_PIN12) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOG_PIN13) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOG_PIN14) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOG_PIN15))
#define VAL_GPIOG_OSPEEDR           (PIN_OSPEED_HIGH(GPIOG_PIN0) |          \
                                     PIN_OSPEED_HIGH(GPIOG_PIN1) |          \
                                     PIN_OSPEED_HIGH(GPIOG_PIN2) |          \
                                     PIN_OSPEED_HIGH(GPIOG_PIN3) |          \
                                     PIN_OSPEED_HIGH(GPIOG_PIN4) |          \
                                     PIN_OSPEED_HIGH(GPIOG_PIN5) |          \
                                     PIN_OSPEED_HIGH(GPIOG_PIN6) |          \
                                     PIN_OSPEED_HIGH(GPIOG_PIN7) |          \
                                     PIN_OSPEED_HIGH(GPIOG_PIN8) |          \
                                     PIN_OSPEED_HIGH(GPIOG_PIN9) |          \
                                     PIN_OSPEED_HIGH(GPIOG_PIN10) |         \
                                     PIN_OSPEED_HIGH(GPIOG_PIN11) |         \
                                     PIN_OSPEED_HIGH(GPIOG_PIN12) |         \
                                     PIN_OSPEED_HIGH(GPIOG_PIN13) |         \
                                     PIN_OSPEED_HIGH(GPIOG_PIN14) |         \
                                     PIN_OSPEED_HIGH(GPIOG_PIN15))
#define VAL_GPIOG_PUPDR             (PIN_PUPDR_FLOATING(GPIOG_PIN0) |       \
                                     PIN_PUPDR_FLOATING(GPIOG_PIN1) |       \
                                     PIN_PUPDR_FLOATING(GPIOG_PIN2) |       \
                                     PIN_PUPDR_FLOATING(GPIOG_PIN3) |       \
                                     PIN_PUPDR_FLOATING(GPIOG_PIN4) |       \
                                     PIN_PUPDR_FLOATING(GPIOG_PIN5) |       \
                                     PIN_PUPDR_FLOATING(GPIOG_PIN6) |       \
                                     PIN_PUPDR_FLOATING(GPIOG_PIN7) |       \
                                     PIN_PUPDR_FLOATING(GPIOG_PIN8) |       \
                                     PIN_PUPDR_FLOATING(GPIOG_PIN9) |       \
                                     PIN_PUPDR_FLOATING(GPIOG_PIN10) |      \
                                     PIN_PUPDR_FLOATING(GPIOG_PIN11) |      \
                                     PIN_PUPDR_FLOATING(GPIOG_PIN12) |      \
                                     PIN_PUPDR_FLOATING(GPIOG_PIN13) |      \
                                     PIN_PUPDR_FLOATING(GPIOG_PIN14) |      \
                                     PIN_PUPDR_FLOATING(GPIOG_PIN15))
#define VAL_GPIOG_ODR               (PIN_ODR_HIGH(GPIOG_PIN0) |             \
                                     PIN_ODR_HIGH(GPIOG_PIN1) |             \
                                     PIN_ODR_HIGH(GPIOG_PIN2) |             \
                                     PIN_ODR_HIGH(GPIOG_PIN3) |             \
                                     PIN_ODR_HIGH(GPIOG_PIN4) |             \
                                     PIN_ODR_HIGH(GPIOG_PIN5) |             \
                                     PIN_ODR_HIGH(GPIOG_PIN6) |             \
                                     PIN_ODR_HIGH(GPIOG_PIN7) |             \
                                     PIN_ODR_HIGH(GPIOG_PIN8) |             \
                                     PIN_ODR_HIGH(GPIOG_PIN9) |             \
                                     PIN_ODR_HIGH(GPIOG_PIN10) |            \
                                     PIN_ODR_HIGH(GPIOG_PIN11) |            \
                                     PIN_ODR_HIGH(GPIOG_PIN12) |            \
                                     PIN_ODR_HIGH(GPIOG_PIN13) |            \
                                     PIN_ODR_HIGH(GPIOG_PIN14) |            \
                                     PIN_ODR_HIGH(GPIOG_PIN15))
#define VAL_GPIOG_AFRL              (PIN_AFIO_AF(GPIOG_PIN0, 0U) |          \
                                     PIN_AFIO_AF(GPIOG_PIN1, 0U) |          \
                                     PIN_AFIO_AF(GPIOG_PIN2, 0U) |          \
                                     PIN_AFIO_AF(GPIOG_PIN3, 0U) |          \
                                     PIN_AFIO_AF(GPIOG_PIN4, 0U) |          \
                                     PIN_AFIO_AF(GPIOG_PIN5, 0U) |          \
                                     PIN_AFIO_AF(GPIOG_PIN6, 0U) |          \
                                     PIN_AFIO_AF(GPIOG_PIN7, 0U))
#define VAL_GPIOG_AFRH              (PIN_AFIO_AF(GPIOG_PIN8, 0U) |          \
                                     PIN_AFIO_AF(GPIOG_PIN9, 0U) |          \
                                     PIN_AFIO_AF(GPIOG_PIN10, 0U) |         \
                                     PIN_AFIO_AF(GPIOG_PIN11, 0U) |         \
                                     PIN_AFIO_AF(GPIOG_PIN12, 0U) |         \
                                     PIN_AFIO_AF(GPIOG_PIN13, 0U) |         \
                                     PIN_AFIO_AF(GPIOG_PIN14, 0U) |         \
                                     PIN_AFIO_AF(GPIOG_PIN15, 0U))

/*
 * GPIOH setup:
 */
#define VAL_GPIOH_MODER             (PIN_MODE_INPUT(GPIOH_OSC_IN) |         \
                                     PIN_MODE_INPUT(GPIOH_OSC_OUT) |        \
                                     PIN_MODE_INPUT(GPIOH_PIN2) |           \
                                     PIN_MODE_INPUT(GPIOH_PIN3) |           \
                                     PIN_MODE_INPUT(GPIOH_PIN4) |           \
                                     PIN_MODE_INPUT(GPIOH_PIN5) |           \
                                     PIN_MODE_INPUT(GPIOH_PIN6) |           \
                                     PIN_MODE_INPUT(GPIOH_PIN7) |           \
                                     PIN_MODE_INPUT(GPIOH_PIN8) |           \
                                     PIN_MODE_INPUT(GPIOH_PIN9) |           \
                                     PIN_MODE_INPUT(GPIOH_PIN10) |          \
                                     PIN_MODE_INPUT(GPIOH_PIN11) |          \
                                     PIN_MODE_INPUT(GPIOH_PIN12) |          \
                                     PIN_MODE_INPUT(GPIOH_PIN13) |          \
                                     PIN_MODE_INPUT(GPIOH_PIN14) |          \
                                     PIN_MODE_INPUT(GPIOH_PIN15))
#define VAL_GPIOH_OTYPER            (PIN_OTYPE_PUSHPULL(GPIOH_OSC_IN) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOH_OSC_OUT) |    \
                                     PIN_OTYPE_PUSHPULL(GPIOH_PIN2) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOH_PIN3) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOH_PIN4) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOH_PIN5) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOH_PIN6) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOH_PIN7) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOH_PIN8) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOH_PIN9) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOH_PIN10) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOH_PIN11) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOH_PIN12) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOH_PIN13) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOH_PIN14) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOH_PIN15))
#define VAL_GPIOH_OSPEEDR           (PIN_OSPEED_HIGH(GPIOH_OSC_IN) |        \
                                     PIN_OSPEED_HIGH(GPIOH_OSC_OUT) |       \
                                     PIN_OSPEED_HIGH(GPIOH_PIN2) |          \
                                     PIN_OSPEED_HIGH(GPIOH_PIN3) |          \
                                     PIN_OSPEED_HIGH(GPIOH_PIN4) |          \
                                     PIN_OSPEED_HIGH(GPIOH_PIN5) |          \
                                     PIN_OSPEED_HIGH(GPIOH_PIN6) |          \
                                     PIN_OSPEED_HIGH(GPIOH_PIN7) |          \
                                     PIN_OSPEED_HIGH(GPIOH_PIN8) |          \
                                     PIN_OSPEED_HIGH(GPIOH_PIN9) |          \
                                     PIN_OSPEED_HIGH(GPIOH_PIN10) |         \
                                     PIN_OSPEED_HIGH(GPIOH_PIN11) |         \
                                     PIN_OSPEED_HIGH(GPIOH_PIN12) |         \
                                     PIN_OSPEED_HIGH(GPIOH_PIN13) |         \
                                     PIN_OSPEED_HIGH(GPIOH_PIN14) |         \
                                     PIN_OSPEED_HIGH(GPIOH_PIN15))
#define VAL_GPIOH_PUPDR             (PIN_PUPDR_FLOATING(GPIOH_OSC_IN) |     \
                                     PIN_PUPDR_FLOATING(GPIOH_OSC_OUT) |    \
                                     PIN_PUPDR_FLOATING(GPIOH_PIN2) |       \
                                     PIN_PUPDR_FLOATING(GPIOH_PIN3) |       \
                                     PIN_PUPDR_FLOATING(GPIOH_PIN4) |       \
                                     PIN_PUPDR_FLOATING(GPIOH_PIN5) |       \
                                     PIN_PUPDR_FLOATING(GPIOH_PIN6) |       \
                                     PIN_PUPDR_FLOATING(GPIOH_PIN7) |       \
                                     PIN_PUPDR_FLOATING(GPIOH_PIN8) |       \
                                     PIN_PUPDR_FLOATING(GPIOH_PIN9) |       \
                                     PIN_PUPDR_FLOATING(GPIOH_PIN10) |      \
                                     PIN_PUPDR_FLOATING(GPIOH_PIN11) |      \
                                     PIN_PUPDR_FLOATING(GPIOH_PIN12) |      \
                                     PIN_PUPDR_FLOATING(GPIOH_PIN13) |      \
                                     PIN_PUPDR_FLOATING(GPIOH_PIN14) |      \
                                     PIN_PUPDR_FLOATING(GPIOH_PIN15))
#define VAL_GPIOH_ODR               (PIN_ODR_HIGH(GPIOH_OSC_IN) |           \
                                     PIN_ODR_HIGH(GPIOH_OSC_OUT) |          \
                                     PIN_ODR_HIGH(GPIOH_PIN2) |             \
                                     PIN_ODR_HIGH(GPIOH_PIN3) |             \
                                     PIN_ODR_HIGH(GPIOH_PIN4) |             \
                                     PIN_ODR_HIGH(GPIOH_PIN5) |             \
                                     PIN_ODR_HIGH(GPIOH_PIN6) |             \
                                     PIN_ODR_HIGH(GPIOH_PIN7) |             \
                                     PIN_ODR_HIGH(GPIOH_PIN8) |             \
                                     PIN_ODR_HIGH(GPIOH_PIN9) |             \
                                     PIN_ODR_HIGH(GPIOH_PIN10) |            \
                                     PIN_ODR_HIGH(GPIOH_PIN11) |            \
                                     PIN_ODR_HIGH(GPIOH_PIN12) |            \
                                     PIN_ODR_HIGH(GPIOH_PIN13) |            \
                                     PIN_ODR_HIGH(GPIOH_PIN14) |            \
                                     PIN_ODR_HIGH(GPIOH_PIN15))
#define VAL_GPIOH_AFRL              (PIN_AFIO_AF(GPIOH_OSC_IN, 0U) |        \
                                     PIN_AFIO_AF(GPIOH_OSC_OUT, 0U) |       \
                                     PIN_AFIO_AF(GPIOH_PIN2, 0U) |          \
                                     PIN_AFIO_AF(GPIOH_PIN3, 0U) |          \
                                     PIN_AFIO_AF(GPIOH_PIN4, 0U) |          \
                                     PIN_AFIO_AF(GPIOH_PIN5, 0U) |          \
                                     PIN_AFIO_AF(GPIOH_PIN6, 0U) |          \
                                     PIN_AFIO_AF(GPIOH_PIN7, 0U))
#define VAL_GPIOH_AFRH              (PIN_AFIO_AF(GPIOH_PIN8, 0U) |          \
                                     PIN_AFIO_AF(GPIOH_PIN9, 0U) |          \
                                     PIN_AFIO_AF(GPIOH_PIN10, 0U) |         \
                                     PIN_AFIO_AF(GPIOH_PIN11, 0U) |         \
                                     PIN_AFIO_AF(GPIOH_PIN12, 0U) |         \
                                     PIN_AFIO_AF(GPIOH_PIN13, 0U) |         \
                                     PIN_AFIO_AF(GPIOH_PIN14, 0U) |         \
                                     PIN_AFIO_AF(GPIOH_PIN15, 0U))

/*
 * GPIOI setup:
 */
#define VAL_GPIOI_MODER             (PIN_MODE_INPUT(GPIOI_PIN0) |           \
                                     PIN_MODE_INPUT(GPIOI_PIN1) |           \
                                     PIN_MODE_INPUT(GPIOI_PIN2) |           \
                                     PIN_MODE_INPUT(GPIOI_PIN3) |           \
                                     PIN_MODE_INPUT(GPIOI_PIN4) |           \
                                     PIN_MODE_INPUT(GPIOI_PIN5) |           \
                                     PIN_MODE_INPUT(GPIOI_PIN6) |           \
                                     PIN_MODE_INPUT(GPIOI_PIN7) |           \
                                     PIN_MODE_INPUT(GPIOI_PIN8) |           \
                                     PIN_MODE_INPUT(GPIOI_PIN9) |           \
                                     PIN_MODE_INPUT(GPIOI_PIN10) |          \
                                     PIN_MODE_INPUT(GPIOI_PIN11) |          \
                                     PIN_MODE_INPUT(GPIOI_PIN12) |          \
                                     PIN_MODE_INPUT(GPIOI_PIN13) |          \
                                     PIN_MODE_INPUT(GPIOI_PIN14) |          \
                                     PIN_MODE_INPUT(GPIOI_PIN15))
#define VAL_GPIOI_OTYPER            (PIN_OTYPE_PUSHPULL(GPIOI_PIN0) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOI_PIN1) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOI_PIN2) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOI_PIN3) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOI_PIN4) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOI_PIN5) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOI_PIN6) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOI_PIN7) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOI_PIN8) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOI_PIN9) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOI_PIN10) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOI_PIN11) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOI_PIN12) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOI_PIN13) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOI_PIN14) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOI_PIN15))
#define VAL_GPIOI_OSPEEDR           (PIN_OSPEED_HIGH(GPIOI_PIN0) |          \
                                     PIN_OSPEED_HIGH(GPIOI_PIN1) |          \
                                     PIN_OSPEED_HIGH(GPIOI_PIN2) |          \
                                     PIN_OSPEED_HIGH(GPIOI_PIN3) |          \
                                     PIN_OSPEED_HIGH(GPIOI_PIN4) |          \
                                     PIN_OSPEED_HIGH(GPIOI_PIN5) |          \
                                     PIN_OSPEED_HIGH(GPIOI_PIN6) |          \
                                     PIN_OSPEED_HIGH(GPIOI_PIN7) |          \
                                     PIN_OSPEED_HIGH(GPIOI_PIN8) |          \
                                     PIN_OSPEED_HIGH(GPIOI_PIN9) |          \
                                     PIN_OSPEED_HIGH(GPIOI_PIN10) |         \
                                     PIN_OSPEED_HIGH(GPIOI_PIN11) |         \
                                     PIN_OSPEED_HIGH(GPIOI_PIN12) |         \
                                     PIN_OSPEED_HIGH(GPIOI_PIN13) |         \
                                     PIN_OSPEED_HIGH(GPIOI_PIN14) |         \
                                     PIN_OSPEED_HIGH(GPIOI_PIN15))
#define VAL_GPIOI_PUPDR             (PIN_PUPDR_FLOATING(GPIOI_PIN0) |       \
                                     PIN_PUPDR_FLOATING(GPIOI_PIN1) |       \
                                     PIN_PUPDR_FLOATING(GPIOI_PIN2) |       \
                                     PIN_PUPDR_FLOATING(GPIOI_PIN3) |       \
                                     PIN_PUPDR_FLOATING(GPIOI_PIN4) |       \
                                     PIN_PUPDR_FLOATING(GPIOI_PIN5) |       \
                                     PIN_PUPDR_FLOATING(GPIOI_PIN6) |       \
                                     PIN_PUPDR_FLOATING(GPIOI_PIN7) |       \
                                     PIN_PUPDR_FLOATING(GPIOI_PIN8) |       \
                                     PIN_PUPDR_FLOATING(GPIOI_PIN9) |       \
                                     PIN_PUPDR_FLOATING(GPIOI_PIN10) |      \
                                     PIN_PUPDR_FLOATING(GPIOI_PIN11) |      \
                                     PIN_PUPDR_FLOATING(GPIOI_PIN12) |      \
                                     PIN_PUPDR_FLOATING(GPIOI_PIN13) |      \
                                     PIN_PUPDR_FLOATING(GPIOI_PIN14) |      \
                                     PIN_PUPDR_FLOATING(GPIOI_PIN15))
#define VAL_GPIOI_ODR               (PIN_ODR_HIGH(GPIOI_PIN0) |             \
                                     PIN_ODR_HIGH(GPIOI_PIN1) |             \
                                     PIN_ODR_HIGH(GPIOI_PIN2) |             \
                                     PIN_ODR_HIGH(GPIOI_PIN3) |             \
                                     PIN_ODR_HIGH(GPIOI_PIN4) |             \
                                     PIN_ODR_HIGH(GPIOI_PIN5) |             \
                                     PIN_ODR_HIGH(GPIOI_PIN6) |             \
                                     PIN_ODR_HIGH(GPIOI_PIN7) |             \
                                     PIN_ODR_HIGH(GPIOI_PIN8) |             \
                                     PIN_ODR_HIGH(GPIOI_PIN9) |             \
                                     PIN_ODR_HIGH(GPIOI_PIN10) |            \
                                     PIN_ODR_HIGH(GPIOI_PIN11) |            \
                                     PIN_ODR_HIGH(GPIOI_PIN12) |            \
                                     PIN_ODR_HIGH(GPIOI_PIN13) |            \
                                     PIN_ODR_HIGH(GPIOI_PIN14) |            \
                                     PIN_ODR_HIGH(GPIOI_PIN15))
#define VAL_GPIOI_AFRL              (PIN_AFIO_AF(GPIOI_PIN0, 0U) |          \
                                     PIN_AFIO_AF(GPIOI_PIN1, 0U) |          \
                                     PIN_AFIO_AF(GPIOI_PIN2, 0U) |          \
                                     PIN_AFIO_AF(GPIOI_PIN3, 0U) |          \
                                     PIN_AFIO_AF(GPIOI_PIN4, 0U) |          \
                                     PIN_AFIO_AF(GPIOI_PIN5, 0U) |          \
                                     PIN_AFIO_AF(GPIOI_PIN6, 0U) |          \
                                     PIN_AFIO_AF(GPIOI_PIN7, 0U))
#define VAL_GPIOI_AFRH              (PIN_AFIO_AF(GPIOI_PIN8, 0U) |          \
                                     PIN_AFIO_AF(GPIOI_PIN9, 0U) |          \
                                     PIN_AFIO_AF(GPIOI_PIN10, 0U) |         \
                                     PIN_AFIO_AF(GPIOI_PIN11, 0U) |         \
                                     PIN_AFIO_AF(GPIOI_PIN12, 0U) |         \
                                     PIN_AFIO_AF(GPIOI_PIN13, 0U) |         \
                                     PIN_AFIO_AF(GPIOI_PIN14, 0U) |         \
                                     PIN_AFIO_AF(GPIOI_PIN15, 0U))


#if !defined(_FROM_ASM_)
#ifdef __cplusplus
extern "C" {
#endif
  void boardInit(void);
#ifdef __cplusplus
}
#endif
#endif /* _FROM_ASM_ */

#endif /* BOARD_H */
