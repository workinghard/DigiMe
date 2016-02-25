#pragma once
#include <pebble.h>

void batt_layer_update_callback(Layer *layer, GContext* ctx);
void batteryMask_load(Layer *window_layer);
void batteryMask_unload();