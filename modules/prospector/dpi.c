/* SPDX-License-Identifier: MIT */
#include <zephyr/device.h>
#include <zephyr/input/input.h>
#include <zephyr/sys/atomic.h>
#include <drivers/behavior.h>
#include <drivers/input_processor.h>
#include <zmk/behavior.h>
#include "dpi_math.h"
#include "prospector.h"
#include "step_metadata.h"

static atomic_t precision_index = ATOMIC_INIT(CHARYBDIS_DEFAULT_PRECISION_INDEX); /* Boot at 300 CPI. */
static atomic_t dpi_index = ATOMIC_INIT(CHARYBDIS_DEFAULT_DPI_INDEX); /* Boot at 800 CPI. */

uint16_t charybdis_pointer_dpi(void) { return charybdis_dpi_value(atomic_get(&dpi_index)); }

uint16_t charybdis_precision_dpi(void) { return charybdis_precision_dpi_value(atomic_get(&precision_index)); }

static int dpi_pressed(struct zmk_behavior_binding *binding,
                       struct zmk_behavior_binding_event event) {
    unsigned int index = atomic_get(&dpi_index);
    atomic_set(&dpi_index, charybdis_dpi_step(index, binding->param1 ? 1 : -1));
    charybdis_display_dpi_changed(false);
    return ZMK_BEHAVIOR_OPAQUE;
}

static int precision_pressed(struct zmk_behavior_binding *binding,
                             struct zmk_behavior_binding_event event) {
    unsigned int index = atomic_get(&precision_index);
    atomic_set(&precision_index, charybdis_dpi_step(index, binding->param1 ? 1 : -1));
    charybdis_display_dpi_changed(true);
    return ZMK_BEHAVIOR_OPAQUE;
}

static int dpi_released(struct zmk_behavior_binding *binding,
                        struct zmk_behavior_binding_event event) {
    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api dpi_behavior_api = {
    STEP_METADATA
    .binding_pressed = dpi_pressed,
    .binding_released = dpi_released,
};

BEHAVIOR_DT_DEFINE(DT_NODELABEL(dpi_step), NULL, NULL, NULL, NULL, POST_KERNEL,
                   CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &dpi_behavior_api);

static const struct behavior_driver_api precision_behavior_api = {
    STEP_METADATA
    .binding_pressed = precision_pressed,
    .binding_released = dpi_released,
};
BEHAVIOR_DT_DEFINE(DT_NODELABEL(precision_step), NULL, NULL, NULL, NULL, POST_KERNEL,
                   CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &precision_behavior_api);

static int dpi_process(const struct device *dev, struct input_event *event,
                       uint32_t param1, uint32_t param2,
                       struct zmk_input_processor_state *state) {
    if (event->type == INPUT_EV_REL &&
        (event->code == INPUT_REL_X || event->code == INPUT_REL_Y)) {
        event->value = charybdis_dpi_scale(event->value, dev == DEVICE_DT_GET(DT_NODELABEL(precision_dpi))
                                             ? charybdis_precision_dpi() : charybdis_pointer_dpi(),
                                         state ? state->remainder : NULL);
    }
    return ZMK_INPUT_PROC_CONTINUE;
}

static const struct zmk_input_processor_driver_api dpi_processor_api = {
    .handle_event = dpi_process,
};

DEVICE_DT_DEFINE(DT_NODELABEL(pointer_dpi), NULL, NULL, NULL, NULL, POST_KERNEL,
                 CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &dpi_processor_api);

DEVICE_DT_DEFINE(DT_NODELABEL(precision_dpi), NULL, NULL, NULL, NULL, POST_KERNEL,
                 CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &dpi_processor_api);
