#include <lvgl.h>
#include <ctype.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define ARRAY_SIZE(a) (sizeof(a)/sizeof((a)[0]))
#define IS_ENABLED(option) (option)
#define CONFIG_PROSPECTOR_LAYER_ROLLER_ALL_CAPS 1
#define BIT(n) (1U << (n))
#define LOG_MODULE_DECLARE(...)
#define LOG_DBG(...)
#define ZMK_EV_EVENT_BUBBLE 0
#define ZMK_LISTENER(...)
#define ZMK_SUBSCRIPTION(...)
#define CONFIG_PROSPECTOR_FIXED_BRIGHTNESS 50
#define CONFIG_ZMK_KEYMAP_LAYER_NAME_MAX_LEN 20
#define ZMK_SPLIT_BLE_PERIPHERAL_COUNT 2
#define DT_NODELABEL(x) 0
#define DT_NODE_CHILD_IDX(x) 0
#define DEVICE_DT_GET_ONE(x) (&fake_device)
struct device {int unused;};
static struct device fake_device;
static int brightness;
static bool device_is_ready(const struct device *dev) {return true;}
static int led_set_brightness(const struct device *dev,int channel,int level) {brightness=level;return 0;}

typedef int atomic_t;
#define ATOMIC_INIT(value) (value)
static int atomic_get(atomic_t *a) {return *a;}
static void atomic_set(atomic_t *a,int value) {*a=value;}
static int64_t now;
static int64_t k_uptime_get(void) {return now;}
struct k_work {void (*callback)(struct k_work *);};
#define K_WORK_DEFINE(name,cb) struct k_work name={cb}
static void *zmk_display_work_q(void) {return NULL;}
static int k_work_submit_to_queue(void *q,struct k_work *w) {w->callback(w);return 0;}
static bool zmk_display_is_initialized(void) {return true;}
#define ZMK_ACTIVITY_ACTIVE 0
static int activity;
static int zmk_activity_get_state(void) {return activity;}
typedef void zmk_event_t;
struct zmk_caps_word_state_changed {bool active;};
static const struct zmk_caps_word_state_changed *as_zmk_caps_word_state_changed(const void *e) {return e;}
struct zmk_peripheral_battery_state_changed {uint8_t source;uint8_t state_of_charge;};
struct zmk_split_central_status_changed {uint8_t slot;bool connected;};
static const struct zmk_peripheral_battery_state_changed *as_zmk_peripheral_battery_state_changed(const void *e) {return e;}
static const struct zmk_split_central_status_changed *as_zmk_split_central_status_changed(const void *e) {return e;}
#define ZMK_DISPLAY_WIDGET_LISTENER(name,stype,callback,getter) static void name##_init(void) {}
static int fake_wpm=72, fake_layer=0;
static int fake_layer_id=-1;
static const char *fake_layer_name;
static int zmk_keymap_layer_index_to_id(int index) {return fake_layer_id < 0 ? index : fake_layer_id;}
static uint8_t fake_caps=2;
static int zmk_wpm_get_state(void) {return fake_wpm;}
static int zmk_keymap_highest_layer_active(void) {return fake_layer;}
static const char *zmk_keymap_layer_name(int layer) {const char *names[]={"QWERTY","Colemak","Symbols","Settings","Precision","Scroll"};return fake_layer_name ? fake_layer_name : names[layer];}
static uint8_t zmk_hid_indicators_get_current_profile(void) {return fake_caps;}
static struct {struct {uint8_t modifiers;} body;} fake_report={.body={.modifiers=3}};
#define zmk_hid_get_keyboard_report() (&fake_report)
static uint16_t fake_dpi=800;
static uint16_t charybdis_pointer_dpi(void) {return fake_dpi;}

typedef struct node {struct node *next;} sys_snode_t;
typedef struct {sys_snode_t *first;} sys_slist_t;
#define SYS_SLIST_STATIC_INIT(x) {NULL}
#define SYS_SLIST_FOR_EACH_CONTAINER(list,var,member) for((var)=(void *)(list)->first;(var);(var)=(void *)(var)->member.next)
static void sys_slist_append(sys_slist_t *list,sys_snode_t *node) {list->first=node;node->next=NULL;}
struct zmk_widget_battery_bar {sys_snode_t node;lv_obj_t *obj;};
extern const lv_font_t FoundryGridnikMedium_20;

static uint16_t fake_precision=300;
static uint16_t charybdis_precision_dpi(void) {return fake_precision;}
static uint8_t fake_brightness=50;
static uint8_t charybdis_brightness(void) {return fake_brightness;}

struct zmk_position_state_changed {uint8_t source;uint32_t position;bool state;int64_t timestamp;};
static const struct zmk_position_state_changed *as_zmk_position_state_changed(const void *event) {return event;}
