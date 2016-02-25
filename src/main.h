#pragma once
#include <pebble.h>
#include "owm_weather.h"
#include "backgroundMask.h"
#include "batteryMask.h"
#include "dateTimeMask.h"
#include "dayNightMask.h"

#define TEMP_NOT_VALID -999

void weather_callback(OWMWeatherInfo *info, OWMWeatherStatus status);
bool needToUpdate();
int getOldTemp();
int getWeatherIcon();
time_t getSunrise();
time_t getSunset();