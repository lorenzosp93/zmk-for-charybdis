/* SPDX-License-Identifier: MIT */
#pragma once

#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)
static const struct behavior_parameter_value_metadata step_values[] = {
    {.display_name = "Decrease", .type = BEHAVIOR_PARAMETER_VALUE_TYPE_VALUE, .value = 0},
    {.display_name = "Increase", .type = BEHAVIOR_PARAMETER_VALUE_TYPE_VALUE, .value = 1},
};
static const struct behavior_parameter_metadata_set step_sets[] = {
    {.param1_values = step_values, .param1_values_len = ARRAY_SIZE(step_values)},
};
static const struct behavior_parameter_metadata step_metadata = {
    .sets = step_sets, .sets_len = ARRAY_SIZE(step_sets),
};
#define STEP_METADATA .parameter_metadata = &step_metadata,
#else
#define STEP_METADATA
#endif
