/* SPDX-License-Identifier: MIT */
#pragma once
#include <stdint.h>

#define CHARYBDIS_DPI_STEP_COUNT 4
#define CHARYBDIS_SENSOR_CPI 1600
#define CHARYBDIS_DEFAULT_DPI_INDEX 1
#define CHARYBDIS_DEFAULT_PRECISION_INDEX 2

static inline uint16_t charybdis_dpi_value(unsigned int index) {
    const uint16_t steps[CHARYBDIS_DPI_STEP_COUNT] = {400, 800, 1200, 1600};
    return steps[index];
}

static inline uint16_t charybdis_precision_dpi_value(unsigned int index) {
    const uint16_t steps[CHARYBDIS_DPI_STEP_COUNT] = {100, 200, 300, 400};
    return steps[index];
}

static inline unsigned int charybdis_dpi_step(unsigned int index, int direction) {
    if (direction > 0 && index + 1 < CHARYBDIS_DPI_STEP_COUNT) {
        return index + 1;
    }
    if (direction < 0 && index > 0) {
        return index - 1;
    }
    return index;
}

static inline int32_t charybdis_dpi_scale(int32_t value, uint16_t dpi, int16_t *remainder) {
    int64_t numerator = (int64_t)value * dpi + (remainder ? *remainder : 0);
    int32_t result = numerator / CHARYBDIS_SENSOR_CPI;
    if (remainder) {
        *remainder = numerator - (int64_t)result * CHARYBDIS_SENSOR_CPI;
    }
    return result;
}
