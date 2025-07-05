/*
 * Copyright (c) 2025 HX711 Advanced Functions for Zephyr by GP
 * SPDX-License-Identifier: Apache-2.0
 */

#include "hx711_advanced.h"
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <math.h>
#include <string.h>

/* Helper function for insertion sort (used in median calculations) */
static void insert_sort(float *array, uint8_t size)
{
    for (uint8_t i = 1; i < size; i++) {
        float key = array[i];
        int j = i - 1;
        
        while (j >= 0 && array[j] > key) {
            array[j + 1] = array[j];
            j--;
        }
        array[j + 1] = key;
    }
}

/* Helper function to read raw value as float */
static float read_raw_float(struct hx711_advanced_data *hx711)
{
    int32_t raw_value;
    int ret = hx711_read_raw(&hx711->base, &raw_value);
    if (ret < 0) {
        return 0.0f;
    }
    return (float)raw_value;
}

int hx711_advanced_init(struct hx711_advanced_data *hx711,
                       const struct device *dout_dev, gpio_pin_t dout_pin, gpio_flags_t dout_flags,
                       const struct device *sck_dev, gpio_pin_t sck_pin, gpio_flags_t sck_flags)
{
    if (!hx711) {
        return -EINVAL;
    }
    
    /* Initialize base HX711 */
    int ret = hx711_init(&hx711->base, dout_dev, dout_pin, dout_flags, sck_dev, sck_pin, sck_flags);
    if (ret < 0) {
        return ret;
    }
    
    /* Initialize advanced parameters */
    hx711->offset = 0;
    hx711->scale = 1.0f;
    hx711->tare_offset = 0.0f;
    hx711->tare_set_flag = false;
    hx711->mode = HX711_AVERAGE_MODE;
    hx711->gain = HX711_CHANNEL_A_GAIN_128;
    hx711->runavg_value = 0.0f;
    hx711->runavg_initialized = false;
    hx711->last_read_time = 0;
    hx711->unit_price = 1.0f;
    
    return 0;
}

float hx711_read_average(struct hx711_advanced_data *hx711, uint8_t times)
{
    if (times < 1) times = 1;
    
    float sum = 0.0f;
    for (uint8_t i = 0; i < times; i++) {
        sum += read_raw_float(hx711);
        k_msleep(1); /* Small delay between readings */
    }
    
    hx711->last_read_time = k_uptime_get();
    return sum / times;
}

float hx711_read_median(struct hx711_advanced_data *hx711, uint8_t times)
{
    if (times > 15) times = 15;
    if (times < 3) times = 3;
    
    float samples[15];
    for (uint8_t i = 0; i < times; i++) {
        samples[i] = read_raw_float(hx711);
        k_msleep(1);
    }
    
    insert_sort(samples, times);
    
    if (times & 0x01) {
        /* Odd number of samples */
        hx711->last_read_time = k_uptime_get();
        return samples[times/2];
    } else {
        /* Even number of samples - average of two middle values */
        hx711->last_read_time = k_uptime_get();
        return (samples[times/2 - 1] + samples[times/2]) / 2.0f;
    }
}

float hx711_read_medavg(struct hx711_advanced_data *hx711, uint8_t times)
{
    if (times > 15) times = 15;
    if (times < 3) times = 3;
    
    float samples[15];
    for (uint8_t i = 0; i < times; i++) {
        samples[i] = read_raw_float(hx711);
        k_msleep(1);
    }
    
    insert_sort(samples, times);
    
    /* Calculate average of middle half */
    uint8_t start = times / 4;
    uint8_t end = times - start;
    float sum = 0.0f;
    
    for (uint8_t i = start; i < end; i++) {
        sum += samples[i];
    }
    
    hx711->last_read_time = k_uptime_get();
    return sum / (end - start);
}

float hx711_read_runavg(struct hx711_advanced_data *hx711, uint8_t times, float alpha)
{
    if (times < 1) times = 1;
    if (alpha < 0.0f) alpha = 0.0f;
    if (alpha > 1.0f) alpha = 1.0f;
    
    float current_value = read_raw_float(hx711);
    
    if (!hx711->runavg_initialized) {
        hx711->runavg_value = current_value;
        hx711->runavg_initialized = true;
    } else {
        hx711->runavg_value = alpha * current_value + (1.0f - alpha) * hx711->runavg_value;
    }
    
    hx711->last_read_time = k_uptime_get();
    return hx711->runavg_value;
}

void hx711_set_raw_mode(struct hx711_advanced_data *hx711)
{
    hx711->mode = HX711_RAW_MODE;
}

void hx711_set_average_mode(struct hx711_advanced_data *hx711)
{
    hx711->mode = HX711_AVERAGE_MODE;
}

void hx711_set_median_mode(struct hx711_advanced_data *hx711)
{
    hx711->mode = HX711_MEDIAN_MODE;
}

void hx711_set_medavg_mode(struct hx711_advanced_data *hx711)
{
    hx711->mode = HX711_MEDAVG_MODE;
}

void hx711_set_runavg_mode(struct hx711_advanced_data *hx711)
{
    hx711->mode = HX711_RUNAVG_MODE;
}

uint8_t hx711_get_mode(struct hx711_advanced_data *hx711)
{
    return hx711->mode;
}

float hx711_get_value(struct hx711_advanced_data *hx711, uint8_t times)
{
    float raw_value;
    
    switch (hx711->mode) {
        case HX711_RAW_MODE:
            raw_value = read_raw_float(hx711);
            break;
        case HX711_AVERAGE_MODE:
            raw_value = hx711_read_average(hx711, times);
            break;
        case HX711_MEDIAN_MODE:
            raw_value = hx711_read_median(hx711, times);
            break;
        case HX711_MEDAVG_MODE:
            raw_value = hx711_read_medavg(hx711, times);
            break;
        case HX711_RUNAVG_MODE:
            raw_value = hx711_read_runavg(hx711, times, 0.5f);
            break;
        default:
            raw_value = hx711_read_average(hx711, times);
            break;
    }
    
    return raw_value - hx711->offset;
}

float hx711_get_units(struct hx711_advanced_data *hx711, uint8_t times)
{
    float value = hx711_get_value(hx711, times);
    return value / hx711->scale;
}

bool hx711_set_gain(struct hx711_advanced_data *hx711, uint8_t gain, bool forced)
{
    if (!forced && hx711->gain == gain) {
        return true; /* Already set to this gain */
    }
    
    /* Validate gain value */
    if (gain != HX711_CHANNEL_A_GAIN_128 && 
        gain != HX711_CHANNEL_A_GAIN_64 && 
        gain != HX711_CHANNEL_B_GAIN_32) {
        return false;
    }
    
    hx711->gain = gain;
    
    /* Note: In a real implementation, you would need to read from the sensor
     * to apply the new gain setting, as the gain is set by the number of
     * clock pulses after the 24 data bits */
    
    return true;
}

uint8_t hx711_get_gain(struct hx711_advanced_data *hx711)
{
    return hx711->gain;
}

void hx711_tare(struct hx711_advanced_data *hx711, uint8_t times)
{
    float tare_value = hx711_read_average(hx711, times);
    hx711->tare_offset = tare_value;
    hx711->tare_set_flag = true;
    printk("HX711: Tare set to %.2f\n", (double)tare_value);
}

float hx711_get_tare(struct hx711_advanced_data *hx711)
{
    return hx711->tare_offset;
}

bool hx711_tare_set(struct hx711_advanced_data *hx711)
{
    return hx711->tare_set_flag;
}

bool hx711_set_scale(struct hx711_advanced_data *hx711, float scale)
{
    if (scale <= 0.0f) {
        return false;
    }
    
    hx711->scale = scale;
    return true;
}

float hx711_get_scale(struct hx711_advanced_data *hx711)
{
    return hx711->scale;
}

void hx711_set_offset(struct hx711_advanced_data *hx711, int32_t offset)
{
    hx711->offset = offset;
}

int32_t hx711_get_offset(struct hx711_advanced_data *hx711)
{
    return hx711->offset;
}

void hx711_calibrate_scale(struct hx711_advanced_data *hx711, float weight, uint8_t times)
{
    if (weight <= 0.0f) {
        printk("HX711: Invalid weight for calibration\n");
        return;
    }
    
    /* First, tare the scale if not already done */
    if (!hx711->tare_set_flag) {
        hx711_tare(hx711, times);
    }
    
    /* Read the raw value with the known weight */
    float raw_value = hx711_read_average(hx711, times);
    
    /* Calculate scale factor */
    float new_scale = (raw_value - hx711->tare_offset) / weight;
    
    if (new_scale > 0.0f) {
        hx711->scale = new_scale;
        printk("HX711: Scale calibrated to %.6f\n", (double)new_scale);
    } else {
        printk("HX711: Calibration failed - invalid scale factor\n");
    }
}

uint32_t hx711_last_time_read(struct hx711_advanced_data *hx711)
{
    return hx711->last_read_time;
}

void hx711_reset_advanced(struct hx711_advanced_data *hx711)
{
    hx711->offset = 0;
    hx711->scale = 1.0f;
    hx711->tare_offset = 0.0f;
    hx711->tare_set_flag = false;
    hx711->mode = HX711_AVERAGE_MODE;
    hx711->gain = HX711_CHANNEL_A_GAIN_128;
    hx711->runavg_value = 0.0f;
    hx711->runavg_initialized = false;
    hx711->last_read_time = 0;
    hx711->unit_price = 1.0f;
    
    printk("HX711: Advanced settings reset\n");
}

float hx711_get_price(struct hx711_advanced_data *hx711, uint8_t times)
{
    return hx711_get_units(hx711, times) * hx711->unit_price;
}

void hx711_set_unit_price(struct hx711_advanced_data *hx711, float price)
{
    hx711->unit_price = price;
}

float hx711_get_unit_price(struct hx711_advanced_data *hx711)
{
    return hx711->unit_price;
} 