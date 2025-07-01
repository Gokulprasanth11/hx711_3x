/*
 * Copyright (c) 2025 HX711 Driver for Zephyr by GP
 * SPDX-License-Identifier: Apache-2.0
 */

#include "hx711_driver.h"
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/util.h>
#include  <stdint.h>
#include <zephyr/sys/printk.h>

int hx711_init(struct hx711_data *hx711, 
               const struct device *dout_dev, gpio_pin_t dout_pin, gpio_flags_t dout_flags,
               const struct device *sck_dev, gpio_pin_t sck_pin, gpio_flags_t sck_flags)
{
	int ret;

	if (!hx711 || !dout_dev || !sck_dev) {
		return -EINVAL;
	}

	/* Store device handles */
	hx711->dout_dev = dout_dev;
	hx711->sck_dev = sck_dev;

	/* Store pin information */
	hx711->dout_pin = dout_pin;
	hx711->sck_pin = sck_pin;
	hx711->dout_flags = dout_flags;
	hx711->sck_flags = sck_flags;

	/* Configure DOUT pin as input with pull-up */
	ret = gpio_pin_configure(hx711->dout_dev, dout_pin, dout_flags);
	if (ret < 0) {
		printk("Failed to configure DOUT pin: %d\n", ret);
		return ret;
	}

	/* Configure SCK pin as output, initially low */
	ret = gpio_pin_configure(hx711->sck_dev, sck_pin, sck_flags);
	if (ret < 0) {
		printk("Failed to configure SCK pin: %d\n", ret);
		return ret;
	}

	/* Power up delay - HX711 needs time to stabilize */
	k_msleep(400);

	/* Set default rate */
	hx711->rate_sps = HX711_DEFAULT_RATE_SPS;

	hx711->is_initialized = true;

	return 0;
}

int hx711_read_raw(struct hx711_data *hx711, int32_t *value)
{
	int ret;
	int32_t raw_value = 0;
	uint8_t i;

	if (!hx711 || !value || !hx711->is_initialized) {
		return -EINVAL;
	}

	/* Wait for data to be ready - use shorter timeout */
	ret = hx711_wait_for_data(hx711, K_MSEC(50));
	if (ret < 0) {
		return ret;
	}

	/* Read 24 bits of data */
	for (i = 0; i < 24; i++) {
		/* Clock high */
		ret = gpio_pin_set(hx711->sck_dev, hx711->sck_pin, 1);
		if (ret < 0) {
			return ret;
		}
		k_busy_wait(1);

		/* Read data bit */
		int data_bit = gpio_pin_get(hx711->dout_dev, hx711->dout_pin);
		if (data_bit < 0) {
			return data_bit;
		}

		/* Clock low */
		ret = gpio_pin_set(hx711->sck_dev, hx711->sck_pin, 0);
		if (ret < 0) {
			return ret;
		}
		k_busy_wait(1);

		/* Shift data into result */
		raw_value = (raw_value << 1) | data_bit;
	}

	/* Additional clock pulses to set channel and gain for next reading */
	/* For 80 SPS: 27 total pulses (24 data + 3 config) = Channel A, Gain 64 */
	/* For 10 SPS: 25 total pulses (24 data + 1 config) = Channel A, Gain 128 */
	/* For 10 SPS: 26 total pulses (24 data + 2 config) = Channel B, Gain 32 */
	
	/* We want 80 SPS, so we need 3 additional pulses (27 total) */
	for (i = 0; i < 3; i++) {
		ret = gpio_pin_set(hx711->sck_dev, hx711->sck_pin, 1);
		if (ret < 0) {
			return ret;
		}
		k_busy_wait(1);

		ret = gpio_pin_set(hx711->sck_dev, hx711->sck_pin, 0);
		if (ret < 0) {
			return ret;
		}
		k_busy_wait(1);
	}

	/* Convert to signed 24-bit value */
	if (raw_value & 0x800000) {
		raw_value |= 0xFF000000;  /* Sign extend negative values */
	}

	*value = raw_value;
	return 0;
}

int hx711_set_rate(struct hx711_data *hx711, uint8_t rate_sps)
{
	/* Store the desired rate for next reading */
	hx711->rate_sps = rate_sps;
	printk("HX711: Rate set to %d SPS (will be applied on next reading)\n", rate_sps);
	return 0;
}

int hx711_wait_for_data(struct hx711_data *hx711, k_timeout_t timeout)
{
	int ret;
	int64_t start_time = k_uptime_get();

	if (!hx711 || !hx711->is_initialized) {
		return -EINVAL;
	}

	/* Wait for DOUT to go low (data ready) */
	while (k_uptime_get() - start_time < timeout.ticks) {
		ret = gpio_pin_get(hx711->dout_dev, hx711->dout_pin);
		if (ret < 0) {
			return ret;
		}
		if (ret == 0) {
			return 0;  /* Data is ready */
		}
		k_msleep(1);
	}

	return -ETIMEDOUT;
}

int hx711_sleep(struct hx711_data *hx711)
{
	int ret;

	if (!hx711 || !hx711->is_initialized) {
		return -EINVAL;
	}

	/* Set SCK high to enter sleep mode */
	ret = gpio_pin_set(hx711->sck_dev, hx711->sck_pin, 1);
	if (ret < 0) {
		return ret;
	}

	/* Wait for sleep mode to take effect (>60us required) */
	k_busy_wait(HX711_SLEEP_DELAY_US);

	printk("HX711 entered sleep mode\n");
	return 0;
}

int hx711_wake_up(struct hx711_data *hx711)
{
	int ret;

	if (!hx711 || !hx711->is_initialized) {
		return -EINVAL;
	}

	/* Set SCK low to wake up */
	ret = gpio_pin_set(hx711->sck_dev, hx711->sck_pin, 0);
	if (ret < 0) {
		return ret;
	}

	/* Wait for power up settling time */
	k_sleep(K_MSEC(400));

	printk("HX711 woke up from sleep mode\n");
	return 0;
}

bool hx711_is_data_ready(struct hx711_data *hx711)
{
	int ret;

	if (!hx711 || !hx711->is_initialized) {
		return false;
	}

	ret = gpio_pin_get(hx711->dout_dev, hx711->dout_pin);
	return (ret == 0);  /* Data ready when DOUT is low */
} 