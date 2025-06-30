/*
 * Copyright (c) 2025 HX711 Pin Configuration created by GP
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef HX711_CONFIG_H
#define HX711_CONFIG_H

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/devicetree.h>

/* HX711 Pin Configuration - Easy to change for different hardware setups */

/* Get GPIO devices from device tree */
#define GPIO0_DEVICE DEVICE_DT_GET(DT_NODELABEL(gpio0))
#define GPIO1_DEVICE DEVICE_DT_GET(DT_NODELABEL(gpio1))

/* HX711 Sensor 0 - GPIO0 */
#define HX711_0_SCK_DEV GPIO0_DEVICE
#define HX711_0_SCK_PIN 2
#define HX711_0_SCK_FLAGS (GPIO_OUTPUT | GPIO_OUTPUT_LOW)

#define HX711_0_DOUT_DEV GPIO0_DEVICE
#define HX711_0_DOUT_PIN 3
#define HX711_0_DOUT_FLAGS (GPIO_INPUT | GPIO_PULL_UP)

#define HX711_0_RATE_DEV GPIO0_DEVICE
#define HX711_0_RATE_PIN 4
#define HX711_0_RATE_FLAGS (GPIO_OUTPUT | GPIO_OUTPUT_HIGH)

/* HX711 Sensor 1 - GPIO0 */
#define HX711_1_SCK_DEV GPIO0_DEVICE
#define HX711_1_SCK_PIN 5
#define HX711_1_SCK_FLAGS (GPIO_OUTPUT | GPIO_OUTPUT_LOW)

#define HX711_1_DOUT_DEV GPIO0_DEVICE
#define HX711_1_DOUT_PIN 6
#define HX711_1_DOUT_FLAGS (GPIO_INPUT | GPIO_PULL_UP)

#define HX711_1_RATE_DEV GPIO0_DEVICE
#define HX711_1_RATE_PIN 7
#define HX711_1_RATE_FLAGS (GPIO_OUTPUT | GPIO_OUTPUT_HIGH)

/* HX711 Sensor 2 - GPIO1 */
#define HX711_2_SCK_DEV GPIO1_DEVICE
#define HX711_2_SCK_PIN 0
#define HX711_2_SCK_FLAGS (GPIO_OUTPUT | GPIO_OUTPUT_LOW)

#define HX711_2_DOUT_DEV GPIO1_DEVICE
#define HX711_2_DOUT_PIN 1
#define HX711_2_DOUT_FLAGS (GPIO_INPUT | GPIO_PULL_UP)

#define HX711_2_RATE_DEV GPIO1_DEVICE
#define HX711_2_RATE_PIN 2
#define HX711_2_RATE_FLAGS (GPIO_OUTPUT | GPIO_OUTPUT_HIGH)

/* HX711 Configuration */
#define HX711_GAIN 128
#define HX711_SAMPLES_PER_SECOND 80
#define HX711_SLEEP_DELAY_US 70  /* >60µs for sleep mode */

/* Calibration values (to be determined experimentally) */
#define HX711_0_OFFSET 0
#define HX711_0_SCALE 1.0f

#define HX711_1_OFFSET 0
#define HX711_1_SCALE 1.0f

#define HX711_2_OFFSET 0
#define HX711_2_SCALE 1.0f

#endif /* HX711_CONFIG_H */ 