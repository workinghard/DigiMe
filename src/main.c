#include "main.h"

// Data store keys
#define KEY_SAVED_TEMPERATURE 0
#define KEY_SAVED_TIMESTAMP 1
#define KEY_SAVED_SUNRISE 2
#define KEY_SAVED_SUNSET 3


// Interval for refreshing the weather data
#define MY_REFRESH_INTERVAL 3600 


static Window *s_main_window;
static int oldTemp = TEMP_NOT_VALID;
static time_t lastFetch = 0.0;
static time_t sunrise = 0.0;
static time_t sunset = 0.0;
static int weatherIcon = 1;

//static int oldTemp = 0;
//static time_t lastFetch = 0.0;

//
// Weather callback function --- BEGIN
//
void weather_callback(OWMWeatherInfo *info, OWMWeatherStatus status) {
  if (status == OWMWeatherStatusAvailable) {
    // If we have valid response, update the values
    oldTemp = info->temp_f;
    // In case we are getting the next day information too early
    if (time(NULL) > sunset ) {
      sunrise = info->sunrise;
      sunset = info->sunset;  
      //APP_LOG(APP_LOG_LEVEL_INFO, "Updating sunrise");
    }
    // Trigger the update of the visual elements
    temp_layer_update_text();
    dayNight_update_pos();
    // save the timestamp
    lastFetch = time(NULL);    
    // Save the weather icon
    info->icon[2] = '\n'; // Cut off the day/night information
    weatherIcon = atoi(info->icon); // TODO: replace atoi. Make it more input robust!
    
    //APP_LOG(APP_LOG_LEVEL_INFO, "Icon: %d", icon);
    //APP_LOG(APP_LOG_LEVEL_INFO, "Time: %ld", lastFetch);
    //APP_LOG(APP_LOG_LEVEL_INFO, "Sunrise: %ld", info->sunrise);
  }
}
static void js_ready_handler(void *context) {
  owm_weather_fetch(weather_callback);
}
bool needToUpdate() {
  time_t now = time(NULL);
  bool needUpdate = false;
  // We don't have a valid data
  if ( oldTemp == TEMP_NOT_VALID || sunrise == 0.0 || sunset == 0.0 ) {
    needUpdate = true;
    APP_LOG(APP_LOG_LEVEL_INFO, "Need update: %d sunrise: %ld sunset: %ld", oldTemp, sunrise, sunset);
  }
  // Last update is too long ago
  if ( now - lastFetch > MY_REFRESH_INTERVAL) {
    needUpdate = true;
    APP_LOG(APP_LOG_LEVEL_INFO, "Need update: data too old %ld", now-lastFetch);
  }
  return needUpdate;
}
int getOldTemp(){
  return oldTemp;
}
time_t getSunrise() {
  return sunrise;
}
time_t getSunset() {
  return sunset;
}
int getWeatherIcon() {
  /* TODO: Make prefilter
01  	clear sky
02  	few clouds
-> Sunny
03  	scattered clouds
04  	broken clouds
-> Clouds
09  	shower rain
10  	rain
11  	thunderstorm
-> Rain
13  	snow
-> Snow
50  	mist
-> Mist
*/
  return weatherIcon;
}

void loadData() {
  if ( persist_exists(KEY_SAVED_TIMESTAMP) ) {
    persist_read_data(KEY_SAVED_TIMESTAMP, &lastFetch, sizeof(time_t));
  }
  if ( persist_exists(KEY_SAVED_TEMPERATURE) ){ 
    oldTemp = persist_read_int(KEY_SAVED_TEMPERATURE);
  }
  if ( persist_exists(KEY_SAVED_SUNRISE) ){
    persist_read_data(KEY_SAVED_SUNRISE, &sunrise, sizeof(time_t));
  }else{
    // if we do inital startup or after reset, just guess the value
    time_t now = time(NULL);
    struct tm *current_time = localtime(&now);
    if ( current_time->tm_hour >= 6 ) { // 
      sunrise = now - ( current_time->tm_hour - 6 ) * 3600;
    }else{
      sunrise = now + ( 6 - current_time->tm_hour ) * 3600;
    }
    APP_LOG(APP_LOG_LEVEL_INFO, "guess sunrise is: = %ld", sunrise);    
  }
  if ( persist_exists(KEY_SAVED_SUNSET) ){
    persist_read_data(KEY_SAVED_SUNSET, &sunset, sizeof(time_t));
  }else{
    time_t now = time(NULL);
    struct tm *current_time = localtime(&now);
    if ( current_time->tm_hour >= 18 ) { // 
      sunset = now - ( current_time->tm_hour - 18 ) * 3600;
    }else{
      sunset = now + ( 18 - current_time->tm_hour ) * 3600;
    }
    APP_LOG(APP_LOG_LEVEL_INFO, "guess sunset is: = %ld", sunset);   
  }
}

//
// Weather callback function -- END
//

//
// Window Functions ----- BEGIN
static void main_window_load(Window *window) {
  // Create Window's child Layers here  
  Layer *window_layer = window_get_root_layer(window);
 
  // Load the data
  loadData();

  // Init the weather framework
  char *api_key = "2cd34ef953aa6d4326799b5097536a0c";
  owm_weather_init(api_key);
  if (needToUpdate()) {
    // Register the JS handler delayed
    app_timer_register(3000, js_ready_handler, NULL);
  }
  APP_LOG(APP_LOG_LEVEL_INFO, "Now: = %ld", time(NULL));
  APP_LOG(APP_LOG_LEVEL_INFO, "lastFetch = %ld", lastFetch);
  
  // Init the line layer
  backgroundMask_load(window_layer);
  
  // Init the battery layer
  batteryMask_load(window_layer);
  
  // Init Date and Timer layer
  dateTimeMask_load(window_layer);
  
  // Init Day and Night layer
  dayNightMask_load(window_layer);
}

static void main_window_unload(Window *window) {  
  // Destroy background mask
  backgroundMask_unload();
  // Destroy battery mask
  batteryMask_unload();  
  // Destroy Date/Time Layer
  dateTimeMask_unload();
  // Destroy Day/Night Layer
  dayNightMask_unload();
}
// Window Functions ----- END
//

static void init(void) {
  s_main_window = window_create();
  window_set_background_color(s_main_window, MY_BACKGROUND_COLOR);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  window_stack_push(s_main_window, true);
}

static void deinit(void) {
  // Save the last temperature first
  persist_write_int(KEY_SAVED_TEMPERATURE, oldTemp);
  // Save the timestamp
  persist_write_data(KEY_SAVED_TIMESTAMP, &lastFetch, sizeof(time_t));
  // Save the sunrise
  persist_write_data(KEY_SAVED_SUNRISE, &sunrise, sizeof(time_t));
  // Save the sunset
  persist_write_data(KEY_SAVED_SUNSET, &sunset, sizeof(time_t));
  
  owm_weather_deinit();
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}