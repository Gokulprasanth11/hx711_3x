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

/* Hardware test function */
void test_hardware_connections(void)
{
	printk("\n=== HARDWARE CONNECTION TEST ===\n");
	
	/* Test GPIO devices */
	const struct device *gpio0 = DEVICE_DT_GET(DT_NODELABEL(gpio0));
	const struct device *gpio1 = DEVICE_DT_GET(DT_NODELABEL(gpio1));
	
	if (!device_is_ready(gpio0)) {
		printk("ERROR: GPIO0 device not ready!\n");
		return;
	}
	printk("GPIO0 device is ready\n");
	
	if (!device_is_ready(gpio1)) {
		printk("ERROR: GPIO1 device not ready!\n");
		return;
	}
	printk("GPIO1 device is ready\n");
	
	/* Test SCK pins as outputs */
	printk("Testing SCK pins as outputs...\n");
	
	/* Configure and test SCK pins */
	gpio_pin_configure(gpio1, HX711_0_SCK_PIN, GPIO_OUTPUT | GPIO_OUTPUT_LOW);
	gpio_pin_configure(gpio1, HX711_1_SCK_PIN, GPIO_OUTPUT | GPIO_OUTPUT_LOW);
	gpio_pin_configure(gpio1, HX711_2_SCK_PIN, GPIO_OUTPUT | GPIO_OUTPUT_LOW);
	
	/* Test SCK pins high */
	gpio_pin_set(gpio1, HX711_0_SCK_PIN, 1);
	gpio_pin_set(gpio1, HX711_1_SCK_PIN, 1);
	gpio_pin_set(gpio1, HX711_2_SCK_PIN, 1);
	k_msleep(100);
	
	/* Test SCK pins low */
	gpio_pin_set(gpio1, HX711_0_SCK_PIN, 0);
	gpio_pin_set(gpio1, HX711_1_SCK_PIN, 0);
	gpio_pin_set(gpio1, HX711_2_SCK_PIN, 0);
	k_msleep(100);
	
	printk("SCK pins configured successfully\n");
	
	/* Test DOUT pins as inputs */
	printk("Testing DOUT pins as inputs...\n");
	
	gpio_pin_configure(gpio0, HX711_0_DOUT_PIN, GPIO_INPUT | GPIO_PULL_UP);
	gpio_pin_configure(gpio0, HX711_1_DOUT_PIN, GPIO_INPUT | GPIO_PULL_UP);
	gpio_pin_configure(gpio0, HX711_2_DOUT_PIN, GPIO_INPUT | GPIO_PULL_UP);
	
	/* Read DOUT pin states */
	int dout0 = gpio_pin_get(gpio0, HX711_0_DOUT_PIN);
	int dout1 = gpio_pin_get(gpio0, HX711_1_DOUT_PIN);
	int dout2 = gpio_pin_get(gpio0, HX711_2_DOUT_PIN);
	
	printk("DOUT pin states: %d %d %d\n", dout0, dout1, dout2);
	
	if (dout0 == 1 && dout1 == 1 && dout2 == 1) {
		printk("WARNING: All DOUT pins are HIGH - sensors may not be connected!\n");
		printk("Check your wiring:\n");
		printk("- VCC to 3.3V or 5V\n");
		printk("- GND to GND\n");
		printk("- DOUT to GPIO0 pins (with pull-up)\n");
		printk("- SCK to GPIO1 pins\n");
	} else {
		printk("Some sensors appear to be connected\n");
	}
	
	printk("=== END HARDWARE TEST ===\n\n");
}

/* Individual sensor test function */
void test_individual_sensors(void)
{
	printk("\n=== INDIVIDUAL SENSOR TEST ===\n");
	
	const struct device *gpio0 = DEVICE_DT_GET(DT_NODELABEL(gpio0));
	const struct device *gpio1 = DEVICE_DT_GET(DT_NODELABEL(gpio1));
	
	/* Test each sensor individually */
	for (int sensor = 0; sensor < 3; sensor++) {
		int sck_pin, dout_pin;
		const char *sensor_name;
		const struct device *sck_dev;
		
		switch(sensor) {
			case 0:
				sck_pin = HX711_0_SCK_PIN;
				dout_pin = HX711_0_DOUT_PIN;
				sensor_name = "Sensor 0";
				sck_dev = gpio1;
				break;
			case 1:
				sck_pin = HX711_1_SCK_PIN;
				dout_pin = HX711_1_DOUT_PIN;
				sensor_name = "Sensor 1";
				sck_dev = gpio1;
				break;
			case 2:
				sck_pin = HX711_2_SCK_PIN;
				dout_pin = HX711_2_DOUT_PIN;
				sensor_name = "Sensor 2";
				sck_dev = gpio1;
				break;
		}
		
		printk("\n--- Testing %s (SCK: GPIO1.%d, DOUT: GPIO0.%d) ---\n", 
		       sensor_name, sck_pin, dout_pin);
		
		/* Configure pins */
		gpio_pin_configure(sck_dev, sck_pin, GPIO_OUTPUT | GPIO_OUTPUT_LOW);
		gpio_pin_configure(gpio0, dout_pin, GPIO_INPUT | GPIO_PULL_UP);
		
		/* Read initial DOUT state */
		int initial_dout = gpio_pin_get(gpio0, dout_pin);
		printk("Initial DOUT state: %d\n", initial_dout);
		
		/* Test SCK control */
		gpio_pin_set(sck_dev, sck_pin, 1);
		k_msleep(50);
		gpio_pin_set(sck_dev, sck_pin, 0);
		k_msleep(50);
		
		/* Read DOUT after SCK pulse */
		int after_dout = gpio_pin_get(gpio0, dout_pin);
		printk("DOUT after SCK pulse: %d\n", after_dout);
		
		if (initial_dout == 1 && after_dout == 1) {
			printk("RESULT: %s appears to be disconnected or not powered\n", sensor_name);
			printk("TROUBLESHOOTING:\n");
			printk("1. Check VCC connection (3.3V or 5V)\n");
			printk("2. Check GND connection\n");
			printk("3. Check load cell wiring to HX711\n");
			printk("4. Check DOUT and SCK wire connections\n");
			printk("5. Try a different load cell\n");
		} else if (initial_dout == 0 || after_dout == 0) {
			printk("RESULT: %s appears to be connected and responding\n", sensor_name);
		}
	}
	
	printk("\n=== END INDIVIDUAL SENSOR TEST ===\n\n");
}

int main(void)
{
	int ret;
	int32_t value_0 = 0, value_1 = 0, value_2 = 0;  /* Initialize to 0 */
	uint32_t sample_count = 0;

	printk("HX711 Multi-Sensor Application Starting...\n");

	/* Run hardware connection test */
	test_hardware_connections();
	
	/* Run individual sensor test */
	test_individual_sensors();

	/* Initialize HX711 sensors using device tree GPIO definitions */
	ret = hx711_init(&hx711_0_data, 
	                 HX711_0_DOUT_DEV, HX711_0_DOUT_PIN, HX711_0_DOUT_FLAGS,
	                 HX711_0_SCK_DEV, HX711_0_SCK_PIN, HX711_0_SCK_FLAGS);
	if (ret < 0) {
		printk("Failed to initialize HX711 sensor 0: %d\n", ret);
		return -1;
	}

	ret = hx711_init(&hx711_1_data, 
	                 HX711_1_DOUT_DEV, HX711_1_DOUT_PIN, HX711_1_DOUT_FLAGS,
	                 HX711_1_SCK_DEV, HX711_1_SCK_PIN, HX711_1_SCK_FLAGS);
	if (ret < 0) {
		printk("Failed to initialize HX711 sensor 1: %d\n", ret);
		return -1;
	}

	ret = hx711_init(&hx711_2_data, 
	                 HX711_2_DOUT_DEV, HX711_2_DOUT_PIN, HX711_2_DOUT_FLAGS,
	                 HX711_2_SCK_DEV, HX711_2_SCK_PIN, HX711_2_SCK_FLAGS);
	if (ret < 0) {
		printk("Failed to initialize HX711 sensor 2: %d\n", ret);
		return -1;
	}

	/* Set all sensors to 80 SPS */
	hx711_set_rate(&hx711_0_data, 80);
	hx711_set_rate(&hx711_1_data, 80);
	hx711_set_rate(&hx711_2_data, 80);

	printk("All HX711 sensors initialized successfully\n");
	printk("Starting continuous reading...\n");
	printk("Format: [Sample] Sensor0 Sensor1 Sensor2\n");

	/* Main loop - continuous reading */
	while (1) {
		/* Debug: Print DOUT pin state for all sensors */
		int dout0 = gpio_pin_get(hx711_0_data.dout_dev, hx711_0_data.dout_pin);
		int dout1 = gpio_pin_get(hx711_1_data.dout_dev, hx711_1_data.dout_pin);
		int dout2 = gpio_pin_get(hx711_2_data.dout_dev, hx711_2_data.dout_pin);
		
		/* Only print when there's a change or every 100 iterations */
		static int last_dout0 = -1, last_dout1 = -1, last_dout2 = -1;
		static int print_counter = 0;
		
		if (dout0 != last_dout0 || dout1 != last_dout1 || dout2 != last_dout2 || 
		    (++print_counter % 100) == 0) {
			printk("DOUT0: %d DOUT1: %d DOUT2: %d\n", dout0, dout1, dout2);
			last_dout0 = dout0;
			last_dout1 = dout1;
			last_dout2 = dout2;
		}

		/* Check if any sensor has data ready (DOUT low) */
		if (dout0 == 0 || dout1 == 0 || dout2 == 0) {
			printk("*** DATA READY DETECTED! ***\n");
			
			/* Read from sensors that have data ready, keep previous values for others */
			if (dout0 == 0) {
				ret = hx711_read_raw(&hx711_0_data, &value_0);
				if (ret < 0) {
					printk("Error reading sensor 0: %d\n", ret);
					/* Keep previous value on error */
				}
			}
			/* If dout0 == 1, keep the previous value_0 */

			if (dout1 == 0) {
				ret = hx711_read_raw(&hx711_1_data, &value_1);
				if (ret < 0) {
					printk("Error reading sensor 1: %d\n", ret);
					/* Keep previous value on error */
				}
			}
			/* If dout1 == 1, keep the previous value_1 */

			if (dout2 == 0) {
				ret = hx711_read_raw(&hx711_2_data, &value_2);
				if (ret < 0) {
					printk("Error reading sensor 2: %d\n", ret);
					/* Keep previous value on error */
				}
			}
			/* If dout2 == 1, keep the previous value_2 */

			/* Print raw 24-bit values */
			printk("[%u] %d %d %d\n", sample_count++, value_0, value_1, value_2);
			k_msleep(1000);
		} else {
			/* No data ready, wait a bit */
			k_msleep(10);
		}
	}
} 