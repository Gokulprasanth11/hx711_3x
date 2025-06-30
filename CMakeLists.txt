# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.21.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(hx711_2025)

target_sources(app PRIVATE src/main.c src/hx711_driver.c) 
