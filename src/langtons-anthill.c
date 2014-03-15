#include <pebble.h>

#include "langton.h"


#define UPDATE_INTERVAL_MS 200
#define STEPS_PER_UPDATE 10


static Window *window;
static LangtonLayer *langton_layer;
static AppTimer *timer;


static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Button: select");
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Button: up");
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Button: down");
}

static void click_config_provider(void *context) {
    window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
    window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
    window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}


static void ants_timer_callback(void *data) {
    for (uint8_t i = 0; i < STEPS_PER_UPDATE; i++) {
        step_ants(langton_layer);
    }
    timer = app_timer_register(UPDATE_INTERVAL_MS, ants_timer_callback, NULL);
}

static void window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    APP_LOG(APP_LOG_LEVEL_INFO, "Window bounds: (%d, %d), (%d, %d)",
            bounds.size.w, bounds.size.h, bounds.origin.x, bounds.origin.y);

    langton_layer = langton_layer_create(bounds, 4);
    layer_add_child(window_layer, langton_layer);
    timer = app_timer_register(UPDATE_INTERVAL_MS, ants_timer_callback, NULL);
}

static void window_unload(Window *window) {
    app_timer_cancel(timer);
    langton_layer_destroy(langton_layer);
}

static void init(void) {
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

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  app_event_loop();
  deinit();
}
