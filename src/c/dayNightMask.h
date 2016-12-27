#pragma once
#include <pebble.h>
#include "main.h"
#include <math.h>

#ifdef PBL_ROUND
#define XOFFSET 20
#else
#define XOFFSET 0
#endif

void dayNightMask_load(Layer *window_layer);
void dayNightMask_unload();
void dayNight_update_pos();
bool isDay();
