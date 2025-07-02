/*
 * Copyright (c) 2025 HX711 Multi-Sensor Application by GP
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <zephyr/logging/log.h>
#include "hx711_driver.h"
#include "hx711_config.h"
#include <stdint.h>
#include <zephyr/devicetree.h>
#include <stdio.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

/* HX711 sensor instances */
static struct hx711_data hx711_0_data;
static struct hx711_data hx711_1_data;
static struct hx711_data hx711_2_data;

/* Power management */
#define INACTIVITY_TIMEOUT_MS 60000
static int64_t last_activity_time = 0;
static bool hx711_powered = true;

/* Calibration data */
static float calibration_scales[3] = {1.0f, 1.0f, 1.0f};
static int32_t calibration_offsets[3] = {0, 0, 0};

/* Power management functions */
static void force_sck_low(void)
{
	const struct device *gpio1 = DEVICE_DT_GET(DT_NODELABEL(gpio1));
	gpio_pin_configure(gpio1, HX711_0_SCK_PIN, GPIO_OUTPUT_LOW);
	gpio_pin_set(gpio1, HX711_0_SCK_PIN, 0);
	LOG_INF("SCK forced LOW");
}

static void power_up_reset(void)
{
	const struct device *gpio1 = DEVICE_DT_GET(DT_NODELABEL(gpio1));
	gpio_pin_set(gpio1, HX711_0_SCK_PIN, 1);
	k_busy_wait(100);
	gpio_pin_set(gpio1, HX711_0_SCK_PIN, 0);
	k_busy_wait(10);
	LOG_INF("HX711 power-up reset complete");
}

static void disable_hx711_power(void)
{
	/* Set all SCK pins high to enter sleep mode */
	hx711_sleep(&hx711_0_data);
	hx711_sleep(&hx711_1_data);
	hx711_sleep(&hx711_2_data);
	hx711_powered = false;
	LOG_INF("HX711 powered OFF due to inactivity");
}

static void enable_hx711_power(void)
{
	/* Wake up all sensors */
	hx711_wake_up(&hx711_0_data);
	hx711_wake_up(&hx711_1_data);
	hx711_wake_up(&hx711_2_data);
	hx711_powered = true;
	LOG_INF("HX711 powered ON");
}

/* Calibration functions */
static void tare_sensor(struct hx711_data *hx711, int sensor_id, int samples)
{
	int32_t sum = 0;
	int32_t value;
	
	LOG_INF("Taring HX711_%d...", sensor_id);
	
	for (int i = 0; i < samples; i++) {
		if (hx711_read_raw(hx711, &value) == 0) {
			sum += value;
		}
		k_msleep(10);
	}
	
	calibration_offsets[sensor_id] = sum / samples;
	LOG_INF("HX711_%d offset: %d", sensor_id, calibration_offsets[sensor_id]);
}

static void calibrate_sensor(struct hx711_data *hx711, int sensor_id, float known_weight, int samples)
{
	int32_t sum = 0;
	int32_t value;
	
	LOG_INF("Calibrating HX711_%d with %.1fg...", sensor_id, (double)known_weight);
	
	for (int i = 0; i < samples; i++) {
		if (hx711_read_raw(hx711, &value) == 0) {
			sum += value;
		}
		k_msleep(10);
	}
	
	int32_t avg_raw = sum / samples;
	int32_t raw_diff = avg_raw - calibration_offsets[sensor_id];
	
	if (raw_diff != 0) {
		calibration_scales[sensor_id] = known_weight / (float)raw_diff;
		LOG_INF("HX711_%d scale: %.6f", sensor_id, (double)calibration_scales[sensor_id]);
	} else {
		LOG_ERR("HX711_%d calibration failed - no weight difference detected", sensor_id);
	}
}

/* Convert raw value to calibrated weight */
static float get_calibrated_weight(int32_t raw_value, int sensor_id)
{
	int32_t adjusted = raw_value - calibration_offsets[sensor_id];
	return (float)adjusted * calibration_scales[sensor_id];
}

/* Improved measurement function */
static void measure_all(void)
{
	struct hx711_data *sensors[3] = {&hx711_0_data, &hx711_1_data, &hx711_2_data};
	int32_t raw_values[3];
	float weights[3];
	int64_t t_start = k_uptime_get();
	
	/* Read all sensors */
	for (int i = 0; i < 3; i++) {
		if (hx711_read_raw(sensors[i], &raw_values[i]) == 0) {
			weights[i] = get_calibrated_weight(raw_values[i], i);
		} else {
			weights[i] = 0.0f;
			LOG_ERR("HX711_%d read failed", i);
		}
	}
	
	int64_t t_end = k_uptime_get();
	int32_t elapsed_ms = (int32_t)(t_end - t_start);
	double sps = 1000.0 / (double)elapsed_ms;
	
	/* Build one-line log string */
	char line_buf[256];
	size_t offset = 0;
	
	for (int i = 0; i < 3; i++) {
		int len = snprintf(&line_buf[offset], sizeof(line_buf) - offset,
		                   "HX711_%d: %.3fg (raw: %d)%s", i, (double)weights[i], raw_values[i],
		                   (i < 2) ? " | " : "");
		if (len < 0 || (offset + len >= sizeof(line_buf))) {
			LOG_WRN("Output truncated at sensor %d", i);
			break;
		}
		offset += (size_t)len;
	}
	
	LOG_INF("%s", line_buf);
	LOG_INF("Elapsed: %d ms, Approx SPS: %.1f", elapsed_ms, sps);
	
	/* Update activity time */
	last_activity_time = k_uptime_get();
}

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

int main(void)
{
	int ret;

	LOG_INF("HX711 Multi-Sensor Application Starting...");

	/* Power management initialization */
	force_sck_low();
	k_msleep(100);
	power_up_reset();

	/* Run hardware connection test */
	test_hardware_connections();

	/* Initialize HX711 sensors using device tree GPIO definitions */
	ret = hx711_init(&hx711_0_data, 
	                 HX711_0_DOUT_DEV, HX711_0_DOUT_PIN, HX711_0_DOUT_FLAGS,
	                 HX711_0_SCK_DEV, HX711_0_SCK_PIN, HX711_0_SCK_FLAGS);
	if (ret < 0) {
		LOG_ERR("Failed to initialize HX711 sensor 0: %d", ret);
		return -1;
	}

	ret = hx711_init(&hx711_1_data, 
	                 HX711_1_DOUT_DEV, HX711_1_DOUT_PIN, HX711_1_DOUT_FLAGS,
	                 HX711_1_SCK_DEV, HX711_1_SCK_PIN, HX711_1_SCK_FLAGS);
	if (ret < 0) {
		LOG_ERR("Failed to initialize HX711 sensor 1: %d", ret);
		return -1;
	}

	ret = hx711_init(&hx711_2_data, 
	                 HX711_2_DOUT_DEV, HX711_2_DOUT_PIN, HX711_2_DOUT_FLAGS,
	                 HX711_2_SCK_DEV, HX711_2_SCK_PIN, HX711_2_SCK_FLAGS);
	if (ret < 0) {
		LOG_ERR("Failed to initialize HX711 sensor 2: %d", ret);
		return -1;
	}

	/* Set all sensors to 80 SPS */
	hx711_set_rate(&hx711_0_data, 80);
	hx711_set_rate(&hx711_1_data, 80);
	hx711_set_rate(&hx711_2_data, 80);

	LOG_INF("All HX711 sensors initialized successfully");
	
	/* Let sensors stabilize */
	k_msleep(200);

	/* Tare all sensors */
	tare_sensor(&hx711_0_data, 0, 5);
	tare_sensor(&hx711_1_data, 1, 5);
	tare_sensor(&hx711_2_data, 2, 5);

	LOG_INF("Place 100g weight on each sensor for calibration...");
	for (int i = 5; i > 0; i--) {
		LOG_INF("%d...", i);
		k_msleep(1000);
	}

	/* Calibrate all sensors */
	calibrate_sensor(&hx711_0_data, 0, 100.0f, 5);
	calibrate_sensor(&hx711_1_data, 1, 100.0f, 5);
	calibrate_sensor(&hx711_2_data, 2, 100.0f, 5);

	LOG_INF("Starting continuous measurement...");

	/* Main loop - continuous reading with power management */
	while (1) {
		if (hx711_powered) {
			measure_all();
			
			/* Check for inactivity timeout */
			if ((k_uptime_get() - last_activity_time) > INACTIVITY_TIMEOUT_MS) {
				disable_hx711_power();
			}
			
			k_msleep(1);  /* High-speed polling */
		} else {
			/* Check if we should wake up (you can add a button or other trigger here) */
			k_sleep(K_MSEC(1000));
			
			/* For now, just wake up after 10 seconds */
			static int64_t sleep_start = 0;
			if (sleep_start == 0) {
				sleep_start = k_uptime_get();
			}
			
			if ((k_uptime_get() - sleep_start) > 10000) {
				enable_hx711_power();
				sleep_start = 0;
			}
		}
	}
} 