/*
 * This file is part of Cleanflight.
 *
 * Cleanflight is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cleanflight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <stdbool.h>

#define TARGET_BOARD_IDENTIFIER "RADIX"

#define CONFIG_START_FLASH_ADDRESS (0x08004000) // 2nd 16kB sector

#define USBD_PRODUCT_STRING     "BrainFPV RADIX"

#define LED0                    PB13
#define LED0_INVERTED

#define LED1                    NONE

#define BEEPER
#define LED_STRIP

#define USE_ESCSERIAL
#define ESCSERIAL_TIMER_TX_HARDWARE 0 // PWM 1

#define USE_BRAINFPV_FPGA
#define BRAINFPV_FPGA_INCLUDE_BITSTREAM
#define BRAINFPVFPGA_SPI_INSTANCE SPI3
#define BRAINFPVFPGA_SPI_DIVISOR  8
#define BRAINFPVFPGA_CS_PIN       PC15
#define BRAINFPVFPGA_CDONE_PIN    PB12
#define BRAINFPVFPGA_CRESET_PIN   PB1
#define BRAINFPVFPGA_CLOCK_PIN    PA8
#define BRAINFPVFPGA_RESET_PIN    PB15

#define IDLE_COUNTS_PER_SEC_AT_NO_LOAD (16564455)

#define BRAINFPV
#define OSD
#define CMS
#define OSD_CALLS_CMS
#define USE_BRAINFPV_OSD
#define VIDEO_BITS_PER_PIXEL 2
#define INCLUDE_VIDEO_QUADSPI
#define VIDEO_QSPI_CLOCK_PIN PB2
#define VIDEO_QSPI_IO0_PIN   PC9
#define VIDEO_QSPI_IO1_PIN   PC10
#define VIDEO_VSYNC          PB5
#define VIDEO_HSYNC          PC2
#define BRAINFPV_OSD_SYNC_TH_DEFAULT 22
#define BRAINFPV_OSD_SYNC_TH_MIN 10
#define BRAINFPV_OSD_SYNC_TH_MAX 40


#define VTX_CONTROL
#define VTX_SMARTAUDIO

#define USE_BRAINFPV_SPECTROGRAPH

#define USE_EXTI
#define GYRO
#define ACC

#define USE_ACCGYRO_BMI160
#define GYRO_BMI160_ALIGN    CW0_DEG
#define ACC_BMI160_ALIGN     CW0_DEG
#define BMI160_SPI_INSTANCE  SPI1
#define BMI160_SPI_DIVISOR   16
#define BMI160_CS_PIN        PC14
#define BMI160_INT_EXTI      PC13

#define BARO
#define BARO_ZERO_ON_ARM
#define USE_BARO_BMP280
#define MS5611_I2C_INSTANCE     I2CDEV_1

#define USABLE_TIMER_CHANNEL_COUNT 7

#define USE_VCP
#define VBUS_SENSING_PIN        PA9
#define VBUS_SENSING_ENABLED

#define USE_UART1
#undef USE_UART1_TX_DMA
#define USE_UART1_TX_NODMA
#define UART1_RX_PIN            PB7
#define UART1_TX_PIN            PB6

#define USE_UART3
#define UART3_RX_PIN            PC5
#define UART3_TX_PIN            PB10

#define USE_UART4
#define UART4_RX_PIN            PA1
#define UART4_TX_PIN            PA0

#define USE_UART6
#undef USE_UART6_TX_DMA
#define UART6_RX_PIN            PC7
#define UART6_TX_PIN            PC6

#define SERIAL_PORT_COUNT       5 //VCP, USART1,  USART3, USART4, USART6

#define USE_SPI

#define USE_SPI_DEVICE_1
#define SPI1_SCK_PIN            PA5
#define SPI1_MISO_PIN           PA6
#define SPI1_MOSI_PIN           PA7

#define USE_SPI_DEVICE_3
#define SPI3_SCK_PIN            PB3
#define SPI3_MISO_PIN           PC11
#define SPI3_MOSI_PIN           PC12

#define USE_I2C
#define I2C_DEVICE              (I2CDEV_1)
#define USE_I2C_PULLUP
#define I2C1_SCL                PB8
#define I2C1_SDA                PB9

#define BOARD_HAS_VOLTAGE_DIVIDER
#define USE_ADC
#define VBAT_ADC_PIN            PC1
#define RSSI_ADC_PIN            PC3
#define CURRENT_METER_ADC_PIN   PC0
#define VBAT_SCALE_DEFAULT      110
#define CURRENT_SCALE_DEFAULT   200

#define DEFAULT_FEATURES        (FEATURE_VBAT | FEATURE_CURRENT_METER )
#define DEFAULT_RX_FEATURE      FEATURE_RX_PPM
#define SERIALRX_PROVIDER       SERIALRX_SBUS

#define SPEKTRUM_BIND
// PPM input
#define BIND_PIN                PB14

#define USE_SERIAL_4WAY_BLHELI_INTERFACE

#define TARGET_IO_PORTA         0xffff
#define TARGET_IO_PORTB         0xffff
#define TARGET_IO_PORTC         0xffff
#define TARGET_IO_PORTD         (BIT(2))

#define USE_DSHOT
#define USED_TIMERS             ( TIM_N(1) | TIM_N(2) | TIM_N(3) | TIM_N(5) | TIM_N(8) | TIM_N(12) )

bool brainfpv_settings_updated;
void brainFPVUpdateSettings(void);