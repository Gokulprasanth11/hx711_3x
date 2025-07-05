/*
 * Copyright (c) 2025 HX711 Advanced Demo Application by GP
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include "hx711_advanced.h"
#include "hx711_config.h"
#include <math.h>

/* Advanced HX711 sensor instances */
static struct hx711_advanced_data hx711_adv_0;
static struct hx711_advanced_data hx711_adv_1;
static struct hx711_advanced_data hx711_adv_2;

/* Demo state */
static bool demo_initialized = false;

/* Function prototypes */
void demo_initialize_sensors(void);
void demo_basic_reading(void);
void demo_reading_modes(void);
void demo_calibration_process(void);
void demo_tare_functionality(void);
void demo_advanced_features(void);
void demo_continuous_monitoring(void);
void print_sensor_status(struct hx711_advanced_data *hx711, const char *name);

void demo_initialize_sensors(void)
{
    printk("\n=== HX711 ADVANCED DEMO INITIALIZATION ===\n");
    
    /* Initialize advanced HX711 sensors */
    int ret = hx711_advanced_init(&hx711_adv_0,
                                 HX711_0_DOUT_DEV, HX711_0_DOUT_PIN, HX711_0_DOUT_FLAGS,
                                 HX711_0_SCK_DEV, HX711_0_SCK_PIN, HX711_0_SCK_FLAGS);
    if (ret < 0) {
        printk("Failed to initialize advanced HX711 sensor 0: %d\n", ret);
        return;
    }
    
    ret = hx711_advanced_init(&hx711_adv_1,
                             HX711_1_DOUT_DEV, HX711_1_DOUT_PIN, HX711_1_DOUT_FLAGS,
                             HX711_1_SCK_DEV, HX711_1_SCK_PIN, HX711_1_SCK_FLAGS);
    if (ret < 0) {
        printk("Failed to initialize advanced HX711 sensor 1: %d\n", ret);
        return;
    }
    
    ret = hx711_advanced_init(&hx711_adv_2,
                             HX711_2_DOUT_DEV, HX711_2_DOUT_PIN, HX711_2_DOUT_FLAGS,
                             HX711_2_SCK_DEV, HX711_2_SCK_PIN, HX711_2_SCK_FLAGS);
    if (ret < 0) {
        printk("Failed to initialize advanced HX711 sensor 2: %d\n", ret);
        return;
    }
    
    printk("All advanced HX711 sensors initialized successfully!\n");
    demo_initialized = true;
}

void demo_basic_reading(void)
{
    if (!demo_initialized) return;
    
    printk("\n=== BASIC READING DEMO ===\n");
    
    /* Test raw reading */
    printk("Testing raw readings...\n");
    float raw_0 = hx711_read_average(&hx711_adv_0, 5);
    float raw_1 = hx711_read_average(&hx711_adv_1, 5);
    float raw_2 = hx711_read_average(&hx711_adv_2, 5);
    
    printk("Raw values (5 samples avg): Sensor0=%.2f, Sensor1=%.2f, Sensor2=%.2f\n", 
           (double)raw_0, (double)raw_1, (double)raw_2);
    
    /* Test different reading modes */
    printk("\nTesting different reading modes on Sensor 0:\n");
    
    /* Raw mode */
    hx711_set_raw_mode(&hx711_adv_0);
    float raw_mode = hx711_get_value(&hx711_adv_0, 1);
    printk("  Raw mode: %.2f\n", (double)raw_mode);
    
    /* Average mode */
    hx711_set_average_mode(&hx711_adv_0);
    float avg_mode = hx711_get_value(&hx711_adv_0, 10);
    printk("  Average mode (10 samples): %.2f\n", (double)avg_mode);
    
    /* Median mode */
    hx711_set_median_mode(&hx711_adv_0);
    float med_mode = hx711_get_value(&hx711_adv_0, 7);
    printk("  Median mode (7 samples): %.2f\n", (double)med_mode);
    
    /* Running average mode */
    hx711_set_runavg_mode(&hx711_adv_0);
    float runavg_mode = hx711_get_value(&hx711_adv_0, 1);
    printk("  Running average mode: %.2f\n", (double)runavg_mode);
    
    /* Reset to average mode */
    hx711_set_average_mode(&hx711_adv_0);
}

void demo_reading_modes(void)
{
    if (!demo_initialized) return;
    
    printk("\n=== READING MODES COMPARISON ===\n");
    
    printk("Comparing different reading modes on Sensor 1:\n");
    printk("Format: Mode | Raw Value | Offset Corrected | Units\n");
    
    /* Test all modes */
    const char *modes[] = {"Raw", "Average", "Median", "MedAvg", "RunAvg"};
    uint8_t mode_values[] = {HX711_RAW_MODE, HX711_AVERAGE_MODE, HX711_MEDIAN_MODE, 
                            HX711_MEDAVG_MODE, HX711_RUNAVG_MODE};
    
    for (int i = 0; i < 5; i++) {
        switch(mode_values[i]) {
            case HX711_RAW_MODE:
                hx711_set_raw_mode(&hx711_adv_1);
                break;
            case HX711_AVERAGE_MODE:
                hx711_set_average_mode(&hx711_adv_1);
                break;
            case HX711_MEDIAN_MODE:
                hx711_set_median_mode(&hx711_adv_1);
                break;
            case HX711_MEDAVG_MODE:
                hx711_set_medavg_mode(&hx711_adv_1);
                break;
            case HX711_RUNAVG_MODE:
                hx711_set_runavg_mode(&hx711_adv_1);
                break;
        }
        float raw_val = hx711_get_value(&hx711_adv_1, 5);
        float units = hx711_get_units(&hx711_adv_1, 5);
        
        printk("%-8s | %8.2f | %15.2f | %8.2f\n", 
               modes[i], (double)(raw_val + hx711_adv_1.offset), (double)raw_val, (double)units);
    }
    
    /* Reset to average mode */
    hx711_set_average_mode(&hx711_adv_1);
}

void demo_calibration_process(void)
{
    if (!demo_initialized) return;
    
    printk("\n=== CALIBRATION PROCESS DEMO ===\n");
    
    printk("Demonstrating calibration process on Sensor 2:\n");
    
    /* Step 1: Tare the scale */
    printk("Step 1: Taring the scale (remove all weight)\n");
    hx711_tare(&hx711_adv_2, 10);
    printk("  Tare offset set to: %.2f\n", (double)hx711_get_tare(&hx711_adv_2));
    
    /* Step 2: Set a known scale factor (simulated calibration) */
    printk("Step 2: Setting scale factor (simulated calibration with 1000g weight)\n");
    float simulated_weight = 1000.0f;  /* 1000 grams */
    float simulated_raw_value = 50000.0f;  /* Simulated raw reading */
    
    /* Calculate scale factor */
    float scale_factor = (simulated_raw_value - hx711_adv_2.tare_offset) / simulated_weight;
    hx711_set_scale(&hx711_adv_2, scale_factor);
    
    printk("  Scale factor set to: %.6f\n", (double)hx711_get_scale(&hx711_adv_2));
    printk("  This means 1 unit = %.4f grams\n", (double)(1.0f / hx711_get_scale(&hx711_adv_2)));
    
    /* Step 3: Test the calibration */
    printk("Step 3: Testing calibration\n");
    float test_units = hx711_get_units(&hx711_adv_2, 5);
    printk("  Current reading in units: %.2f\n", (double)test_units);
    
    /* Simulate different weights */
    printk("Step 4: Simulating different weights\n");
    for (int weight = 0; weight <= 2000; weight += 500) {
        /* Simulate weight by adjusting offset */
        float simulated_offset = hx711_adv_2.tare_offset + (weight * scale_factor);
        hx711_set_offset(&hx711_adv_2, (int32_t)simulated_offset);
        
        float units = hx711_get_units(&hx711_adv_2, 1);
        printk("  Simulated %d grams -> Reading: %.1f grams\n", weight, (double)units);
    }
    
    /* Reset offset */
    hx711_set_offset(&hx711_adv_2, 0);
}

void demo_tare_functionality(void)
{
    if (!demo_initialized) return;
    
    printk("\n=== TARE FUNCTIONALITY DEMO ===\n");
    
    printk("Demonstrating tare functionality on Sensor 0:\n");
    
    /* Initial state */
    printk("Initial state:\n");
    print_sensor_status(&hx711_adv_0, "Sensor 0");
    
    /* First tare */
    printk("\nStep 1: First tare (empty scale)\n");
    hx711_tare(&hx711_adv_0, 5);
    print_sensor_status(&hx711_adv_0, "Sensor 0");
    
    /* Simulate adding weight */
    printk("\nStep 2: Simulating adding 500g weight\n");
    float weight_offset = 500.0f * hx711_adv_0.scale;
    hx711_set_offset(&hx711_adv_0, (int32_t)weight_offset);
    
    float units = hx711_get_units(&hx711_adv_0, 1);
    printk("  Reading with 500g weight: %.1f grams\n", (double)units);
    
    /* Second tare (with weight on scale) */
    printk("\nStep 3: Second tare (with weight on scale)\n");
    hx711_tare(&hx711_adv_0, 5);
    print_sensor_status(&hx711_adv_0, "Sensor 0");
    
    /* Simulate adding more weight */
    printk("\nStep 4: Simulating adding 200g more weight\n");
    float additional_offset = 200.0f * hx711_adv_0.scale;
    hx711_set_offset(&hx711_adv_0, (int32_t)additional_offset);
    
    units = hx711_get_units(&hx711_adv_0, 1);
    printk("  Reading with additional 200g: %.1f grams\n", (double)units);
    
    /* Reset */
    hx711_set_offset(&hx711_adv_0, 0);
    hx711_reset_advanced(&hx711_adv_0);
}

void demo_advanced_features(void)
{
    if (!demo_initialized) return;
    
    printk("\n=== ADVANCED FEATURES DEMO ===\n");
    
    printk("Testing advanced features on Sensor 1:\n");
    
    /* Gain setting */
    printk("Gain settings:\n");
    printk("  Current gain: %d\n", hx711_get_gain(&hx711_adv_1));
    
    bool ret = hx711_set_gain(&hx711_adv_1, HX711_CHANNEL_A_GAIN_64, false);
    printk("  Setting gain to 64: %s\n", ret ? "Success" : "Failed");
    printk("  New gain: %d\n", hx711_get_gain(&hx711_adv_1));
    
    /* Pricing functionality */
    printk("\nPricing functionality:\n");
    hx711_set_unit_price(&hx711_adv_1, 2.50f);  /* $2.50 per gram */
    printk("  Unit price set to: $%.2f per gram\n", (double)hx711_get_unit_price(&hx711_adv_1));
    
    /* Simulate weight and calculate price */
    float test_weight = 100.0f;  /* 100 grams */
    hx711_set_offset(&hx711_adv_1, (int32_t)(test_weight * hx711_adv_1.scale));
    
    float units = hx711_get_units(&hx711_adv_1, 1);
    float price = hx711_get_price(&hx711_adv_1, 1);
    printk("  Weight: %.1f grams, Price: $%.2f\n", (double)units, (double)price);
    
    /* Timing information */
    printk("\nTiming information:\n");
    uint32_t last_read = hx711_last_time_read(&hx711_adv_1);
    printk("  Last read time: %u ms\n", last_read);
    
    /* Reset */
    hx711_set_offset(&hx711_adv_1, 0);
    hx711_set_gain(&hx711_adv_1, HX711_CHANNEL_A_GAIN_128, true);
}

void demo_continuous_monitoring(void)
{
    if (!demo_initialized) return;
    
    printk("\n=== CONTINUOUS MONITORING DEMO ===\n");
    printk("Starting continuous monitoring for 10 seconds...\n");
    printk("Format: Time(ms) | Sensor0(g) | Sensor1(g) | Sensor2(g)\n");
    
    uint32_t start_time = k_uptime_get();
    uint32_t last_print = 0;
    
    while (k_uptime_get() - start_time < 10000) {  /* 10 seconds */
        uint32_t current_time = k_uptime_get();
        
        /* Print every 500ms */
        if (current_time - last_print >= 500) {
            float units_0 = hx711_get_units(&hx711_adv_0, 3);
            float units_1 = hx711_get_units(&hx711_adv_1, 3);
            float units_2 = hx711_get_units(&hx711_adv_2, 3);
            
            printk("%8u | %9.1f | %9.1f | %9.1f\n", 
                   current_time - start_time, (double)units_0, (double)units_1, (double)units_2);
            
            last_print = current_time;
        }
        
        k_msleep(100);
    }
    
    printk("Continuous monitoring completed.\n");
}

void print_sensor_status(struct hx711_advanced_data *hx711, const char *name)
{
    printk("%s Status:\n", name);
    printk("  Offset: %d\n", hx711_get_offset(hx711));
    printk("  Scale: %.6f\n", (double)hx711_get_scale(hx711));
    printk("  Tare offset: %.2f\n", (double)hx711_get_tare(hx711));
    printk("  Tare set: %s\n", hx711_tare_set(hx711) ? "Yes" : "No");
    printk("  Mode: %d\n", hx711_get_mode(hx711));
    printk("  Gain: %d\n", hx711_get_gain(hx711));
    printk("  Current units: %.2f\n", (double)hx711_get_units(hx711, 1));
}

void run_hx711_advanced_demo(void)
{
    printk("\n============================================================\n");
    printk("HX711 ADVANCED FUNCTIONS DEMONSTRATION\n");
    printk("============================================================\n");
    
    /* Initialize sensors */
    demo_initialize_sensors();
    if (!demo_initialized) {
        printk("Failed to initialize sensors. Demo cannot continue.\n");
        return;
    }
    
    /* Run all demos */
    demo_basic_reading();
    k_msleep(2000);
    
    demo_reading_modes();
    k_msleep(2000);
    
    demo_calibration_process();
    k_msleep(2000);
    
    demo_tare_functionality();
    k_msleep(2000);
    
    demo_advanced_features();
    k_msleep(2000);
    
    demo_continuous_monitoring();
    
    printk("\n============================================================\n");
    printk("HX711 ADVANCED DEMO COMPLETED\n");
    printk("============================================================\n");
} 