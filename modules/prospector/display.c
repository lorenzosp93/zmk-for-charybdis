/* SPDX-License-Identifier: MIT */
#include <lvgl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/led.h>
#include <zephyr/sys/atomic.h>
#include <zmk/activity.h>
#include <zmk/display.h>
#include <zmk/event_manager.h>
#include <zmk/events/activity_state_changed.h>
#include <zmk/events/position_state_changed.h>
#include <zmk/events/caps_word_state_changed.h>
#include <zmk/hid.h>
#include <zmk/hid_indicators.h>
#include <zmk/keymap.h>
#include <zmk/wpm.h>
#include "widgets/battery_bar.h"
#include "prospector.h"

#define FG 0xf5f6f7
#define MUTED 0xa0a9b6
#define ACTIVE 0x30f5bd
#define BG 0x070809
#define BLUE 0x69baff
#define GOLD 0xffe45d
#define VIOLET 0xbd8aff
#define RED 0xff527c
#define PINK 0xff69df
#define WPM_MAX 150
#define WPM_ANIMATION_MS 400
#define DPI_NOTICE_MS 3000

static lv_obj_t *heading, *value, *context, *caps_label, *cw_label, *wpm_label;
static lv_obj_t *wpm_bar;
static int32_t animated_wpm;
static uint16_t last_default_dpi, last_precision_dpi;
static lv_obj_t *modifier_boxes[4];
static lv_obj_t *modifier_labels[4];
static struct zmk_widget_battery_bar battery_widget;
static atomic_t caps_word_active;
// This assembled keyboard paired right first. Infer the side from subsequent keys.
static atomic_t left_peripheral_slot = ATOMIC_INIT(1);
static int last_left_slot = -1;
static int64_t dpi_notice_until;
static uint8_t last_mods = UINT8_MAX;
static int last_wpm = -1, last_caps = -1, last_cw = -1, last_layer = -1;
static char last_layer_name[CONFIG_ZMK_KEYMAP_LAYER_NAME_MAX_LEN];
static bool notice_visible;
static bool ready;
enum notice_kind { NOTICE_NORMAL, NOTICE_PRECISION, NOTICE_BRIGHTNESS };
static atomic_t requested_notice;
static enum notice_kind notice_kind;
static void backlight_work_cb(struct k_work *work);

static lv_obj_t *label(lv_obj_t *parent, const char *text, int x, int y,
                       const lv_font_t *font, uint32_t color) {
    lv_obj_t *obj = lv_label_create(parent);
    lv_label_set_text(obj, text);
    lv_obj_set_style_text_font(obj, font, 0);
    lv_obj_set_style_text_color(obj, lv_color_hex(color), 0);
    lv_obj_set_pos(obj, x, y);
    return obj;
}

static lv_color_t wpm_color(int32_t scaled_wpm) {
    // Neon: cyan -> violet -> magenta -> orange, at 0/50/100/150 WPM.
    const uint32_t stops[] = {0x3bdfff, 0x9f7aff, PINK, 0xffb14a};
    int segment = scaled_wpm / 500;
    if (segment >= 3) return lv_color_hex(stops[3]);
    uint8_t mix = (scaled_wpm % 500) * 255 / 500;
    return lv_color_mix(lv_color_hex(stops[segment + 1]), lv_color_hex(stops[segment]), mix);
}

static void wpm_animation_cb(void *obj, int32_t scaled_wpm) {
    animated_wpm = scaled_wpm;
    lv_bar_set_value(obj, scaled_wpm, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(obj, wpm_color(scaled_wpm), LV_PART_INDICATOR);
}

static void update_wpm(int wpm) {
    if (wpm == last_wpm) return;
    lv_label_set_text_fmt(wpm_label, "%d WPM", wpm);
    int32_t target = (wpm < 0 ? 0 : wpm > WPM_MAX ? WPM_MAX : wpm) * 10;
    lv_anim_del(wpm_bar, wpm_animation_cb);
    if (last_wpm < 0) {
        wpm_animation_cb(wpm_bar, target);
    } else {
        lv_anim_t animation;
        lv_anim_init(&animation);
        lv_anim_set_var(&animation, wpm_bar);
        lv_anim_set_exec_cb(&animation, wpm_animation_cb);
        lv_anim_set_values(&animation, animated_wpm, target);
        lv_anim_set_time(&animation, WPM_ANIMATION_MS);
        lv_anim_set_path_cb(&animation, lv_anim_path_ease_out);
        lv_anim_start(&animation);
    }
    last_wpm = wpm;
}

static void render_center(void) {
    uint16_t normal = charybdis_pointer_dpi(), precision = charybdis_precision_dpi();
    if (normal != last_default_dpi || precision != last_precision_dpi) {
        lv_label_set_text_fmt(context, "DPI %u / %u", normal, precision);
        last_default_dpi = normal;
        last_precision_dpi = precision;
    }
    bool show_dpi = k_uptime_get() < dpi_notice_until;
    int layer = zmk_keymap_layer_index_to_id(zmk_keymap_highest_layer_active());
    const char *name = zmk_keymap_layer_name(layer);
    if (show_dpi) {
        lv_label_set_text(heading, notice_kind == NOTICE_BRIGHTNESS ? "DISPLAY BRIGHTNESS" :
                                    notice_kind == NOTICE_PRECISION ? "PRECISION DPI" : "DEFAULT DPI");
        lv_obj_set_style_text_color(value, lv_color_hex(notice_kind == NOTICE_BRIGHTNESS ? GOLD :
                                              notice_kind == NOTICE_PRECISION ? ACTIVE : BLUE), 0);
        lv_obj_set_y(value, lv_disp_get_ver_res(NULL) / 2 - 52);
        lv_obj_set_style_text_font(value, &lv_font_montserrat_48, 0);
        if (notice_kind == NOTICE_BRIGHTNESS) {
            lv_label_set_text_fmt(value, "%u%%", charybdis_brightness());
        } else {
            lv_label_set_text_fmt(value, "%u", notice_kind == NOTICE_PRECISION ? precision : normal);
        }
    } else if (notice_visible || layer != last_layer ||
               (name && strcmp(last_layer_name, name) != 0)) {
        lv_label_set_text(heading, "ACTIVE LAYER");
        const uint32_t layer_colors[] = {ACTIVE, VIOLET, GOLD, BLUE, PINK, RED};
        lv_obj_set_style_text_color(value, lv_color_hex(layer < ARRAY_SIZE(layer_colors) ?
                                                       layer_colors[layer] : FG), 0);
        lv_obj_set_y(value, lv_disp_get_ver_res(NULL) / 2 - 44);
        lv_obj_set_style_text_font(value, &lv_font_montserrat_40, 0);
        if (name && name[0]) {
            snprintf(last_layer_name, sizeof(last_layer_name), "%s", name);
            char display_name[sizeof(last_layer_name)];
            snprintf(display_name, sizeof(display_name), "%s", name);
            if (IS_ENABLED(CONFIG_PROSPECTOR_LAYER_ROLLER_ALL_CAPS)) {
                for (char *ch = display_name; *ch; ch++) *ch = toupper((unsigned char)*ch);
            }
            lv_label_set_text(value, display_name);
        } else {
            lv_label_set_text_fmt(value, "LAYER %d", layer);
        }
    }
    last_layer = layer;
    notice_visible = show_dpi;
}

/* This timer runs on the display queue, the same context as all LVGL calls.
 * Poll the final HID modifier byte so implicit and masked modifiers are also
 * represented correctly, without depending on keycode listener ordering. */
static void refresh(lv_timer_t *timer) {
    // Keep the upstream battery event/connection handling; add readable coloured values.
    if (battery_widget.obj) {
        int left_slot = atomic_get(&left_peripheral_slot);
        if (left_slot != last_left_slot) {
            // Reverse the flex presentation without changing child indices:
            // upstream battery/connection events still target their original slots.
            lv_obj_set_flex_flow(battery_widget.obj,
                                left_slot == 0 ? LV_FLEX_FLOW_ROW : LV_FLEX_FLOW_ROW_REVERSE);
            last_left_slot = left_slot;
        }
        for (int side = 0; side < 2; side++) {
            lv_obj_t *container = lv_obj_get_child(battery_widget.obj, side);
            lv_obj_t *bar = lv_obj_get_child(container, 0);
            lv_obj_t *number = lv_obj_get_child(container, 1);
            const char *text = lv_label_get_text(number);
            if (text[0] >= '0' && text[0] <= '9') {
                int level = atoi(text);
                lv_color_t color = lv_color_hex(level < 10 ? RED : level < 20 ? GOLD : ACTIVE);
                if (lv_obj_get_style_text_color(number, 0).full != color.full) {
                    lv_obj_set_style_text_color(number, color, 0);
                    lv_obj_set_style_bg_color(bar, color, LV_PART_INDICATOR);
                    lv_obj_set_style_bg_grad_color(bar, color, LV_PART_INDICATOR);
                }
            }
        }
    }
    update_wpm(zmk_wpm_get_state());
    int caps = (zmk_hid_indicators_get_current_profile() & BIT(1)) != 0;
    int cw = atomic_get(&caps_word_active);
    if (caps != last_caps) {
        lv_obj_set_style_text_color(caps_label, lv_color_hex(caps ? GOLD : MUTED), 0);
        last_caps = caps;
    }
    if (cw != last_cw) {
        lv_obj_set_style_text_color(cw_label, lv_color_hex(cw ? VIOLET : MUTED), 0);
        last_cw = cw;
    }
    uint8_t mods = zmk_hid_get_keyboard_report()->body.modifiers;
    if (mods != last_mods) {
        const uint32_t colors[] = {BLUE, GOLD, VIOLET, ACTIVE};
        const uint8_t masks[] = {BIT(0) | BIT(4), BIT(2) | BIT(6),
                                 BIT(3) | BIT(7), BIT(1) | BIT(5)};
        for (int i = 0; i < 4; i++) {
            bool active = (mods & masks[i]) != 0;
            lv_obj_set_style_bg_color(modifier_boxes[i], lv_color_hex(active ? colors[i] : 0x181e27), 0);
            lv_obj_set_style_text_color(modifier_labels[i], lv_color_hex(active ? BG : MUTED), 0);
        }
        last_mods = mods;
    }
    render_center();
}

static void dpi_notice_work_cb(struct k_work *work) {
    if (!ready) {
        return;
    }
    notice_kind = atomic_get(&requested_notice);
    backlight_work_cb(NULL);
    dpi_notice_until = k_uptime_get() + DPI_NOTICE_MS;
    render_center();
}
K_WORK_DEFINE(dpi_notice_work, dpi_notice_work_cb);

void charybdis_display_dpi_changed(bool precision) {
    atomic_set(&requested_notice, precision ? NOTICE_PRECISION : NOTICE_NORMAL);
    if (zmk_display_is_initialized()) {
        k_work_submit_to_queue(zmk_display_work_q(), &dpi_notice_work);
    }
}

void charybdis_display_brightness_changed(void) {
    atomic_set(&requested_notice, NOTICE_BRIGHTNESS);
    if (zmk_display_is_initialized()) {
        k_work_submit_to_queue(zmk_display_work_q(), &dpi_notice_work);
    }
}

// Split key events carry a stable pairing slot and a physical key position.
// Positions are unaffected by Studio's key assignments or the base layout.
static int left_slot_for_position(uint8_t source, uint32_t position) {
    if (source >= 2 || position >= 35) {
        return -1;
    }
    bool left = position < 30 ? position % 10 < 5 : position < 33;
    return left ? source : 1 - source;
}

static int peripheral_side_listener(const zmk_event_t *event) {
    const struct zmk_position_state_changed *ev = as_zmk_position_state_changed(event);
    if (ev->state) {
        int slot = left_slot_for_position(ev->source, ev->position);
        if (slot >= 0) {
            atomic_set(&left_peripheral_slot, slot);
        }
    }
    return ZMK_EV_EVENT_BUBBLE;
}
ZMK_LISTENER(charybdis_peripheral_side, peripheral_side_listener);
ZMK_SUBSCRIPTION(charybdis_peripheral_side, zmk_position_state_changed);

static int caps_word_listener(const zmk_event_t *event) {
    const struct zmk_caps_word_state_changed *ev = as_zmk_caps_word_state_changed(event);
    atomic_set(&caps_word_active, ev->active);
    return ZMK_EV_EVENT_BUBBLE;
}
ZMK_LISTENER(charybdis_caps_word, caps_word_listener);
ZMK_SUBSCRIPTION(charybdis_caps_word, zmk_caps_word_state_changed);

static void backlight_work_cb(struct k_work *work) {
    const struct device *leds = DEVICE_DT_GET_ONE(pwm_leds);
    if (device_is_ready(leds)) {
        led_set_brightness(leds, DT_NODE_CHILD_IDX(DT_NODELABEL(disp_bl)),
                           zmk_activity_get_state() == ZMK_ACTIVITY_ACTIVE
                               ? charybdis_brightness() : 0);
    }
}
K_WORK_DEFINE(backlight_work, backlight_work_cb);

static int activity_listener(const zmk_event_t *event) {
    if (zmk_display_is_initialized()) {
        k_work_submit_to_queue(zmk_display_work_q(), &backlight_work);
    }
    return ZMK_EV_EVENT_BUBBLE;
}
ZMK_LISTENER(charybdis_backlight, activity_listener);
ZMK_SUBSCRIPTION(charybdis_backlight, zmk_activity_state_changed);

lv_obj_t *zmk_display_status_screen(void) {
    const int width = lv_disp_get_hor_res(NULL);
    const int height = lv_disp_get_ver_res(NULL);
    lv_obj_t *screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, lv_color_hex(BG), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_all(screen, 0, 0);
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);

    caps_label = label(screen, "CAPS", 16, height - 99, &lv_font_montserrat_16, MUTED);
    cw_label = label(screen, "CW", 72, height - 99, &lv_font_montserrat_16, MUTED);
    wpm_label = label(screen, "0 WPM", 16, 12, &lv_font_montserrat_20, FG);
    lv_obj_set_width(wpm_label, 109);
    wpm_bar = lv_bar_create(screen);
    lv_obj_set_pos(wpm_bar, 125, 22);
    lv_obj_set_size(wpm_bar, width - 141, 8);
    lv_bar_set_range(wpm_bar, 0, WPM_MAX * 10);
    lv_obj_set_style_bg_color(wpm_bar, lv_color_hex(0x202632), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(wpm_bar, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(wpm_bar, LV_OPA_COVER, LV_PART_INDICATOR);
    lv_obj_set_style_radius(wpm_bar, 3, LV_PART_MAIN);
    lv_obj_set_style_radius(wpm_bar, 3, LV_PART_INDICATOR);

    // LVGL swaps the axes for the adapter's 90/270-degree orientation.
    // Use its logical size (280 x 240), not the panel's raw 240 x 280.
    heading = label(screen, "ACTIVE LAYER", 16, height / 2 - 66, &lv_font_montserrat_14, MUTED);
    value = label(screen, "", 16, height / 2 - 44, &lv_font_montserrat_40, FG);
    lv_obj_set_width(value, width - 32);
    lv_label_set_long_mode(value, LV_LABEL_LONG_DOT);
    context = label(screen, "", 110, height - 99, &lv_font_montserrat_16, 0xdae5f4);
    lv_obj_set_width(context, width - 126);
    lv_obj_set_style_text_align(context, LV_TEXT_ALIGN_RIGHT, 0);

    const char *names[] = {"CTRL", "ALT", "GUI", "SHIFT"};
    const int modifier_width = (width - 32 - 15) / 4;
    for (int i = 0; i < 4; i++) {
        modifier_boxes[i] = lv_obj_create(screen);
        lv_obj_remove_style_all(modifier_boxes[i]);
        lv_obj_set_pos(modifier_boxes[i], 16 + i * (modifier_width + 5), height - 75);
        lv_obj_set_size(modifier_boxes[i], modifier_width, 24);
        lv_obj_set_style_radius(modifier_boxes[i], 5, 0);
        lv_obj_set_style_bg_opa(modifier_boxes[i], LV_OPA_COVER, 0);
        lv_obj_clear_flag(modifier_boxes[i], LV_OBJ_FLAG_SCROLLABLE);
        modifier_labels[i] = label(modifier_boxes[i], names[i], 0, 0,
                                    &lv_font_montserrat_14, MUTED);
        lv_obj_center(modifier_labels[i]);
    }

    zmk_widget_battery_bar_init(&battery_widget, screen);
    lv_obj_t *battery = zmk_widget_battery_bar_obj(&battery_widget);
    lv_obj_set_size(battery, width, 47);
    for (int side = 0; side < 2; side++) {
        lv_obj_t *container = lv_obj_get_child(battery, side);
        lv_obj_set_style_text_font(lv_obj_get_child(container, 1), &lv_font_montserrat_28, 0);
        lv_obj_set_style_text_font(lv_obj_get_child(container, 3), &lv_font_montserrat_28, 0);
    }
    lv_obj_align(battery, LV_ALIGN_BOTTOM_MID, 0, 0);
    label(screen, "L", 17, height - 45, &lv_font_montserrat_14, MUTED);
    label(screen, "R", width / 2 + 12, height - 45, &lv_font_montserrat_14, MUTED);

    ready = true;
    refresh(NULL);
    lv_timer_create(refresh, 50, NULL);
    backlight_work_cb(NULL);
    return screen;
}
