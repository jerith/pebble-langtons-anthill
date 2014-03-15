#include <pebble.h>

#include "langton.h"


#define UPDATE_INTERVAL_MS 100
#define STEPS_PER_UPDATE 20


static Window *window;
static LangtonLayer *langton_layer;
static AppTimer *timer;


static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Resetting.");
    langton_layer_reset(langton_layer);
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Adding ant and resetting.");
    langton_layer_add_ant(langton_layer);
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Removing ant and resetting.");
    langton_layer_remove_ant(langton_layer);
}

static void click_config_provider(void *context) {
    window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
    window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
    window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}


static void ants_timer_callback(void *data) {
    for (uint8_t i = 0; i < STEPS_PER_UPDATE; i++) {
        langton_layer_step_ants(langton_layer);
    }
    timer = app_timer_register(UPDATE_INTERVAL_MS, ants_timer_callback, NULL);
}

static void window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    langton_layer = langton_layer_create(bounds, 1);
    layer_add_child(window_layer, langton_layer);
    timer = app_timer_register(UPDATE_INTERVAL_MS, ants_timer_callback, NULL);
}

static void window_unload(Window *window) {
    app_timer_cancel(timer);
    langton_layer_destroy(langton_layer);
}

static void init(void) {
    srand(time(NULL));
    window = window_create();
    window_set_click_config_provider(window, click_config_provider);
    window_set_window_handlers(window, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload,
    });
    const bool animated = true;
    window_stack_push(window, animated);
}

static void deinit(void) {
    window_destroy(window);
}

int main(void) {
    init();
    app_event_loop();
    deinit();
}
