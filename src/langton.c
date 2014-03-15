#include "langton.h"


#define ROW_BYTES(w) (((w + 31) / 32) * 4)
#define PIXEL_BYTE(point) ((point.x / 8) + point.y * ROW_BYTES(grid.size.w) + sizeof(_bitmap_header))


typedef struct {
    uint16_t row_size_bytes;
    uint16_t info_flags;
    GPoint origin;
    GSize size;
} _bitmap_header;


static uint8_t *new_grid_data(GSize size) {
    size_t data_size;
    uint8_t *data;
    _bitmap_header header;

    header.row_size_bytes = ROW_BYTES(size.w);
    header.info_flags = (1 << 12);
    header.origin = GPointZero;
    header.size = size;

    data_size = header.row_size_bytes * size.h + sizeof(_bitmap_header);
    data = (uint8_t *) malloc(data_size);
    if (data == NULL) {
        APP_LOG(APP_LOG_LEVEL_WARNING, "NULL data!");
        return NULL;
    }

    memset(data, 0, data_size);
    memcpy(data, &header, sizeof(_bitmap_header));
    return data;
}

static GColor get_cell_colour(LangtonGrid grid, GPoint point) {
    uint8_t byte = grid.data[PIXEL_BYTE(point)];
    uint8_t mask = 1 << (point.x % 8);
    if (byte & mask) {
        return GColorBlack;
    } else {
        return GColorWhite;
    }
}


static void toggle_cell_colour(LangtonGrid grid, GPoint point) {
    uint8_t byte = grid.data[PIXEL_BYTE(point)];
    uint8_t mask = 1 << (point.x % 8);
    grid.data[PIXEL_BYTE(point)] = byte ^ mask;
}


static void langton_layer_update(LangtonLayer *langton_layer, GContext *ctx) {
    LangtonLayerData *data = (LangtonLayerData *)layer_get_data(langton_layer);
    GBitmap *bmp = data->grid.bitmap;

    graphics_context_set_compositing_mode(ctx, GCompOpAssignInverted);
    graphics_draw_bitmap_in_rect(ctx, bmp, bmp->bounds);
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
    data->grid.bitmap = gbitmap_create_with_data(data->grid.data);
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
        gbitmap_destroy(data->grid.bitmap);
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


static int16_t clamp(int16_t x, int16_t max) {
    while (x < 0) {
        x += max;
    }
    while (x >= max) {
        x -= max;
    }
    return x;
};


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
            ant->point.y = clamp(ant->point.y + 1, data->grid.size.h);
            break;
        case WEST:
            ant->point.x = clamp(ant->point.x - 1, data->grid.size.w);
            break;
        case SOUTH:
            ant->point.y = clamp(ant->point.y - 1, data->grid.size.h);
            break;
        case EAST:
            ant->point.x = clamp(ant->point.x + 1, data->grid.size.w);
            break;
        }
    }

    layer_mark_dirty(langton_layer);
}
