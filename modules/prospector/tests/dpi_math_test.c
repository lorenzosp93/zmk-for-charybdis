/* SPDX-License-Identifier: MIT */
#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include "../dpi_math.h"

int main(void) {
    assert(charybdis_dpi_value(CHARYBDIS_DEFAULT_DPI_INDEX) == 800);
    assert(charybdis_precision_dpi_value(CHARYBDIS_DEFAULT_PRECISION_INDEX) == 300);
    unsigned int index = 0;
    assert(charybdis_dpi_value(index) == 400);
    assert(charybdis_dpi_step(index, -1) == 0);
    for (unsigned int next = 1; next < CHARYBDIS_DPI_STEP_COUNT; next++) {
        index = charybdis_dpi_step(index, 1);
        assert(index == next);
        assert(charybdis_dpi_value(index) == (next + 1) * 400);
    }
    assert(charybdis_dpi_step(index, 1) == index);
    assert(charybdis_dpi_step(index, -1) == index - 1);

    // Slow sub-pixel movement must survive across reports in both directions.
    for (unsigned int i = 0; i < CHARYBDIS_DPI_STEP_COUNT; i++) {
        uint16_t dpi = charybdis_dpi_value(i);
        int16_t remainder = 0;
        int32_t positive = 0, negative = 0;
        for (int n = 0; n < 1600; n++) {
            positive += charybdis_dpi_scale(1, dpi, &remainder);
        }
        assert(positive == dpi && remainder == 0);
        for (int n = 0; n < 1600; n++) {
            negative += charybdis_dpi_scale(-1, dpi, &remainder);
        }
        assert(negative == -dpi && remainder == 0);
    }
    for (unsigned int i = 0; i < CHARYBDIS_DPI_STEP_COUNT; i++) {
        uint16_t dpi = charybdis_precision_dpi_value(i);
        assert(dpi == (i + 1) * 100);
        int16_t fraction = 0;
        int32_t total = 0;
        for (int n = 0; n < 1600; n++) total += charybdis_dpi_scale(1, dpi, &fraction);
        assert(total == dpi && fraction == 0);
        for (int n = 0; n < 1600; n++) total += charybdis_dpi_scale(-1, dpi, &fraction);
        assert(total == 0 && fraction == 0);
    }
    int16_t remainder = 0;
    assert(charybdis_dpi_scale(3, 400, &remainder) == 0);
    assert(charybdis_dpi_scale(-3, 400, &remainder) == 0 && remainder == 0);
    // A sensitivity change preserves fractions without spurious motion.
    assert(charybdis_dpi_scale(1, 400, &remainder) == 0);
    assert(charybdis_dpi_scale(1, 1200, &remainder) == 1 && remainder == 0);
    // Large movement cannot overflow the intermediate multiplier.
    assert(charybdis_dpi_scale(INT32_MAX, 1600, &remainder) == INT32_MAX);
    assert(charybdis_dpi_scale(INT32_MIN, 1600, &remainder) == INT32_MIN);
    assert(charybdis_dpi_scale(8, 400, NULL) == 2);
    return 0;
}
