#include <pebble.h>
#include "dayNightMask.h"

Layer *s_sun_layer;
Layer *s_backgr_layer;
Layer *s_frgnd_layer;
TextLayer *s_moon_font_layer;
GFont s_moon_font;
Layer *s_sky_layer;
Layer *s_black_layer;

// Feature request
// TODO: Distinguish between nothern/southern hemisphere

static const char *moon_phase_arr[] = { "0", "0", "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", 
                                        "0", "0", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z"};

int getMoonPhase() {
	long lp = 2551443; 
	// var now = new Date(year,month-1,day,20,35,0);						
  struct tm now = *localtime(&(time_t){getTime()});
	//var new_moon = new Date(1970, 0, 7, 20, 35, 0);
  struct tm newMoon = *localtime(&(time_t){getTime()});
  newMoon.tm_year = 70;
  newMoon.tm_mon = 0;
  newMoon.tm_mday = 7;
	//var phase = ((now.getTime() - new_moon.getTime())/1000) % lp;
  long phase = (mktime(&now) - mktime(&newMoon)) % lp;  
	return floor(phase /(24*3600)) + 1;
  //return 17;
}

static void sun_layer_update_proc(Layer *layer, GContext *ctx) {
  if ( isDay() ) {
    int centerX = 18;
    int centerY = 18;
    int sunSize = 7;
    
    // Draw the sun body
    graphics_context_set_stroke_width(ctx, 5);
    graphics_context_set_stroke_color(ctx, GColorPastelYellow);
    graphics_draw_circle(ctx, GPoint(centerX, centerY), sunSize);
    //graphics_context_set_stroke_color(ctx, GColorIcterine );
    //graphics_draw_circle(ctx, GPoint(centerX, centerY), 6);
    graphics_context_set_stroke_color(ctx, GColorYellow  );
    graphics_draw_circle(ctx, GPoint(centerX, centerY), 3);
    
  
    // Draw the sun arms
    graphics_context_set_stroke_width(ctx, 1);
    // right
    graphics_draw_line(ctx, GPoint(centerX+sunSize+6, centerY ), GPoint(centerX+sunSize+10, centerY));
    // left
    graphics_draw_line(ctx, GPoint(centerX-sunSize-10, centerY ), GPoint(centerX-sunSize-6, centerY));
    // top
    graphics_draw_line(ctx, GPoint(centerX, centerY+sunSize+6 ), GPoint(centerX, centerY+sunSize+10));
    // bot
    graphics_draw_line(ctx, GPoint(centerX, centerY-sunSize-10 ), GPoint(centerX, centerY-sunSize-6));
    // botright
    graphics_draw_line(ctx, GPoint(centerX+sunSize+2, centerY+sunSize+2), GPoint(centerX+sunSize+5, centerY+sunSize+5));
    // topright
    graphics_draw_line(ctx, GPoint(centerX+sunSize+2, centerY-sunSize-2), GPoint(centerX+sunSize+5, centerY-sunSize-5));
    // botleft
    graphics_draw_line(ctx, GPoint(centerX-sunSize-2, centerY+sunSize+2), GPoint(centerX-sunSize-5, centerY+sunSize+5));
    // topleft
    graphics_draw_line(ctx, GPoint(centerX-sunSize-2, centerY-sunSize-2), GPoint(centerX-sunSize-5, centerY-sunSize-5));
  }
}

int getDayPosY(int x){
  double dy = -0.008 * (x-55) * (x-55) + 25;
  int y = (int)dy;
  return 25 - y;
}
int getDayPosX(time_t sunrise, time_t sunset){
  time_t now = getTime();
  time_t dayDuration = sunset - sunrise;
  time_t dayTime = now - sunrise;
  int posX = 110 * dayTime / dayDuration;
  return posX;
}

int getNightPosY(int x){
  double dy = -0.012 * (x-58) * (x-58) + 40;
  int y = (int)dy;
  return 40 - y;
}
int getNightPosX(time_t sunrise, time_t sunset){
  // Doesn't matter if we have data for actual day or following one
  time_t now = getTime();
  time_t nightDuration = 86400 - (sunset - sunrise);
  time_t nightTime;
  if ( now < sunrise && now < sunset ) {
    // We have already data for the next day
    APP_LOG(APP_LOG_LEVEL_INFO, "SunsetCalc: %ld", sunset - 86400);
    nightTime = now - (sunset - 86400);
  }else{
    nightTime = now - sunset;     
  }
#ifdef SETDEBUG  
  APP_LOG(APP_LOG_LEVEL_INFO, "Nightime: %ld duration: %ld", nightTime, nightDuration);
#endif  
  int posX = 116 * nightTime / nightDuration;
  return posX;
}

void allLayerUpdate(bool isDay) {    
  if (isDay) {
    // Display all day layer
    layer_set_hidden(s_sun_layer, false);
    layer_set_hidden(s_frgnd_layer, false);
    layer_set_hidden(s_backgr_layer, false);
    // Hide all night layer
    layer_set_hidden(text_layer_get_layer(s_moon_font_layer), true);
    layer_set_hidden(s_sky_layer, true);
    layer_set_hidden(s_black_layer, true);
  }else{
    // Display all night layer
    layer_set_hidden(text_layer_get_layer(s_moon_font_layer), false);
    layer_set_hidden(s_sky_layer, false);
    layer_set_hidden(s_black_layer, false);

    // Hide all day layer
    layer_set_hidden(s_sun_layer, true);
    layer_set_hidden(s_frgnd_layer, true);
    layer_set_hidden(s_backgr_layer, true);
  }
  // Let all layer update
  layer_mark_dirty(s_sun_layer);
  layer_mark_dirty(s_frgnd_layer);
  layer_mark_dirty(s_backgr_layer);
  layer_mark_dirty(text_layer_get_layer(s_moon_font_layer));
  layer_mark_dirty(s_sky_layer);
  layer_mark_dirty(s_black_layer);
}

bool isNight(time_t now, time_t sunrise, time_t sunset) {
  bool isNight = false;
  if ( sunrise > now && sunset > now) { // We have data for the next day
    isNight = true;
  }else{
    if (((now-sunrise) > 0 ) && ((now-sunset) < 0)) {
      isNight = false;
    }else{
      isNight = true;
    }
  }
  return isNight;
}
bool isDay(){
  time_t sunset = getSunset();
  time_t sunrise = getSunrise();
  time_t now = getTime();
  bool isDayTime = true;
  if ( isNight(now, sunrise, sunset) ) {
    isDayTime = false;
  } 
  return isDayTime;
}

void dayNight_update_pos() {
  time_t sunset = getSunset();
  time_t sunrise = getSunrise();
  time_t now = getTime();
  //APP_LOG(APP_LOG_LEVEL_INFO, "Now: %ld", now);
  //APP_LOG(APP_LOG_LEVEL_INFO, "Sunset: %ld", sunset);
  //APP_LOG(APP_LOG_LEVEL_INFO, "Sunrise: %ld", sunrise);
  
  if ( isNight(now, sunrise, sunset) == false ) {
    // It's day
#ifdef SETDEBUG    
    APP_LOG(APP_LOG_LEVEL_INFO, "It's day");
#endif    
    // Calculate the position
    int posX = getDayPosX(sunrise, sunset);
    // Reposition the sun
    layer_set_frame(s_sun_layer, GRect(posX, getDayPosY(posX), 40, 40));
    // Make the sun layer with day background visible
    allLayerUpdate(true);
  }else{ 
    // It's night
#ifdef SETDEBUG    
    APP_LOG(APP_LOG_LEVEL_INFO, "Is night");
#endif
    int posX = getNightPosX(sunrise, sunset);
    // Reposition the moon
    layer_set_frame(text_layer_get_layer(s_moon_font_layer), GRect(posX, getNightPosY(posX),26,26));
    //layer_set_frame(text_layer_get_layer(s_moon_font_layer), GRect(70, 20,26,26));
    allLayerUpdate(false);
  }
}

static void backgr_layer_update_proc(Layer *layer, GContext *ctx) {
  if (isDay()) {
    GRect bounds = layer_get_bounds(layer);
    graphics_context_set_fill_color(ctx, GColorPictonBlue);
    graphics_fill_rect(ctx, GRect(0, 0, bounds.size.w, bounds.size.h/2), 0, GCornerNone);
    graphics_context_set_fill_color(ctx, GColorBlueMoon );
    graphics_fill_rect(ctx, GRect(0, bounds.size.h/2, bounds.size.w, bounds.size.h/2), 0, GCornerNone);
  }
}

static void forgr_layer_update_proc(Layer *layer, GContext *ctx) {
  if (isDay()) {
    GRect bounds = layer_get_bounds(layer);
    graphics_context_set_fill_color(ctx, GColorBlue );
    graphics_fill_rect(ctx, GRect(0, 0, bounds.size.w, bounds.size.h/2), 0, GCornerNone);  
    graphics_context_set_fill_color(ctx, MY_BACKGROUND_COLOR );
    graphics_fill_rect(ctx, GRect(0, bounds.size.h/2, bounds.size.w, bounds.size.h/2), 0, GCornerNone);  
  }
}

// Night background layer
static void sky_layer_update_proc(Layer *layer, GContext *ctx){
  if ( isDay() == false ){
    int i, xPos, yPos;
    GRect bounds = layer_get_bounds(layer);
    graphics_context_set_stroke_color(ctx, MY_LINE_FGND_COLOR);
    srand(100); // Make sure we have always the same sky
    for (i=0;i<200;i++){
      // Random position
      xPos = rand() % bounds.size.w;
      yPos = rand() % bounds.size.h;
      // Draw pixel
      graphics_draw_pixel(ctx, GPoint(xPos, yPos));
    }
  }
}

// Night foreground layer
static void black_layer_update_proc(Layer *layer, GContext *ctx){
  if ( isDay() == false ) {
    GRect bounds = layer_get_bounds(layer);
    graphics_context_set_fill_color(ctx, MY_BACKGROUND_COLOR );
    graphics_fill_rect(ctx, GRect(0, 0, bounds.size.w, bounds.size.h), 0, GCornerNone);    
  }
}

void dayNightMask_load(Layer *window_layer) {
  GRect bounds = layer_get_bounds(window_layer);
  
  // Draw foreground first
  s_sky_layer = layer_create(GRect(0,0, bounds.size.w, 48));
  layer_set_update_proc(s_sky_layer, sky_layer_update_proc);
  layer_add_child(window_layer, s_sky_layer);
    
  // draw moon
  s_moon_font_layer = text_layer_create(GRect(60, 0, 24, 24));
  text_layer_set_background_color(s_moon_font_layer, MY_BACKGROUND_COLOR);
  GColor moonColor;
  int moonPhase = getMoonPhase() - 1;
  if ( moonPhase == 0 || moonPhase == 1 ) { // don't have a font for the black moon
    moonColor = MY_BLACK_MOON_COLOR; 
  }else{
    moonColor = MY_MOON_COLOR;
  }
  text_layer_set_text_color(s_moon_font_layer, moonColor);
  text_layer_set_text_alignment(s_moon_font_layer, GTextAlignmentCenter);
  text_layer_set_text(s_moon_font_layer, moon_phase_arr[moonPhase]);
  layer_add_child(window_layer, text_layer_get_layer(s_moon_font_layer));
  
  // Load and set custom font
  s_moon_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_MOON_24));
  text_layer_set_font(s_moon_font_layer, s_moon_font);
  
  // Draw night background
  s_black_layer = layer_create(GRect(0,48, bounds.size.w, 24));
  layer_set_update_proc(s_black_layer, black_layer_update_proc);
  layer_add_child(window_layer, s_black_layer);
    
  // Draw background first
  s_backgr_layer = layer_create(GRect(0, 0, bounds.size.w, 36));
  layer_set_update_proc(s_backgr_layer, backgr_layer_update_proc);
  layer_add_child(window_layer, s_backgr_layer);
    
  // Draw the sun
  s_sun_layer = layer_create(GRect(0, 25, 40, 40));
  layer_set_update_proc(s_sun_layer, sun_layer_update_proc);
  layer_add_child(window_layer, s_sun_layer);
    
  // Draw foreground last
  s_frgnd_layer = layer_create(GRect(0, 36, bounds.size.w, 36));
  layer_set_update_proc(s_frgnd_layer, forgr_layer_update_proc);
  layer_add_child(window_layer, s_frgnd_layer);
    
  // Draw the layer time dependant
  dayNight_update_pos();
}
void dayNightMask_unload() {
  text_layer_destroy(s_moon_font_layer);
  layer_destroy(s_sky_layer);
  layer_destroy(s_sun_layer);
  layer_destroy(s_backgr_layer);
  layer_destroy(s_frgnd_layer);
  layer_destroy(s_black_layer);
  // Unload custom font
  fonts_unload_custom_font(s_moon_font);
}
