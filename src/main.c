/*
 * Copyright (c) 2025 HX711 Multi-Sensor Application by GP
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <zephyr/logging/log.h>
#include <math.h>
#include "hx711_driver.h"
#include "hx711_config.h"
#include "hx711_advanced.h"
#include "hx711_demo.h"
#include <stdint.h>
#include <zephyr/devicetree.h>

LOG_MODULE_REGISTER(main, CONFIG_LOG_DEFAULT_LEVEL);

/* HX711 sensor instances */
static struct hx711_data hx711_0_data;
static struct hx711_data hx711_1_data;
static struct hx711_data hx711_2_data;

/* Advanced HX711 sensor instances */
static struct hx711_advanced_data hx711_adv_0_data;
static struct hx711_advanced_data hx711_adv_1_data;
static struct hx711_advanced_data hx711_adv_2_data;

/* Function prototypes */
void test_hardware_connections(void);
void test_individual_sensors(void);
void run_comprehensive_calibration(void);
void print_weight_measurements(void);
void print_sensor_calibration_status(struct hx711_advanced_data *hx711, const char *name);

/* Hardware test function */
void test_hardware_connections(void)
{
	LOG_INF("=== HARDWARE CONNECTION TEST ===");
	
	/* Test GPIO devices */
	const struct device *gpio0 = DEVICE_DT_GET(DT_NODELABEL(gpio0));
	const struct device *gpio1 = DEVICE_DT_GET(DT_NODELABEL(gpio1));
	
	if (!device_is_ready(gpio0)) {
		LOG_ERR("GPIO0 device not ready!");
		return;
	}
	LOG_INF("GPIO0 device is ready");
	
	if (!device_is_ready(gpio1)) {
		LOG_ERR("GPIO1 device not ready!");
		return;
	}
	LOG_INF("GPIO1 device is ready");
	
	/* Test SCK pins as outputs */
	LOG_INF("Testing SCK pins as outputs...");
	
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
	
	LOG_INF("SCK pins configured successfully");
	
	/* Test DOUT pins as inputs */
	LOG_INF("Testing DOUT pins as inputs...");
	
	gpio_pin_configure(gpio0, HX711_0_DOUT_PIN, GPIO_INPUT | GPIO_PULL_UP);
	gpio_pin_configure(gpio0, HX711_1_DOUT_PIN, GPIO_INPUT | GPIO_PULL_UP);
	gpio_pin_configure(gpio0, HX711_2_DOUT_PIN, GPIO_INPUT | GPIO_PULL_UP);
	
	/* Read DOUT pin states */
	int dout0 = gpio_pin_get(gpio0, HX711_0_DOUT_PIN);
	int dout1 = gpio_pin_get(gpio0, HX711_1_DOUT_PIN);
	int dout2 = gpio_pin_get(gpio0, HX711_2_DOUT_PIN);
	
	LOG_INF("DOUT pin states: %d %d %d", dout0, dout1, dout2);
	
	if (dout0 == 1 && dout1 == 1 && dout2 == 1) {
		LOG_WRN("All DOUT pins are HIGH - sensors may not be connected!");
		LOG_INF("Check your wiring:");
		LOG_INF("- VCC to 3.3V or 5V");
		LOG_INF("- GND to GND");
		LOG_INF("- DOUT to GPIO0 pins (with pull-up)");
		LOG_INF("- SCK to GPIO1 pins");
	} else {
		LOG_INF("Some sensors appear to be connected");
	}
	
	LOG_INF("=== END HARDWARE TEST ===");
}

/* Individual sensor test function */
void test_individual_sensors(void)
{
	LOG_INF("=== INDIVIDUAL SENSOR TEST ===");
	
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
		
		LOG_INF("--- Testing %s (SCK: GPIO1.%d, DOUT: GPIO0.%d) ---", 
		       sensor_name, sck_pin, dout_pin);
		
		/* Configure pins */
		gpio_pin_configure(sck_dev, sck_pin, GPIO_OUTPUT | GPIO_OUTPUT_LOW);
		gpio_pin_configure(gpio0, dout_pin, GPIO_INPUT | GPIO_PULL_UP);
		
		/* Read initial DOUT state */
		int initial_dout = gpio_pin_get(gpio0, dout_pin);
		LOG_INF("Initial DOUT state: %d", initial_dout);
		
		/* Test SCK control */
		gpio_pin_set(sck_dev, sck_pin, 1);
		k_msleep(50);
		gpio_pin_set(sck_dev, sck_pin, 0);
		k_msleep(50);
		
		/* Read DOUT after SCK pulse */
		int after_dout = gpio_pin_get(gpio0, dout_pin);
		LOG_INF("DOUT after SCK pulse: %d", after_dout);
		
		if (initial_dout == 1 && after_dout == 1) {
			LOG_WRN("RESULT: %s appears to be disconnected or not powered", sensor_name);
			LOG_INF("TROUBLESHOOTING:");
			LOG_INF("1. Check VCC connection (3.3V or 5V)");
			LOG_INF("2. Check GND connection");
			LOG_INF("3. Check load cell wiring to HX711");
			LOG_INF("4. Check DOUT and SCK wire connections");
			LOG_INF("5. Try a different load cell");
		} else if (initial_dout == 0 || after_dout == 0) {
			LOG_INF("RESULT: %s appears to be connected and responding", sensor_name);
		}
	}
	
	LOG_INF("=== END INDIVIDUAL SENSOR TEST ===");
}

/* Comprehensive calibration function */
void run_comprehensive_calibration(void) {
	LOG_INF("=== HX711 COMPREHENSIVE CALIBRATION ===");
	
	struct hx711_advanced_data *sensors[] = {&hx711_adv_0_data, &hx711_adv_1_data, &hx711_adv_2_data};
	const char *sensor_names[] = {"Sensor 0", "Sensor 1", "Sensor 2"};
	
	for (int sensor_id = 0; sensor_id < 3; sensor_id++) {
		LOG_INF("--- Calibrating %s ---", sensor_names[sensor_id]);
		
		/* Step 1: Initial State */
		LOG_INF("Step 1: Initial State");
		float initial_raw = hx711_read_average(sensors[sensor_id], 3);
		LOG_INF("  Raw reading: %.2f", initial_raw);
		LOG_INF("  Current scale: %.6f", hx711_get_scale(sensors[sensor_id]));
		LOG_INF("  Current tare: %.2f", hx711_get_tare(sensors[sensor_id]));
		
		/* Step 2: Tare (Zero Calibration) */
		LOG_INF("Step 2: Tare Calibration (Remove all weight from %s)", sensor_names[sensor_id]);
		LOG_INF("  Press any key when %s is empty, then wait 3 seconds...", sensor_names[sensor_id]);
		k_sleep(K_SECONDS(3));
		
		hx711_tare(sensors[sensor_id], 10); // Take 10 readings for stable tare
		LOG_INF("  Tare offset set to: %.2f", hx711_get_tare(sensors[sensor_id]));
		
		/* Step 3: Scale Calibration */
		LOG_INF("Step 3: Scale Calibration");
		LOG_INF("  Place a known weight (e.g., 1000g) on %s", sensor_names[sensor_id]);
		LOG_INF("  Press any key when weight is placed, then wait 3 seconds...");
		k_sleep(K_SECONDS(3));
		
		/* Read the raw value with known weight */
		float raw_with_weight = hx711_read_average(sensors[sensor_id], 5);
		
		/* Calculate scale factor (assuming 1000g known weight) */
		float known_weight_g = 1000.0f; // Known weight in grams
		float tare_offset = hx711_get_tare(sensors[sensor_id]);
		float scale_factor = (raw_with_weight - tare_offset) / known_weight_g;
		
		hx711_set_scale(sensors[sensor_id], scale_factor);
		LOG_INF("  Raw reading with weight: %.2f", raw_with_weight);
		LOG_INF("  Scale factor calculated: %.6f", scale_factor);
		LOG_INF("  This means 1 unit = %.4f grams", 1.0f / scale_factor);
		
		/* Step 4: Verify Calibration */
		LOG_INF("Step 4: Verify Calibration");
		float measured_weight = hx711_get_units(sensors[sensor_id], 5);
		float error_percent = ((measured_weight - known_weight_g) / known_weight_g) * 100.0f;
		
		LOG_INF("  Measured weight: %.1f grams", measured_weight);
		LOG_INF("  Expected weight: %.1f grams", known_weight_g);
		LOG_INF("  Error: %.2f%%", error_percent);
		
		if (fabs(error_percent) < 5.0f) {
			LOG_INF("  ✓ Calibration successful! Error < 5%%");
		} else {
			LOG_WRN("  ⚠ Calibration may need adjustment. Error > 5%%");
		}
		
		/* Step 5: Test Different Weights */
		LOG_INF("Step 5: Test Different Weights");
		LOG_INF("  Remove weight from %s and wait 2 seconds...", sensor_names[sensor_id]);
		k_sleep(K_SECONDS(2));
		
		float zero_weight = hx711_get_units(sensors[sensor_id], 3);
		LOG_INF("  Zero reading: %.1f grams", zero_weight);
		
		LOG_INF("  Place half weight (500g) on %s and wait 2 seconds...", sensor_names[sensor_id]);
		k_sleep(K_SECONDS(2));
		
		float half_weight = hx711_get_units(sensors[sensor_id], 3);
		LOG_INF("  Half weight reading: %.1f grams", half_weight);
		
		LOG_INF("--- End %s Calibration ---", sensor_names[sensor_id]);
		k_sleep(K_SECONDS(2));
	}
	
	LOG_INF("=== All Sensors Calibrated ===");
}

/* Print weight measurements in multiple units */
void print_weight_measurements(void) {
	LOG_INF("=== WEIGHT MEASUREMENTS ===");
	
	struct hx711_advanced_data *sensors[] = {&hx711_adv_0_data, &hx711_adv_1_data, &hx711_adv_2_data};
	const char *sensor_names[] = {"Sensor 0", "Sensor 1", "Sensor 2"};
	
	for (int i = 0; i < 3; i++) {
		float weight_g = hx711_get_units(sensors[i], 3);
		float weight_kg = weight_g / 1000.0f;
		float weight_lbs = weight_g * 0.00220462f;
		float force_n = weight_g * 0.00980665f;
		
		LOG_INF("%s:", sensor_names[i]);
		LOG_INF("  Weight: %.1f grams", weight_g);
		LOG_INF("  Weight: %.3f kg", weight_kg);
		LOG_INF("  Weight: %.2f lbs", weight_lbs);
		LOG_INF("  Force:  %.3f N", force_n);
	}
}

/* Print sensor calibration status */
void print_sensor_calibration_status(struct hx711_advanced_data *hx711, const char *name) {
	if (!hx711 || !name) {
		return;
	}
	
	LOG_INF("%s Calibration Status:", name);
	LOG_INF("  Scale Factor:  %.6f", hx711_get_scale(hx711));
	LOG_INF("  Tare Offset:   %.2f", hx711_get_tare(hx711));
	LOG_INF("  Calibration:   %s", hx711_get_scale(hx711) > 1.0f ? "Calibrated" : "Not Calibrated");
	LOG_INF("  Gain Setting:  %d", hx711_get_gain(hx711));
	LOG_INF("  Reading Mode:  %d", hx711_get_mode(hx711));
}

int main(void)
{
	int ret;
	uint32_t sample_count = 0;

	LOG_INF("=== HX711 Multi-Sensor Application Starting ===");
	LOG_INF("Supporting 3 HX711 sensors with comprehensive calibration");

	/* Run hardware connection test */
	test_hardware_connections();
	k_sleep(K_SECONDS(1));
	
	/* Run individual sensor test */
	test_individual_sensors();
	k_sleep(K_SECONDS(2));

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

	LOG_INF("All basic HX711 sensors initialized successfully");
	
	/* Initialize advanced HX711 sensors */
	ret = hx711_advanced_init(&hx711_adv_0_data, 
	                         HX711_0_DOUT_DEV, HX711_0_DOUT_PIN, HX711_0_DOUT_FLAGS,
	                         HX711_0_SCK_DEV, HX711_0_SCK_PIN, HX711_0_SCK_FLAGS);
	if (ret < 0) {
		LOG_ERR("Failed to initialize advanced HX711 sensor 0: %d", ret);
	} else {
		LOG_INF("Advanced HX711 sensor 0 initialized successfully");
	}

	ret = hx711_advanced_init(&hx711_adv_1_data, 
	                         HX711_1_DOUT_DEV, HX711_1_DOUT_PIN, HX711_1_DOUT_FLAGS,
	                         HX711_1_SCK_DEV, HX711_1_SCK_PIN, HX711_1_SCK_FLAGS);
	if (ret < 0) {
		LOG_ERR("Failed to initialize advanced HX711 sensor 1: %d", ret);
	} else {
		LOG_INF("Advanced HX711 sensor 1 initialized successfully");
	}

	ret = hx711_advanced_init(&hx711_adv_2_data, 
	                         HX711_2_DOUT_DEV, HX711_2_DOUT_PIN, HX711_2_DOUT_FLAGS,
	                         HX711_2_SCK_DEV, HX711_2_SCK_PIN, HX711_2_SCK_FLAGS);
	if (ret < 0) {
		LOG_ERR("Failed to initialize advanced HX711 sensor 2: %d", ret);
	} else {
		LOG_INF("Advanced HX711 sensor 2 initialized successfully");
	}
	
	/* Run comprehensive calibration */
	LOG_INF("=== Starting Comprehensive Calibration ===");
	run_comprehensive_calibration();
	k_sleep(K_SECONDS(2));
	
	/* Run the advanced demo */
	LOG_INF("=== Running HX711 Advanced Functions Demo ===");
	run_hx711_advanced_demo();
	k_sleep(K_SECONDS(2));
	
	LOG_INF("=== Starting Continuous Weight Monitoring ===");
	LOG_INF("Press Ctrl+C to stop");

	/* Main loop - continuous weight monitoring */
	while (1) {
		sample_count++;
		LOG_INF("=== Reading #%u ===", sample_count);
		
		/* Print weight measurements for all sensors */
		print_weight_measurements();
		
		/* Print calibration status every 10 readings */
		if (sample_count % 10 == 0) {
			LOG_INF("=== Calibration Status ===");
			print_sensor_calibration_status(&hx711_adv_0_data, "Sensor 0");
			print_sensor_calibration_status(&hx711_adv_1_data, "Sensor 1");
			print_sensor_calibration_status(&hx711_adv_2_data, "Sensor 2");
		}
		
		LOG_INF("--- End Reading #%u ---", sample_count);
		k_sleep(K_SECONDS(5)); // 5 second interval between readings
	}
} 