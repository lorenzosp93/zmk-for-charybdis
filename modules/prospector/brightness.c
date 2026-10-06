/* SPDX-License-Identifier: MIT */
#include <zephyr/sys/atomic.h>
#include <drivers/behavior.h>
#include <zmk/behavior.h>
#include "dpi_math.h"
#include "prospector.h"
#include "step_metadata.h"

static atomic_t brightness_index = ATOMIC_INIT(1); /* Start at 50%. */
uint8_t charybdis_brightness(void) { return (atomic_get(&brightness_index) + 1) * 25; }

static int brightness_pressed(struct zmk_behavior_binding *binding,
                              struct zmk_behavior_binding_event event) {
    atomic_set(&brightness_index, charybdis_dpi_step(atomic_get(&brightness_index),
                                                   binding->param1 ? 1 : -1));
    charybdis_display_brightness_changed();
    return ZMK_BEHAVIOR_OPAQUE;
}
static int brightness_released(struct zmk_behavior_binding *binding,
                               struct zmk_behavior_binding_event event) {
    return ZMK_BEHAVIOR_OPAQUE;
}
static const struct behavior_driver_api brightness_api = {
    STEP_METADATA
    .binding_pressed = brightness_pressed,
    .binding_released = brightness_released,
};
BEHAVIOR_DT_DEFINE(DT_NODELABEL(brightness_step), NULL, NULL, NULL, NULL, POST_KERNEL,
                   CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &brightness_api);
