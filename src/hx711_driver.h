/*
 * Copyright (c) 2025 HX711 Driver for Zephyr by GP
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef HX711_DRIVER_H_
#define HX711_DRIVER_H_

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/device.h>
#include "hx711_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/* HX711 data structure */
struct hx711_data {
	const struct device *dout_dev;
	const struct device *sck_dev;
	const struct device *rate_dev;
	gpio_pin_t dout_pin;
	gpio_pin_t sck_pin;
	gpio_pin_t rate_pin;
	gpio_flags_t dout_flags;
	gpio_flags_t sck_flags;
	gpio_flags_t rate_flags;
	bool is_initialized;
};

/* Function prototypes */
int hx711_init(struct hx711_data *hx711, 
               const struct device *dout_dev, gpio_pin_t dout_pin, gpio_flags_t dout_flags,
               const struct device *sck_dev, gpio_pin_t sck_pin, gpio_flags_t sck_flags,
               const struct device *rate_dev, gpio_pin_t rate_pin, gpio_flags_t rate_flags);

int hx711_read_raw(struct hx711_data *hx711, int32_t *value);
int hx711_wait_for_data(struct hx711_data *hx711, k_timeout_t timeout);
int hx711_sleep(struct hx711_data *hx711);
int hx711_wake_up(struct hx711_data *hx711);
int hx711_set_rate(struct hx711_data *hx711, uint8_t rate_sps);
bool hx711_is_data_ready(struct hx711_data *hx711);

#ifdef __cplusplus
}
#endif

#endif /* HX711_DRIVER_H_ */ 