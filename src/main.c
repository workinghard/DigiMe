#include "main.h"

// Data store keys
#define KEY_SAVED_TEMPERATURE 0
#define KEY_SAVED_TIMESTAMP 1
#define KEY_SAVED_SUNRISE 2
#define KEY_SAVED_SUNSET 3
#define KEY_SAVED_FAHRENHEIT 4
#define KEY_SAVED_APIKEY 5

// Interval for refreshing the weather data
#define MY_REFRESH_INTERVAL 3600 

typedef enum {
  DIGIMEMessageKeySelTime = KeySelTime,
  DIGIMEMessageKeySelTemp = KeySelTemp,
  DIGIMEMessageKeySelAPIKEY = KeySelAPIKEY,
  DIGIMEKeySelLan = KeySelLan
} DIGIMEMessageKey;


static Window *s_main_window;
static int oldTemp = TEMP_NOT_VALID;
static time_t lastFetch = 0.0;
static time_t sunrise = 0.0;
static time_t sunset = 0.0;
static int weatherIcon = 1;
static bool temp_fahrenheit = true;
static char s_api_key[OWM_WEATHER_APIKEYSIZE];  

static const char api_key_default[] = "2cd34ef953aa6d4326799b5097536a0c";

//static int oldTemp = 0;
//static time_t lastFetch = 0.0;

//
// Weather callback function --- BEGIN
//
void weather_callback(OWMWeatherInfo *info, OWMWeatherStatus status) {
  if (status == OWMWeatherStatusAvailable) {
    // If we have valid response, update the values
    if (temp_fahrenheit) {
      oldTemp = info->temp_f;      
    }else{
      oldTemp = info->temp_c;
    }
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
#ifdef SETDEBUG    
    APP_LOG(APP_LOG_LEVEL_INFO, "Need update: %d sunrise: %ld sunset: %ld", oldTemp, sunrise, sunset);
#endif
  }
  // Last update is too long ago
  if ( now - lastFetch > MY_REFRESH_INTERVAL) {
    needUpdate = true;
#ifdef SETDEBUG     
    APP_LOG(APP_LOG_LEVEL_INFO, "Need update: data too old %ld", now-lastFetch);
#endif    
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
bool isFahrenheit() {
  return temp_fahrenheit;
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
#ifdef SETDEBUG     
    APP_LOG(APP_LOG_LEVEL_INFO, "guess sunrise is: = %ld", sunrise);    
#endif
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
#ifdef SETDEBUG     
    APP_LOG(APP_LOG_LEVEL_INFO, "guess sunset is: = %ld", sunset);  
#endif    
  }
  if ( persist_exists(KEY_SAVED_FAHRENHEIT) ){
    temp_fahrenheit = persist_read_bool(KEY_SAVED_FAHRENHEIT);
  }
  if ( persist_exists(KEY_SAVED_APIKEY) ){
    persist_read_data(KEY_SAVED_APIKEY, &s_api_key, sizeof(s_api_key));
  }else{
    strncpy(s_api_key, api_key_default, sizeof(api_key_default));
    s_api_key[34] = '\n'; // termination
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
  owm_weather_init(s_api_key);
  if (needToUpdate()) {
    // Register the JS handler delayed
    app_timer_register(3000, js_ready_handler, NULL);
  }
#ifdef SETDEBUG   
  APP_LOG(APP_LOG_LEVEL_INFO, "Now: = %ld", time(NULL));
  APP_LOG(APP_LOG_LEVEL_INFO, "lastFetch = %ld", lastFetch);
#endif  
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


//-----------------------------------------------------------------------
// OWMWeather starts here ...
//-----------------------------------------------------------------------
static OWMWeatherInfo *s_info;
static OWMWeatherCallback *s_callback;
static OWMWeatherStatus s_status;

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
#ifdef SETDEBUG   
  APP_LOG(APP_LOG_LEVEL_INFO, "Message received!");
#endif  
  // Parse response
  bool configResponse = false;
  /*  
  Tuple *select_time = dict_find(iter, DIGIMEMessageKeySelTime);
  if(select_time) {
    bool twenty_four_format = select_time->value->int32 == 1;
    if ( twenty_four_format) {
      APP_LOG(APP_LOG_LEVEL_INFO, "24 hour format");      
    }else{
      APP_LOG(APP_LOG_LEVEL_INFO, "12 hour format"); 
    }
    configResponse = true;
  }
  */
  Tuple *select_temp = dict_find(iter, DIGIMEMessageKeySelTemp);
  if(select_temp) {
    temp_fahrenheit = select_temp->value->int32 == 1;
    configResponse = true;

    /*
    if ( temp_fahrenheit ) {
      APP_LOG(APP_LOG_LEVEL_INFO, "Fahrenheit format");      
    }else{
      APP_LOG(APP_LOG_LEVEL_INFO, "Celsius format"); 
    }
    */
  }
  Tuple *select_api_key = dict_find(iter, DIGIMEMessageKeySelAPIKEY);
  if (select_api_key) {
    strncpy(s_api_key, select_api_key->value->cstring, OWM_WEATHER_APIKEYSIZE);
    // Check if it looks like correct API key
    if (strlen(s_api_key) < 32) {
      // Field is empty or small API key, take default
      strncpy(s_api_key, api_key_default, sizeof(api_key_default));
#ifdef SETDEBUG      
      APP_LOG(APP_LOG_LEVEL_INFO, "Take default key"); 
#endif      
    }
  }
  if (configResponse) {
    // Request new weather data
    owm_weather_fetch(weather_callback); 
  }
  
  Tuple *reply_tuple = dict_find(iter, OWMWeatherAppMessageKeyReply);
  if(reply_tuple) {

    Tuple *desc_short_tuple = dict_find(iter, OWMWeatherAppMessageKeyDescriptionShort);
    strncpy(s_info->description_short, desc_short_tuple->value->cstring, OWM_WEATHER_BUFFER_SIZE);
    
    Tuple *desc_name_tuple = dict_find(iter, OWMWeatherAppMessageKeyName);
    strncpy(s_info->name, desc_name_tuple->value->cstring, OWM_WEATHER_BUFFER_SIZE);

    Tuple *sunr_tuple = dict_find(iter, OWMWeatherAppMessageKeySunrise);
    s_info->sunrise = sunr_tuple->value->int32;
#ifdef SETDEBUG    
    APP_LOG(APP_LOG_LEVEL_INFO, "OWM Sunrise: %ld", s_info->sunrise);
#endif    
    Tuple *suns_tuple = dict_find(iter, OWMWeatherAppMessageKeySunset);
    s_info->sunset = suns_tuple->value->int32;
#ifdef SETDEBUG    
    APP_LOG(APP_LOG_LEVEL_INFO, "OWM Sunset: %ld", s_info->sunset);
#endif    
    Tuple *temp_tuple = dict_find(iter, OWMWeatherAppMessageKeyTempK);
    s_info->temp_k = temp_tuple->value->int32;
    s_info->temp_c = s_info->temp_k - 273; 
    s_info->temp_f = (temp_tuple->value->int32 - 273.15)* 1.8000 + 32.00;
    //s_info->temp_f = ((s_info->temp_c * 9) / 5 /* *1.8 or 9/5 */) + 32;
    
    Tuple *icon_tuple = dict_find(iter, OWMWeatherAppMessageKeyIcon);
    strncpy(s_info->icon, icon_tuple->value->cstring, OWM_WEATHER_BUFFER_SIZE);
    
    s_info->timestamp = time(NULL);

    s_status = OWMWeatherStatusAvailable;
    //app_message_deregister_callbacks();
    s_callback(s_info, s_status);
  }

  Tuple *err_tuple = dict_find(iter, OWMWeatherAppMessageKeyBadKey);
  if(err_tuple) {
    s_status = OWMWeatherStatusBadKey;
    s_callback(s_info, s_status);
  }

  err_tuple = dict_find(iter, OWMWeatherAppMessageKeyLocationUnavailable);
  if(err_tuple) {
    s_status = OWMWeatherStatusLocationUnavailable;
    s_callback(s_info, s_status);
  }
}

static void fail_and_callback() {
#ifdef SETDEBUG  
  APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to send request!");
#endif
  s_status = OWMWeatherStatusFailed;
  s_callback(s_info, s_status);
}

static bool fetch() {
  DictionaryIterator *out;
  AppMessageResult result = app_message_outbox_begin(&out);
  if(result != APP_MSG_OK) {
    fail_and_callback();
    return false;
  }

  dict_write_cstring(out, OWMWeatherAppMessageKeyRequest, s_api_key);

  result = app_message_outbox_send();
  if(result != APP_MSG_OK) {
    fail_and_callback();
    return false;
  }

  s_status = OWMWeatherStatusPending;
  s_callback(s_info, s_status);
  return true;
}

void owm_weather_init(char *api_key) {
  if(s_info) {
    free(s_info);
  }

  if(!api_key) {
#ifdef SETDEBUG    
    APP_LOG(APP_LOG_LEVEL_ERROR, "API key was NULL!");
#endif
    return;
  }

  strncpy(s_api_key, api_key, sizeof(s_api_key));

  s_info = (OWMWeatherInfo*)malloc(sizeof(OWMWeatherInfo));
  s_status = OWMWeatherStatusNotYetFetched;
}

bool owm_weather_fetch(OWMWeatherCallback *callback) {
  if(!s_info) {
#ifdef SETDEBUG    
    APP_LOG(APP_LOG_LEVEL_ERROR, "OWM Weather library is not initialized!");
#endif
    return false;
  }

  if(!callback) {
#ifdef SETDEBUG    
    APP_LOG(APP_LOG_LEVEL_ERROR, "OWMWeatherCallback was NULL!");
#endif
    return false;
  }

  s_callback = callback;

  if(!bluetooth_connection_service_peek()) {
    s_status = OWMWeatherStatusBluetoothDisconnected;
    s_callback(s_info, s_status);
    return false;
  }

  return fetch();
}

void owm_weather_deinit() {
  if(s_info) {
    free(s_info);
    s_info = NULL;
    s_callback = NULL;
  }
}

OWMWeatherInfo* owm_weather_peek() {
  if(!s_info) {
#ifdef SETDEBUG    
    APP_LOG(APP_LOG_LEVEL_ERROR, "OWM Weather library is not initialized!");
#endif
    return NULL;
  }

  return s_info;
}

//---------------------------------


static void init(void) {
  s_main_window = window_create();
  window_set_background_color(s_main_window, MY_BACKGROUND_COLOR);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  window_stack_push(s_main_window, true);
  
  app_message_register_inbox_received(inbox_received_handler);
  app_message_open(2026, 656);
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
  // Save the temperature format 
  persist_write_bool(KEY_SAVED_FAHRENHEIT, temp_fahrenheit);
  // Save API key 
  persist_write_data(KEY_SAVED_APIKEY, s_api_key, sizeof(s_api_key));

  
  owm_weather_deinit();
  window_destroy(s_main_window);
  app_message_deregister_callbacks();
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}