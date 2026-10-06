/* SPDX-License-Identifier: MIT */
#pragma once
#include <stdint.h>
#include <stdbool.h>

uint16_t charybdis_pointer_dpi(void);
uint16_t charybdis_precision_dpi(void);
uint8_t charybdis_brightness(void);
void charybdis_display_dpi_changed(bool precision);
void charybdis_display_brightness_changed(void);
