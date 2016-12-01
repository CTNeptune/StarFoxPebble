#include <pebble.h>
  
static Window *s_main_window;
static TextLayer *s_time_layer, *s_shield_layer;
static Layer *pebble_battery_layer;

static int batteryPblPercent;
static char s_shield_buffer[] = "SHIELD";

static BitmapLayer *s_bitmap_layer;
static GBitmap *s_bg_bitmap;

static GFont *s_time_font;
static GFont *s_shield_font;

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Create a long-lived buffer
  static char buffer[] = "00:00";
  // Write the current hour
  strftime(buffer, sizeof(buffer), "%I:%M", tick_time);
  
  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, buffer);
  
  text_layer_set_text(s_shield_layer, s_shield_buffer);
}

void pebble_battery_layer_update_callback(Layer *me, GContext* ctx) {
	
	graphics_context_set_stroke_color(ctx, GColorBlack);
	graphics_context_set_fill_color(ctx, GColorBlack);

	graphics_fill_rect(ctx, GRect(0, 2, (int)((batteryPblPercent/100.0)*30.0), 20), 0, GCornerNone);
}
void batteryChanged(BatteryChargeState batt) {
	batteryPblPercent = batt.charge_percent;
	layer_mark_dirty(pebble_battery_layer);
	
}

static void main_window_load(Window *window) {
  // Create time TextLayer
  s_time_layer = text_layer_create(GRect(0, 16, 144, 50));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  
  
  s_shield_layer = text_layer_create(GRect(34,132,48,32));
  text_layer_set_background_color(s_shield_layer, GColorClear);
  text_layer_set_text_color(s_shield_layer,GColorBlack);
  
  // Improve the layout to be more like a watchface
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_DIGITAL_24));
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  
  s_shield_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_DIGITAL_12));
  text_layer_set_font(s_shield_layer,s_shield_font);
  text_layer_set_text_alignment(s_shield_layer,GTextAlignmentLeft);
  
  //Create the GBitmap, specifying the 'Identifier' chosen earlier, prefixed with RESOURCE_ID_. This will manage the image data:
  s_bg_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BG);
  
  //Declare a bitmap layer
  static BitmapLayer *s_bitmap_layer;
  
  //Create bitmap layer and set it to show the GBitmap
  s_bitmap_layer = bitmap_layer_create(GRect(0,0,144,168));
  bitmap_layer_set_bitmap(s_bitmap_layer,s_bg_bitmap);
  
  //Add the bitmaplayer as a child layer to the window:
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_bitmap_layer));
  
  // Add text layer as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_shield_layer));
  
  pebble_battery_layer = layer_create(GRect(34, 144, 32, 10));
	layer_set_update_proc(pebble_battery_layer, pebble_battery_layer_update_callback);
	layer_add_child(window_get_root_layer(window), pebble_battery_layer);
  layer_mark_dirty(pebble_battery_layer);
  
}

static void main_window_unload(Window *window) {
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_shield_layer);
  layer_destroy(pebble_battery_layer);

  //Destroy bitmap layer
  bitmap_layer_destroy(s_bitmap_layer);
  gbitmap_destroy(s_bg_bitmap);
  fonts_unload_custom_font(s_time_font);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  // Make sure the time is displayed from the start
  update_time();
  
  layer_mark_dirty(pebble_battery_layer);
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  battery_state_service_subscribe(batteryChanged);
  
  BatteryChargeState pbl_batt = battery_state_service_peek();
	batteryPblPercent = pbl_batt.charge_percent;
}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}