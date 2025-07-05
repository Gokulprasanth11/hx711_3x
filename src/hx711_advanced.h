/*
 * Copyright (c) 2025 HX711 Advanced Functions for Zephyr by GP
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef HX711_ADVANCED_H_
#define HX711_ADVANCED_H_

#include "hx711_driver.h"
#include <zephyr/kernel.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Reading modes */
#define HX711_AVERAGE_MODE    0x00
#define HX711_MEDIAN_MODE     0x01
#define HX711_MEDAVG_MODE     0x02
#define HX711_RUNAVG_MODE     0x03
#define HX711_RAW_MODE        0x04

/* Gain constants */
#define HX711_CHANNEL_A_GAIN_128  128
#define HX711_CHANNEL_A_GAIN_64   64
#define HX711_CHANNEL_B_GAIN_32   32

/* Advanced HX711 data structure */
struct hx711_advanced_data {
    struct hx711_data base;           /* Base HX711 data */
    
    /* Calibration data */
    int32_t offset;
    float scale;
    float tare_offset;
    bool tare_set_flag;
    
    /* Reading mode */
    uint8_t mode;
    
    /* Gain setting */
    uint8_t gain;
    
    /* Running average data */
    float runavg_value;
    bool runavg_initialized;
    
    /* Timing */
    uint32_t last_read_time;
    
    /* Pricing (optional) */
    float unit_price;
};

/* Advanced initialization */
int hx711_advanced_init(struct hx711_advanced_data *hx711,
                       const struct device *dout_dev, gpio_pin_t dout_pin, gpio_flags_t dout_flags,
                       const struct device *sck_dev, gpio_pin_t sck_pin, gpio_flags_t sck_flags);

/* Advanced reading functions */
float hx711_read_average(struct hx711_advanced_data *hx711, uint8_t times);
float hx711_read_median(struct hx711_advanced_data *hx711, uint8_t times);
float hx711_read_medavg(struct hx711_advanced_data *hx711, uint8_t times);
float hx711_read_runavg(struct hx711_advanced_data *hx711, uint8_t times, float alpha);

/* Mode setting functions */
void hx711_set_raw_mode(struct hx711_advanced_data *hx711);
void hx711_set_average_mode(struct hx711_advanced_data *hx711);
void hx711_set_median_mode(struct hx711_advanced_data *hx711);
void hx711_set_medavg_mode(struct hx711_advanced_data *hx711);
void hx711_set_runavg_mode(struct hx711_advanced_data *hx711);
uint8_t hx711_get_mode(struct hx711_advanced_data *hx711);

/* Processed reading functions */
float hx711_get_value(struct hx711_advanced_data *hx711, uint8_t times);
float hx711_get_units(struct hx711_advanced_data *hx711, uint8_t times);

/* Gain functions */
bool hx711_set_gain(struct hx711_advanced_data *hx711, uint8_t gain, bool forced);
uint8_t hx711_get_gain(struct hx711_advanced_data *hx711);

/* Tare functions */
void hx711_tare(struct hx711_advanced_data *hx711, uint8_t times);
float hx711_get_tare(struct hx711_advanced_data *hx711);
bool hx711_tare_set(struct hx711_advanced_data *hx711);

/* Calibration functions */
bool hx711_set_scale(struct hx711_advanced_data *hx711, float scale);
float hx711_get_scale(struct hx711_advanced_data *hx711);
void hx711_set_offset(struct hx711_advanced_data *hx711, int32_t offset);
int32_t hx711_get_offset(struct hx711_advanced_data *hx711);
void hx711_calibrate_scale(struct hx711_advanced_data *hx711, float weight, uint8_t times);

/* Utility functions */
uint32_t hx711_last_time_read(struct hx711_advanced_data *hx711);
void hx711_reset_advanced(struct hx711_advanced_data *hx711);

/* Pricing functions */
float hx711_get_price(struct hx711_advanced_data *hx711, uint8_t times);
void hx711_set_unit_price(struct hx711_advanced_data *hx711, float price);
float hx711_get_unit_price(struct hx711_advanced_data *hx711);

#ifdef __cplusplus
}
#endif

#endif /* HX711_ADVANCED_H_ */ 