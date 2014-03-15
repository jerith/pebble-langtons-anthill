#pragma once

#include <pebble.h>

typedef enum { NORTH, SOUTH, EAST, WEST } LangtonDirection;

typedef struct {
    GSize size;
    uint8_t *data;
    GBitmap *bitmap;
} LangtonGrid;


typedef struct {
    GPoint point;
    LangtonDirection direction;
} LangtonAnt;


typedef Layer LangtonLayer;


typedef struct {
    LangtonGrid grid;
    uint8_t ant_count;
    LangtonAnt ants[10];
} LangtonLayerData;


LangtonLayer *langton_layer_create(GRect frame, uint8_t ant_count);
void langton_layer_destroy(LangtonLayer *langton_layer);
void langton_layer_reset(LangtonLayer *langton_layer);
void langton_layer_add_ant(LangtonLayer *langton_layer);
void langton_layer_remove_ant(LangtonLayer *langton_layer);
void langton_layer_step_ants(LangtonLayer *langton_layer);
