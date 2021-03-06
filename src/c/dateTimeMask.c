#include "dateTimeMask.h"

// English
static const char *month_names_arr_e[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
static const char *day_names_arr_e[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
// German
static const char *month_names_arr_g[] = { "Jan", "Feb", "Mär", "Apr", "Mai", "Jun", "Jul", "Aug", "Sep", "Okt", "Nov", "Dez" };
static const char *day_names_arr_g[] = { "Son", "Mon", "Die", "Mit", "Don", "Fre", "Sam" };
// Spanish
static const char *month_names_arr_s[] = { "Ene", "Feb", "Mar", "Abr", "May", "Jun", "Jul", "Ago", "Sep", "Oct", "Nov", "Dic" };
static const char *day_names_arr_s[] = { "Dom", "Lun", "Mar", "Mie", "Jue", "Vie", "Sab" };

static TextLayer *s_date_layer;
static TextLayer *s_time_layer;

//
// Update the time layer
//
void local_update_time(struct tm* tick_time) {
  static char s_time_buffer[16];
  if (clock_is_24h_style()) {
    strftime(s_time_buffer, sizeof(s_time_buffer), "%H:%M", tick_time);
  } else {
    strftime(s_time_buffer, sizeof(s_time_buffer), "%I:%M", tick_time);
  }
  text_layer_set_text(s_time_layer, s_time_buffer);
  layer_mark_dirty(text_layer_get_layer(s_time_layer));
}

//
// Update the date layer
//
void local_update_date(struct tm* tick_time) {
  static char s_buffer[12];
  const char **month_names_arr;
  const char **day_names_arr;
  // Select the language
  switch (getLang()) {
    case 'e':
      // English
      month_names_arr = month_names_arr_e;
      day_names_arr = day_names_arr_e; 
      break;
    case 'g':
      // German
      month_names_arr = month_names_arr_g;
      day_names_arr = day_names_arr_g;
      break;
    case 's':
      // Spanish
      month_names_arr = month_names_arr_s;
      day_names_arr = day_names_arr_s;
      break;
    default:
      // Just in case ...
      month_names_arr = month_names_arr_e;
      day_names_arr = day_names_arr_e;       
  }
  
  //static char s_date_day[] = "XXX";
  //strftime(s_date_day,sizeof(s_date_day), "%a", tick_time);
  snprintf(s_buffer, sizeof(s_buffer), "%s %s %d",
             day_names_arr[tick_time->tm_wday],
             month_names_arr[tick_time->tm_mon], 
             tick_time->tm_mday);
  text_layer_set_text(s_date_layer, s_buffer);
  layer_mark_dirty(text_layer_get_layer(s_date_layer));
}

//
// handle the minute calls
//
static void handle_minute_tick(struct tm* tick_time, TimeUnits units_changed) {
  if ( (units_changed & MINUTE_UNIT) ) {
    // Update the minutes
    local_update_time(tick_time);
    // Update the sun/moon position
    dayNight_update_pos();
    
    if ( tick_time->tm_min == 0 ) {
      // Update the date
      local_update_date(tick_time);      
    }
    
    if (needToUpdate()) {
      // Request new weather data if needed
      owm_weather_fetch(weather_callback); 
    }
  }
}

//
// Force to update the time
//
void updateDateTime() {
  time_t now = getTime();
  struct tm *current_time = localtime(&now);
  local_update_time(current_time);
  local_update_date(current_time);
}

//
// DateTime load function
//
void dateTimeMask_load(Layer *window_layer) {
  // Create time layer
  GRect bounds = layer_get_bounds(window_layer);

  s_time_layer = text_layer_create(GRect(0, 68, bounds.size.w, 46));
  text_layer_set_text_color(s_time_layer, MY_TIME_FGND_COLOR);
  text_layer_set_background_color(s_time_layer, MY_TIME_BGND_COLOR);
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  
  // Create date layer
#ifdef PBL_PLATFORM_CHALK  
  s_date_layer = text_layer_create(GRect(30,126,80, 32));
#else
  s_date_layer = text_layer_create(GRect(16,126,80, 32));
#endif  
  text_layer_set_text_color(s_date_layer, MY_DATE_FGND_COLOR);
  text_layer_set_background_color(s_date_layer, MY_DATE_BGND_COLOR);
  text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentLeft);
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));
  
  // Ensures time is displayed immediately (will break if NULL tick event accessed).
  // (This is why it's a good idea to have a separate routine to do the update itself.)
  updateDateTime();

  
  // Subscribe to the timer service
  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
}

//
// Datetime unload function
//
void dateTimeMask_unload(){
  // Unsubscribe the timer
  tick_timer_service_unsubscribe();
  // destroy Date layer
  text_layer_destroy(s_date_layer);
  // Destroy Time layer
  text_layer_destroy(s_time_layer);
}