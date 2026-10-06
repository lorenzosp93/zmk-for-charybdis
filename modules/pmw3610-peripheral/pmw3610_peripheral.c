/* SPDX-License-Identifier: MIT */

#include <zephyr/devicetree.h>
#include <zephyr/sys/util.h>
#include <zmk/keymap.h>

/*
 * The pinned inorichi driver unconditionally asks for the active layer.
 * ZMK excludes keymap.c on peripherals. Keep the sensor in MOVE mode here;
 * the central's input listener owns precision and scroll processing.
 */
BUILD_ASSERT(DT_PROP_LEN(DT_NODELABEL(trackball), scroll_layers) == 0,
             "Configure scroll layers on the central input listener");
BUILD_ASSERT(DT_PROP_LEN(DT_NODELABEL(trackball), snipe_layers) == 0,
             "Configure precision layers on the central input listener");
BUILD_ASSERT(DT_PROP(DT_NODELABEL(trackball), automouse_layer) < 0,
             "Automouse must be implemented on the central");

zmk_keymap_layer_index_t zmk_keymap_highest_layer_active(void) { return 0; }
