/*
 * Copyright (c) 2025 HX711 Pin Configuration created by GP
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef HX711_CONFIG_H
#define HX711_CONFIG_H

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/devicetree.h>

/* HX711 Pin Configuration - Easy to change for different hardware setups */

/* Get GPIO devices from device tree */
#define GPIO0_DEVICE DEVICE_DT_GET(DT_NODELABEL(gpio0))
#define GPIO1_DEVICE DEVICE_DT_GET(DT_NODELABEL(gpio1))

/* HX711 Sensor 0 - TENS_1 */
#define HX711_0_SCK_DEV GPIO1_DEVICE
#define HX711_0_SCK_PIN 11
#define HX711_0_SCK_FLAGS (GPIO_OUTPUT | GPIO_OUTPUT_LOW)

#define HX711_0_DOUT_DEV GPIO0_DEVICE
#define HX711_0_DOUT_PIN 29
#define HX711_0_DOUT_FLAGS (GPIO_INPUT | GPIO_PULL_UP)

#define HX711_0_RATE_DEV NULL
#define HX711_0_RATE_PIN 0
#define HX711_0_RATE_FLAGS 0

/* HX711 Sensor 1 - TENS_2 */
#define HX711_1_SCK_DEV GPIO1_DEVICE
#define HX711_1_SCK_PIN 15
#define HX711_1_SCK_FLAGS (GPIO_OUTPUT | GPIO_OUTPUT_LOW)

#define HX711_1_DOUT_DEV GPIO0_DEVICE
#define HX711_1_DOUT_PIN 3
#define HX711_1_DOUT_FLAGS (GPIO_INPUT | GPIO_PULL_UP)

#define HX711_1_RATE_DEV NULL
#define HX711_1_RATE_PIN 0
#define HX711_1_RATE_FLAGS 0

/* HX711 Sensor 2 - TENS_3 */
#define HX711_2_SCK_DEV GPIO1_DEVICE
#define HX711_2_SCK_PIN 14
#define HX711_2_SCK_FLAGS (GPIO_OUTPUT | GPIO_OUTPUT_LOW)

#define HX711_2_DOUT_DEV GPIO0_DEVICE
#define HX711_2_DOUT_PIN 28
#define HX711_2_DOUT_FLAGS (GPIO_INPUT | GPIO_PULL_UP)

#define HX711_2_RATE_DEV NULL
#define HX711_2_RATE_PIN 0
#define HX711_2_RATE_FLAGS 0

/* HX711 Configuration - Updated based on datasheet */
/* Note: Rate and gain are linked according to datasheet Table 1:
 * - 25 pulses: Channel A, Gain 128, 10 SPS
 * - 26 pulses: Channel B, Gain 32, 10 SPS  
 * - 27 pulses: Channel A, Gain 64, 80 SPS
 */
#define HX711_DEFAULT_GAIN 64      /* Default gain (will be set to 64 for 80 SPS) */
#define HX711_DEFAULT_RATE_SPS 80  /* Default data rate in SPS */
#define HX711_SLEEP_DELAY_US 70    /* >60µs for sleep mode */

/* Calibration values (to be determined experimentally) */
#define HX711_0_OFFSET 0
#define HX711_0_SCALE 1.0f

#define HX711_1_OFFSET 0
#define HX711_1_SCALE 1.0f

#define HX711_2_OFFSET 0
#define HX711_2_SCALE 1.0f

#endif /* HX711_CONFIG_H */ 