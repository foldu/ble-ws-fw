# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.1)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(ble-weatherstation)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")

target_sources(app PRIVATE src/main.c src/sensor.c src/mesh.c src/attention.c)
