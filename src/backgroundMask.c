#include "backgroundMask.h"

static Layer *s_line_layer;      // Simple line
static TextLayer *s_temp_layer;  // Temperature

//
// Draw functions for the line layer
//
void line_layer_update_callback(Layer *layer, GContext* ctx) {
  graphics_context_set_stroke_color(ctx, MY_LINE_FGND_COLOR);
  // Horizontal line
  graphics_draw_line(ctx, GPoint(10, 0), GPoint(136, 0));
  graphics_draw_line(ctx, GPoint(10, 1), GPoint(136, 1)); 
}

//
// Draw functions for the temperature layer
//
void temp_layer_update_text() {
  if ( getOldTemp() != TEMP_NOT_VALID ) {
    static char s_buffer[5];
    snprintf(s_buffer, sizeof(s_buffer), "%d°",  getOldTemp());
    text_layer_set_text(s_temp_layer, s_buffer);
    layer_mark_dirty(text_layer_get_layer(s_temp_layer));
  }
}

//
// Everything what needs to happen if the watchface display again
//
void backgroundMask_load(Layer *window_layer) {
  // Init the line layer
  GRect bounds = layer_get_bounds(window_layer);
  s_line_layer = layer_create(GRect(0, 120, bounds.size.w, 5)); 
  layer_set_update_proc(s_line_layer, line_layer_update_callback); 
  layer_add_child(window_layer, s_line_layer);
  
  // Init the temperature layer
  s_temp_layer = text_layer_create(GRect(105,126,30, 32));
  text_layer_set_text_color(s_temp_layer, MY_TEMP_FGND_COLOR);
  text_layer_set_background_color(s_temp_layer, MY_TEMP_BGND_COLOR);
  text_layer_set_font(s_temp_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_text_alignment(s_temp_layer, GTextAlignmentRight);
  // Use the cached temperature
  temp_layer_update_text();
  layer_add_child(window_layer, text_layer_get_layer(s_temp_layer));
}

//
// We are going in the background. Free everything not needed
//
void backgroundMask_unload() {
  // destroy line layer
  layer_destroy(s_line_layer);
  // destro temperature layer
  text_layer_destroy(s_temp_layer);
}