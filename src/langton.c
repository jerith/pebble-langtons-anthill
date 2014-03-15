#include "langton.h"


static uint8_t *new_grid_data(GSize size) {
    return (uint8_t *) malloc(size.w * size.h);
}

static GColor get_cell_colour(LangtonGrid grid, GPoint point) {
    if (grid.data[point.x + point.y * grid.size.w]) {
        return GColorBlack;
    } else {
        return GColorWhite;
    }
}


static void toggle_cell_colour(LangtonGrid grid, GPoint point) {
    uint8_t colour = grid.data[point.x + point.y * grid.size.w];
    grid.data[point.x + point.y * grid.size.w] = !colour;
}


static void langton_layer_update(LangtonLayer *langton_layer, GContext *ctx) {
    LangtonLayerData *data = (LangtonLayerData *)layer_get_data(langton_layer);
    GPoint point;
    GColor colour;

    for (int16_t y = 0; y < data->grid.size.h; y++) {
        for (int16_t x = 0; x < data->grid.size.w; x++) {
            point = (GPoint) {x, y};
            colour = get_cell_colour(data->grid, point);
            graphics_context_set_stroke_color(ctx, colour);
            graphics_draw_pixel(ctx, (GPoint) {x, y});
        }
    }
}


LangtonLayer *langton_layer_create(GRect frame, uint8_t ant_count) {
    LangtonLayer *langton_layer = layer_create_with_data(frame, sizeof(LangtonLayerData));
    LangtonLayerData *data = (LangtonLayerData *)layer_get_data(langton_layer);

    if (ant_count < 1) {
        ant_count = 1;
    }
    if (ant_count > 10) {
        ant_count = 10;
    }

    data->grid.size = frame.size;
    data->grid.data = new_grid_data(frame.size);
    APP_LOG(APP_LOG_LEVEL_WARNING, "data->grid.data: %p", data->grid.data);
    data->ant_count = ant_count;

    for (uint8_t i = 0; i < ant_count; i++) {
        data->ants[i].point = (GPoint) {
            (frame.size.w / (ant_count * 2)) + (frame.size.w * i) / ant_count,
            (frame.size.h / (ant_count * 2)) + (frame.size.h * i) / ant_count,
        };
        data->ants[i].direction = NORTH;
    }

    layer_set_update_proc(langton_layer, langton_layer_update);
    layer_mark_dirty(langton_layer);
    return langton_layer;
}


void langton_layer_destroy(LangtonLayer *langton_layer) {
    LangtonLayerData *data = (LangtonLayerData *)layer_get_data(langton_layer);
    if (data->grid.data != NULL) {
        free((void *) data->grid.data);
    }
    layer_destroy(langton_layer);
}


static LangtonDirection turn_left(LangtonDirection direction) {
    switch (direction) {
    case NORTH:
        return WEST;
    case WEST:
        return SOUTH;
    case SOUTH:
        return EAST;
    case EAST:
        return NORTH;
    default:
        return NORTH;
    };
}


static LangtonDirection turn_right(LangtonDirection direction) {
    switch (direction) {
    case NORTH:
        return EAST;
    case EAST:
        return SOUTH;
    case SOUTH:
        return WEST;
    case WEST:
        return NORTH;
    default:
        return NORTH;
    };
}


void step_ants(LangtonLayer *langton_layer) {
    LangtonLayerData *data = (LangtonLayerData *)layer_get_data(langton_layer);
    LangtonAnt *ant;
    uint8_t i;

    // Turn ants
    for (i = 0; i < data->ant_count; i++) {
        ant = &(data->ants[i]);
        if (get_cell_colour(data->grid, ant->point) == GColorWhite) {
            ant->direction = turn_right(ant->direction);
        } else {
            ant->direction = turn_left(ant->direction);
        }
    }

    // Update grid
    for (i = 0; i < data->ant_count; i++) {
        // TODO: Deduplicate cells
        toggle_cell_colour(data->grid, data->ants[i].point);
    }

    // Advance ants
    for (i = 0; i < data->ant_count; i++) {
        ant = &(data->ants[i]);
        switch(ant->direction) {
        case NORTH:
            ant->point.y = (ant->point.y + 1) % data->grid.size.h;
            break;
        case WEST:
            ant->point.x = (ant->point.x - 1) % data->grid.size.w;
            break;
        case SOUTH:
            ant->point.y = (ant->point.y - 1) % data->grid.size.h;
            break;
        case EAST:
            ant->point.x = (ant->point.x + 1) % data->grid.size.w;
            break;
        }
    }

    layer_mark_dirty(langton_layer);
}
