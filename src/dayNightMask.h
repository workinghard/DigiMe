#pragma once
#include <pebble.h>
#include "main.h"
#include <math.h>

void dayNightMask_load(Layer *window_layer);
void dayNightMask_unload();
void dayNight_update_pos();
bool isDay();
