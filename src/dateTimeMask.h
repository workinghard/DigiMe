#pragma once
#include <pebble.h>
#include "main.h"

#define MY_TIME_FGND_COLOR GColorWhite
#define MY_TIME_BGND_COLOR GColorBlack
#define MY_DATE_FGND_COLOR GColorWhite
#define MY_DATE_BGND_COLOR GColorBlack

void dateTimeMask_load(Layer *window_layer);
void dateTimeMask_unload();