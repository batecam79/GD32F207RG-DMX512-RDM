/**
 * @file dmx4.h
 *
 */
/* Copyright (C) 2022-2024 by Arjan van Vught mailto:info@gd32-dmx.org
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef BOARD_DMX4_H_
#define BOARD_DMX4_H_

#if !defined(BOARD_DMX4)
# error This file should not be included
#endif

#if defined (MCU_GD32F20X_MCU_H_) || defined (MCU_GD32F407_MCU_H_)
# error This file should be included later
#endif

#if !(defined(CONSOLE_I2C) || defined(CONSOLE_NULL) || defined(CONFIG_USE_SOFTUART0))
# error USART0 is used for console
#endif

#include <stdint.h>

/**
 * LEDs
 */

#define LED1_GPIO_PINx		GPIO_PIN_0
#define LED1_GPIOx			GPIOC
#define LED1_RCU_GPIOx		RCU_GPIOC

#define LED2_GPIO_PINx		GPIO_PIN_2
#define LED2_GPIOx			GPIOC
#define LED2_RCU_GPIOx		RCU_GPIOC

#define LED3_GPIO_PINx		GPIO_PIN_3
#define LED3_GPIOx			GPIOC
#define LED3_RCU_GPIOx		RCU_GPIOC

#define LED_BLINK_PIN       LED1_GPIO_PINx
#define LED_BLINK_GPIO_PORT LED1_GPIOx
#define LED_BLINK_GPIO_CLK	LED1_RCU_GPIOx

/**
 * KEYs
 */

#define KEY2_PINx						GPIO_PIN_14
#define KEY2_GPIOx						GPIOA
#define KEY2_RCU_GPIOx					RCU_GPIOA

#define KEY_BOOTLOADER_TFTP_GPIO_PINx	KEY2_PINx
#define KEY_BOOTLOADER_TFTP_GPIOx		KEY2_GPIOx
#define KEY_BOOTLOADER_TFTP_RCU_GPIOx	KEY2_RCU_GPIOx

/**
 * I2C
 */

#define I2C0_REMAP
#define I2C_REMAP						GPIO_I2C0_REMAP
#define I2C_PERIPH						I2C0_PERIPH
#define I2C_RCU_I2Cx					I2C0_RCU_I2C0
#define I2C_GPIO_AFx					I2C0_GPIO_AFx
#define I2C_SCL_RCU_GPIOx				I2C0_SCL_RCU_GPIOx
#define I2C_SCL_GPIOx					I2C0_SCL_GPIOx
#define I2C_SCL_GPIO_PINx				I2C0_SCL_GPIO_PINx
#define I2C_SDA_RCU_GPIOx				I2C0_SDA_RCU_GPIOx
#define I2C_SDA_GPIOx					I2C0_SDA_GPIOx
#define I2C_SDA_GPIO_PINx				I2C0_SDA_GPIO_PINx

/**
 * SPI
 */

#define SPI_PERIPH			SPI2_PERIPH
#define SPI_RCU_SPIx		SPI2_RCU_SPI2
#define SPI_RCU_GPIOx		SPI2_RCU_GPIOx
#define SPI_GPIO_AFx		SPI2_GPIO_AFx
#define SPI_GPIOx			SPI2_GPIOx
#define SPI_SCK_GPIO_PINx	SPI2_SCK_GPIO_PINx
#define SPI_MISO_GPIO_PINx	SPI2_MISO_GPIO_PINx
#define SPI_MOSI_GPIO_PINx	SPI2_MOSI_GPIO_PINx
#define SPI_NSS_GPIOx		SPI2_NSS_GPIOx
#define SPI_NSS_RCU_GPIOx	SPI2_NSS_RCU_GPIOx
#define SPI_NSS_GPIO_PINx	SPI2_NSS_GPIO_PINx
#define SPI_DMAx			SPI2_DMAx
#define SPI_DMA_CHx			SPI2_TX_DMA_CHx
#define SPI_DMA_SUBPERIx	SPI2_TX_DMA_SUBPERIx

/**
 * U(S)ART
 */

#define USART0_REMAP
#define USART2_PARTIAL_REMAP

/**
 * Panel LEDs
 */

#ifdef __cplusplus
namespace hal {
namespace panelled {
static constexpr uint32_t ACTIVITY = 0;
static constexpr uint32_t ARTNET = 0;
static constexpr uint32_t DDP = 0;
static constexpr uint32_t SACN = 0;
static constexpr uint32_t LTC_IN = 0;
static constexpr uint32_t LTC_OUT = 0;
static constexpr uint32_t MIDI_IN = 0;
static constexpr uint32_t MIDI_OUT = 0;
static constexpr uint32_t OSC_IN = 0;
static constexpr uint32_t OSC_OUT = 0;
static constexpr uint32_t TCNET = 0;
// DMX
static constexpr uint32_t PORT_A_TX  = (1U << 0);
static constexpr uint32_t PORT_A_RX  = (1U << 8);
#define CONFIG_PANELLED_RDM_PORT
static constexpr uint32_t PORT_A_RDM = (1U << 16);
}  // namespace panelled
}  // namespace hal
#endif

#define PANELLED_595_COUNT			2
#define PANELLED_595_CS_GPIOx		GPIOA
#define PANELLED_595_CS_RCU_GPIOx	RCU_GPIOA
#define PANELLED_595_CS_GPIO_PINx	GPIO_PIN_15

/**
 * SPI flash
 */

#define SPI_FLASH_CS_RCU_GPIOx			RCU_GPIOB
#define SPI_FLASH_CS_GPIOx				GPIOB
#define SPI_FLASH_CS_GPIO_PINx			GPIO_PIN_1

/**
 * EXT PHY
 */

#define LINK_CHECK_GPIO_CLK				RCU_GPIOB
#define LINK_CHECK_GPIO_PORT			GPIOB
#define LINK_CHECK_GPIO_PIN 			GPIO_PIN_0
#define LINK_CHECK_EXTI_LINE			EXTI_0
#define LINK_CHECK_EXTI_IRQn			EXTI0_IRQn
#define LINK_CHECK_IRQ_HANDLE			EXTI0_IRQHandler
#if !defined (GD32F4XX)
# define LINK_CHECK_EXTI_CLK			RCU_AF
# define LINK_CHECK_EXTI_PORT_SOURCE	GPIO_PORT_SOURCE_GPIOB
# define LINK_CHECK_EXTI_PIN_SOURCE		GPIO_PIN_SOURCE_0
# define LINK_CHECK_EXTI_SOURCE_CONFIG	gpio_exti_source_select
# define LINK_CHECK_GPIO_CONFIG			gpio_init(LINK_CHECK_GPIO_PORT, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, LINK_CHECK_GPIO_PIN);
#else
# define LINK_CHECK_EXTI_CLK			RCU_SYSCFG
# define LINK_CHECK_EXTI_PORT_SOURCE	EXTI_SOURCE_GPIOB
# define LINK_CHECK_EXTI_PIN_SOURCE		EXTI_SOURCE_PIN0
# define LINK_CHECK_EXTI_SOURCE_CONFIG	syscfg_exti_line_config
# define LINK_CHECK_GPIO_CONFIG			gpio_mode_set(LINK_CHECK_GPIO_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, LINK_CHECK_GPIO_PIN);
#endif

/**
 * MCU and BOARD name
 */

#if defined (GD32F20X_CL)
# include "mcu/gd32f207_mcu.h"
# define GD32_MCU_NAME	"GD32F207RG"
#elif defined (GD32F407)
# include "mcu/gd32f407_mcu.h"
# define GD32_MCU_NAME	"GD32F407RE"
#else
# error MCU is not supported
#endif

#define GD32_BOARD_NAME			"DMX4"

#include "gd32_gpio.h"

#define GD32_BOARD_LED1			GD32_PORT_TO_GPIO(GD32_GPIO_PORTC, 0)
#define GD32_BOARD_LED2			GD32_PORT_TO_GPIO(GD32_GPIO_PORTC, 2)
#define GD32_BOARD_LED3			GD32_PORT_TO_GPIO(GD32_GPIO_PORTC, 3)
#define GD32_BOARD_STATUS_LED	GD32_BOARD_LED1

/**
 * LCD
 */

#define DISPLAYTIMEOUT_GPIO		GD32_PORT_TO_GPIO(GD32_GPIO_PORTA, 14)	// KEY2

/**
 * SPI LCD
 */

#define SPI_LCD_RST_PIN			GPIO_EXT_7
#define SPI_LCD_DC_GPIO 		GPIO_EXT_26
#define SPI_LCD_BL_GPIO			GPIO_EXT_22
#if defined(SPI_LCD_HAVE_CS_GPIO)
# define SPI_LCD_CS_GPIO		GPIO_EXT_24
#endif

#include "gpio_header.h"

#endif /* BOARD_DMX4_H_ */
