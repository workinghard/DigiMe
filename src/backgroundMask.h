#pragma once
#include <pebble.h>
#include "main.h"

#define MY_BACKGROUND_COLOR GColorBlack 
#define MY_MOON_COLOR GColorWhite
#define MY_BLACK_MOON_COLOR GColorDarkGray
#define MY_LINE_FGND_COLOR GColorWhite
#define MY_TEMP_FGND_COLOR GColorWhite
#define MY_TEMP_BGND_COLOR GColorBlack

void backgroundMask_load(Layer *window_layer);
void backgroundMask_unload();
void line_layer_update_callback(Layer *layer, GContext* ctx);
void temp_layer_update_text();