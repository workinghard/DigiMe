#include "batteryMask.h"

static Layer *s_batt_layer;
static int battLvl = 100;

//
// Draw battery layer
//
void batt_layer_update_callback(Layer *layer, GContext* ctx) {
  graphics_context_set_stroke_color(ctx, GColorWhite);
  
  // Block lines
  if ( battLvl > 10 ) {
    // Lowest level
    graphics_draw_rect (ctx, GRect(1, 29, 2, 9));
  }
  if ( battLvl > 39 ) {
    // Middle level
    graphics_draw_rect (ctx, GRect(1, 17, 2, 9));
  }
  if ( battLvl > 79 ) {
    // Highest level
    graphics_draw_rect (ctx, GRect(1, 5, 2, 9));
  }  
}

//
// Battery handler
//
static void battery_handler(BatteryChargeState new_state) {
  // Update the battery value and redraw the layer
  battLvl = new_state.charge_percent;
  layer_mark_dirty(s_batt_layer);
}

//
// Battery load functions
//
void batteryMask_load(Layer *window_layer) {
  // Init the battery layer
  s_batt_layer = layer_create(GRect(98, 122, 10, 50));
  layer_set_update_proc(s_batt_layer, batt_layer_update_callback);
  layer_add_child(window_layer, s_batt_layer);
  
  // Get the current battery level
  battery_handler(battery_state_service_peek());
  
  // Subscribe to the Battery State Service
  battery_state_service_subscribe(battery_handler);
}

//
// Battery unload function
void batteryMask_unload() {
  // Unsubscribe to the battery state service
  battery_state_service_unsubscribe();
  // Destroy the battery layer
  layer_destroy(s_batt_layer);
}