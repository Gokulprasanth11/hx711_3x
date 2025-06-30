/*
 * Copyright (c) 2025 HX711 Multi-Sensor Application by GP
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include "hx711_driver.h"
#include "hx711_config.h"
#include <stdint.h>
#include <zephyr/devicetree.h>

/* HX711 sensor instances */
static struct hx711_data hx711_0_data;
static struct hx711_data hx711_1_data;
static struct hx711_data hx711_2_data;

int main(void)
{
	int ret;
	int32_t value_0, value_1, value_2;
	uint32_t sample_count = 0;

	printk("HX711 Multi-Sensor Application Starting...\n");

	/* Initialize HX711 sensors using device tree GPIO definitions */
	ret = hx711_init(&hx711_0_data, 
	                 HX711_0_DOUT_DEV, HX711_0_DOUT_PIN, HX711_0_DOUT_FLAGS,
	                 HX711_0_SCK_DEV, HX711_0_SCK_PIN, HX711_0_SCK_FLAGS,
	                 HX711_0_RATE_DEV, HX711_0_RATE_PIN, HX711_0_RATE_FLAGS);
	if (ret < 0) {
		printk("Failed to initialize HX711 sensor 0: %d\n", ret);
		return -1;
	}

	ret = hx711_init(&hx711_1_data, 
	                 HX711_1_DOUT_DEV, HX711_1_DOUT_PIN, HX711_1_DOUT_FLAGS,
	                 HX711_1_SCK_DEV, HX711_1_SCK_PIN, HX711_1_SCK_FLAGS,
	                 HX711_1_RATE_DEV, HX711_1_RATE_PIN, HX711_1_RATE_FLAGS);
	if (ret < 0) {
		printk("Failed to initialize HX711 sensor 1: %d\n", ret);
		return -1;
	}

	ret = hx711_init(&hx711_2_data, 
	                 HX711_2_DOUT_DEV, HX711_2_DOUT_PIN, HX711_2_DOUT_FLAGS,
	                 HX711_2_SCK_DEV, HX711_2_SCK_PIN, HX711_2_SCK_FLAGS,
	                 HX711_2_RATE_DEV, HX711_2_RATE_PIN, HX711_2_RATE_FLAGS);
	if (ret < 0) {
		printk("Failed to initialize HX711 sensor 2: %d\n", ret);
		return -1;
	}

	/* Set all sensors to 80 SPS */
	hx711_set_rate(&hx711_0_data, 80);
	hx711_set_rate(&hx711_1_data, 80);
	hx711_set_rate(&hx711_2_data, 80);

	printk("All HX711 sensors initialized successfully with rate pins\n");
	printk("Starting continuous reading at 80 SPS...\n");
	printk("Format: [Sample] Sensor0 Sensor1 Sensor2\n");

	/* Main loop - continuous reading */
	while (1) {
		/* Debug: Print DOUT pin state for all sensors */
		printk("DOUT0: %d DOUT1: %d DOUT2: %d\n",
			gpio_pin_get(hx711_0_data.dout_dev, hx711_0_data.dout_pin),
			gpio_pin_get(hx711_1_data.dout_dev, hx711_1_data.dout_pin),
			gpio_pin_get(hx711_2_data.dout_dev, hx711_2_data.dout_pin));

		/* Read from all three sensors */
		ret = hx711_read_raw(&hx711_0_data, &value_0);
		if (ret < 0) {
			printk("Error reading sensor 0: %d\n", ret);
		}

		ret = hx711_read_raw(&hx711_1_data, &value_1);
		if (ret < 0) {
			printk("Error reading sensor 1: %d\n", ret);
		}

		ret = hx711_read_raw(&hx711_2_data, &value_2);
		if (ret < 0) {
			printk("Error reading sensor 2: %d\n", ret);
		}

		/* Print raw 24-bit values */
		printk("[%u] %d %d %d\n", sample_count++, value_0, value_1, value_2);
	}
}