#include <pebble.h>
#include <ctype.h>
  
#include "pebble_process_info.h"
#include "weather_images.h"
#include "graphics_simple.h"

extern const PebbleProcessInfo __pbl_app_info;

#define KEY_MMS_TEMPERATURE 0
#define KEY_MMS_CONDITIONS 1
#define KEY_MMS_BLUETOOTH 2
#define KEY_MMS_BATTERY 3
#define KEY_MMS_HOURLY_BEEP 4
#define KEY_MMS_SCREEN_COLOR 5
#define KEY_MMS_DND 6
#define KEY_MMS_DND_START 7
#define KEY_MMS_DND_END 8
#define KEY_MMS_HOUR_LEAD_ZERO 9
#define KEY_MMS_HUMIDITY 10
#define KEY_MMS_SHOW_HUMIDITY 11
  
static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_bottom_info_layer;

#ifdef PBL_ROUND
  static TextLayer *s_temperature_layer;
#endif
  
static Layer *s_weather_image_layer;

static Layer *s_bluetooth_layer;

static bool last_bluetooth_connection;

static bool weather_out_of_date;

static char s_config_bluetooth[32];
static char s_config_battery[32];
static char s_config_screen_color[7];
static char s_config_hourly_beep[32];
static char s_config_dnd[32];
static uint8_t s_config_dnd_start;
static uint8_t s_config_dnd_end;
static char s_time_buffer[32];
static char s_temperature_buffer[8];
static uint8_t s_humidity_buffer;
static bool s_config_show_humidity;
static char s_date_buffer[32];
static char s_day_of_week_buffer[32];

static uint8_t battery_level;
static bool battery_plugged;
static bool battery_charging;
static Layer *s_battery_layer;
static void battery_handler();
static void battery_layer_update_callback(Layer *layer, GContext *ctx);

static int16_t get_current_version();

static void update_time(struct tm *tick_time);
static void update_weather();
static void update_weather_image(Layer *layer, GContext *ctx);

static void bluetooth_handler(bool connected);
static void bluetooth_layer_update(Layer *layer, GContext *ctx);

static GColor get_main_color();
static GColor get_main_color_inverted();
static GColor get_background_color();
static GColor get_middle_color();
static void update_layers_color();
static const char *get_random_color();

static Layer *s_background_layer;
static void draw_background(Layer *layer, GContext *ctx);
unsigned int HexStringToUInt(char const *hexstring);
static void update_background_color();
static void set_bottom_info();
static void persist_read_and_set();

static int LAYER_VERTICAL_INDENT = 0;

int last_beeped = 0;
static void make_beep();

static bool is_silence(int32_t hour);

static bool hour_lead_zero;

static void update_time(struct tm *tick_time) {
  // Write the current hours and minutes into the buffer
  if(clock_is_24h_style() == true) {
    //Use 24 hour format
    if (hour_lead_zero) {
      strftime(s_time_buffer, sizeof(s_time_buffer), "%H:%M", tick_time);
    }
    else {
      strftime(s_time_buffer, sizeof(s_time_buffer), "%k:%M", tick_time);
    }
  } else {
    //Use 12 hour format
    if (hour_lead_zero) {
      strftime(s_time_buffer, sizeof(s_time_buffer), "%I:%M", tick_time);
    }
    else {
      strftime(s_time_buffer, sizeof(s_time_buffer), "%l:%M", tick_time);
    }
  }
  
  if (strncmp(s_time_buffer, " ", 1) == 0) {
    clock_copy_time_string(s_time_buffer, sizeof("0:00"));
  }
  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, s_time_buffer);
  
  static bool update_bottom_info = false;
  if (0 == strlen(s_date_buffer) || (tick_time->tm_min==0 && tick_time->tm_sec==0)) {
    static char bufferDayNum[] = "01";
    strftime(bufferDayNum, sizeof(bufferDayNum), "%e", tick_time);
    
    static char bufferMonth[32];
    strftime(bufferMonth, sizeof(bufferMonth), "%b", tick_time);
    
    if (strncmp(bufferDayNum, " ", 1) == 0) {
      snprintf(s_date_buffer, sizeof(s_date_buffer), "%s%s", bufferMonth, bufferDayNum);
    }
    else {
      snprintf(s_date_buffer, sizeof(s_date_buffer), "%s %s", bufferMonth, bufferDayNum);
    }
    update_bottom_info = true;
  }
  
  if (0 == strlen(s_day_of_week_buffer) || (tick_time->tm_min==0 && tick_time->tm_sec==0)) {
    strftime(s_day_of_week_buffer, sizeof(s_day_of_week_buffer), "%a", tick_time);
    update_bottom_info = true;
  }
  
  if (update_bottom_info) {
    set_bottom_info();
  }
  
  if (last_beeped!=tick_time->tm_hour
      && !is_silence(tick_time->tm_hour)
      && strcmp(s_config_hourly_beep, "beep")==0
//       && tick_time->tm_min==0
      && tick_time->tm_sec==0) {
    last_beeped = tick_time->tm_hour;
    make_beep();
  }
}

static void main_window_load(Window *window) {
  // Read saved data from persistent store
  persist_read_and_set();
  APP_LOG(APP_LOG_LEVEL_INFO, "Make Me Smile version %d.%d",
          (int)(get_current_version()/10), get_current_version()%10);
  
  int pbl_round_correction = 0;
  int no_leco_correction = 0;
  GRect window_bounds = layer_get_bounds(window_get_root_layer(window));
  
  if (228 == window_bounds.size.h) {
    LAYER_VERTICAL_INDENT = 44;
  }
  
  // Create bottom background Layer
  s_background_layer = layer_create(window_bounds);
  layer_set_update_proc(s_background_layer, draw_background);
  layer_add_child(window_get_root_layer(window), s_background_layer);
  
  // Create weather image layer
  s_weather_image_layer = layer_create(GRect(0, 7, window_bounds.size.w, 101+LAYER_VERTICAL_INDENT));
  layer_set_update_proc(s_weather_image_layer, update_weather_image);
  layer_add_child(window_get_root_layer(window), s_weather_image_layer);
  
  // Create time TextLayer
  #ifndef FONT_KEY_LECO_38_BOLD_NUMBERS
  no_leco_correction = 4;
  #endif
  s_time_layer = text_layer_create(GRect(0, 100+LAYER_VERTICAL_INDENT+no_leco_correction, window_bounds.size.w, 38+LAYER_VERTICAL_INDENT/4));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, get_main_color());
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  #ifdef FONT_KEY_LECO_38_BOLD_NUMBERS
    text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_LECO_38_BOLD_NUMBERS));
  #else
    text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_34_MEDIUM_NUMBERS));
  #endif
  #ifdef FONT_KEY_LECO_42_NUMBERS
    if (228 == window_bounds.size.h) {
      text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS));
    }
  #endif
  // Add it as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
  
  // Create bottom info Layer
  #ifdef PBL_ROUND
    pbl_round_correction = -6;
  #endif
  s_bottom_info_layer = text_layer_create(GRect(0, 138+LAYER_VERTICAL_INDENT+LAYER_VERTICAL_INDENT/4+pbl_round_correction, window_bounds.size.w, 30+(int)(LAYER_VERTICAL_INDENT/8)));
  text_layer_set_background_color(s_bottom_info_layer, GColorClear);
  text_layer_set_text_color(s_bottom_info_layer, get_main_color());
  text_layer_set_text_alignment(s_bottom_info_layer, GTextAlignmentCenter);
  if (228 == window_bounds.size.h) {
    text_layer_set_font(s_bottom_info_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  }
  else {
    text_layer_set_font(s_bottom_info_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  }
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_bottom_info_layer));
  
  #ifdef PBL_ROUND
    s_temperature_layer = text_layer_create(GRect(72, 151, 50, 30));
    text_layer_set_background_color(s_temperature_layer, GColorClear);
    text_layer_set_text_color(s_temperature_layer, get_main_color());
    text_layer_set_text_alignment(s_temperature_layer, GTextAlignmentCenter);
    text_layer_set_font(s_temperature_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_temperature_layer));
  #endif
  
  // Handle current bluetooth connection state
  #ifdef PBL_ROUND
  s_bluetooth_layer = layer_create(GRect(12, 110, 20, 20));
  #else
  s_bluetooth_layer = layer_create(GRect(3, 2, 20, 20));
  #endif
  layer_set_update_proc(s_bluetooth_layer, bluetooth_layer_update);
  layer_add_child(window_get_root_layer(window), s_bluetooth_layer);
  
  bluetooth_handler(bluetooth_connection_service_peek());
  
  // Pebble Battery
  BatteryChargeState initial = battery_state_service_peek();
  battery_level = initial.charge_percent;
  battery_plugged = initial.is_plugged;
  battery_charging = initial.is_charging;
  #ifdef PBL_ROUND
  s_battery_layer = layer_create(GRect(window_bounds.size.w-23, 110, 8, 22));
  #else
  s_battery_layer = layer_create(GRect(-3+window_bounds.size.w-22, 4, 22, 8));
  #endif
  layer_set_update_proc(s_battery_layer, battery_layer_update_callback);
  layer_add_child(window_get_root_layer(window), s_battery_layer);
  
  battery_handler();
}

static void main_window_unload(Window *window) {
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);
  
  #ifdef PBL_ROUND
    text_layer_destroy(s_temperature_layer);
  #endif
  
  // Destroy date elements
  text_layer_destroy(s_bottom_info_layer);

  // Destroy layers
  layer_destroy(s_background_layer);
  layer_destroy(s_weather_image_layer);
  layer_destroy(s_bluetooth_layer);
  layer_destroy(s_battery_layer);
  
  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time(tick_time);
  
  // Get weather update every 30 minutes
  if(tick_time->tm_min % 30 == 0 || (weather_out_of_date && tick_time->tm_min % 2 == 0)) {
    update_weather();
  }
}

static void update_weather() {
  // If bluetooth connection exists
  if(bluetooth_connection_service_peek()) {
    // Begin dictionary
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    // Add a key-value pair
    dict_write_uint8(iter, 0, 0);

    // Send the message!
    app_message_outbox_send();
  }
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  static bool update_bottom_info = false;
  
  Tuple *temperature_tuple = dict_find(iterator, MESSAGE_KEY_TEMPERATURE);
  if (temperature_tuple) {
    snprintf(s_temperature_buffer, sizeof(s_temperature_buffer), "%s", temperature_tuple->value->cstring);
    persist_write_string(KEY_MMS_TEMPERATURE, s_temperature_buffer);
    update_bottom_info = true;
  }
  
  Tuple *conditions_tuple = dict_find(iterator, MESSAGE_KEY_CONDITIONS);
  if (conditions_tuple) {
    persist_write_int(KEY_MMS_CONDITIONS, (int)conditions_tuple->value->int32);
    layer_mark_dirty(s_weather_image_layer);
  }
  
  Tuple *bluetooth_tuple = dict_find(iterator, MESSAGE_KEY_BLUETOOTH);
  if (bluetooth_tuple) {
    snprintf(s_config_bluetooth, sizeof(s_config_bluetooth), "%s", bluetooth_tuple->value->cstring);
    persist_write_string(KEY_MMS_BLUETOOTH, s_config_bluetooth);
    // Handle current bluetooth connection state
    bluetooth_handler(bluetooth_connection_service_peek());
    update_weather();
  }
  
  Tuple *battery_tuple = dict_find(iterator, MESSAGE_KEY_BATTERY);
  if (battery_tuple) {
    if (battery_tuple->value->uint8 != 0) {
      snprintf(s_config_battery, sizeof(s_config_battery), "battery_always");
    }
    else {
      snprintf(s_config_battery, sizeof(s_config_battery), "on_charge");
    }
    persist_write_string(KEY_MMS_BATTERY, s_config_battery);
    battery_handler();
  }
  
  Tuple *screen_color_tuple = dict_find(iterator, MESSAGE_KEY_SCREEN_COLOR);
  if (screen_color_tuple) {
    snprintf(s_config_screen_color, sizeof(s_config_screen_color), "%s", screen_color_tuple->value->cstring);
    persist_write_string(KEY_MMS_SCREEN_COLOR, s_config_screen_color);
    update_layers_color();
  }
  
  Tuple *hourly_beep_tuple = dict_find(iterator, MESSAGE_KEY_HOURLY_BEEP);
  if (hourly_beep_tuple) {
    if (hourly_beep_tuple->value->uint8 != 0) {
      snprintf(s_config_hourly_beep, sizeof(s_config_hourly_beep), "beep");
    }
    else {
      snprintf(s_config_hourly_beep, sizeof(s_config_hourly_beep), "do_not_beep");
    }
    persist_write_string(KEY_MMS_HOURLY_BEEP, s_config_hourly_beep);
  }
  
  Tuple *hourly_lead_zero_tuple = dict_find(iterator, MESSAGE_KEY_HOUR_LEAD_ZERO);
  if (hourly_lead_zero_tuple) {
    hour_lead_zero = hourly_lead_zero_tuple->value->uint8 != 0;
    persist_write_bool(KEY_MMS_HOUR_LEAD_ZERO, hour_lead_zero);
    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);
    update_time(tick_time);
  }
  
  Tuple *dnd_tuple = dict_find(iterator, MESSAGE_KEY_DND);
  if (dnd_tuple) {
    if (dnd_tuple->value->uint8 != 0) {
      snprintf(s_config_dnd, sizeof(s_config_dnd), "dnd_on");
    }
    else {
      snprintf(s_config_dnd, sizeof(s_config_dnd), "dnd_off");
    }
    persist_write_string(KEY_MMS_DND, s_config_dnd);
  }
  
  Tuple *dnd_start_tuple = dict_find(iterator, MESSAGE_KEY_DND_START);
  if (dnd_start_tuple) {
    s_config_dnd_start = (int)dnd_start_tuple->value->int32;
    persist_write_int(KEY_MMS_DND_START, s_config_dnd_start);
  }
  
  Tuple *dnd_end_tuple = dict_find(iterator, MESSAGE_KEY_DND_END);
  if (dnd_end_tuple) {
    s_config_dnd_end = (int)dnd_end_tuple->value->int32;
    persist_write_int(KEY_MMS_DND_END, s_config_dnd_end);
  }
  
  Tuple *humidity_tuple = dict_find(iterator, MESSAGE_KEY_HUMIDITY);
  if (humidity_tuple) {
    s_humidity_buffer = (int)humidity_tuple->value->int32;
    s_humidity_buffer = s_humidity_buffer==100 ? 99 : s_humidity_buffer;
    persist_write_int(KEY_MMS_HUMIDITY, s_humidity_buffer);
    update_bottom_info = true;
  }
  
  Tuple *show_humidity_tuple = dict_find(iterator, MESSAGE_KEY_SHOW_HUMIDITY);
  if (show_humidity_tuple) {
    s_config_show_humidity = show_humidity_tuple->value->uint8 != 0;
    persist_write_bool(KEY_MMS_SHOW_HUMIDITY, s_config_show_humidity);
    update_bottom_info = true;
  }
  
  if (update_bottom_info) {
    set_bottom_info();
  }
}

static void bluetooth_handler(bool connected) {
  layer_mark_dirty(s_bluetooth_layer);
  if (!connected) {
    // Get a tm structure
    time_t temp = time(NULL); 
    struct tm *tick_time = localtime(&temp);
    
    if (last_bluetooth_connection != connected && battery_plugged == false && !is_silence(tick_time->tm_hour)) {
      // Vibe pattern: ON for 300ms:
      static const uint32_t segments[] = {300};
      VibePattern pat = {
        .durations = segments,
        .num_segments = ARRAY_LENGTH(segments),
      };
      vibes_enqueue_custom_pattern(pat);
    }
  }
  last_bluetooth_connection = connected;
}

static void bluetooth_layer_update(Layer *layer, GContext *ctx) {
  if (bluetooth_connection_service_peek()) {
    if (strcmp(s_config_bluetooth, "always") == 0) {
      // BLUETOOTH_CONNECTED
      draw_bluetooth(ctx, get_main_color());
    }
    else {
      // EMPTY
      draw_hide_bluetooth(ctx, get_background_color());
    }
  } else if (strcmp(s_config_bluetooth, "never") != 0) {
    // BLUETOOTH_DISCONNECTED
    draw_bluetooth_disconnected(ctx, get_main_color(), get_background_color(), get_middle_color());
  }
}

static void battery_handler() {
  BatteryChargeState charge_state = battery_state_service_peek();
  battery_level = charge_state.charge_percent;
  battery_plugged = charge_state.is_plugged;
  battery_charging = charge_state.is_charging;
  layer_mark_dirty(s_battery_layer);
}

static void battery_layer_update_callback(Layer *layer, GContext *ctx) {
  #if defined(PBL_ROUND)
  char *direction = "vertical";
  #else
  char *direction = "horizontal";
  #endif
  if (battery_plugged && !battery_charging) {
    draw_hide_battery_lighting(ctx, get_background_color(), direction);
    draw_battery_charged(ctx, get_main_color(), 7, direction);
  }
  else {
    if (battery_plugged || strcmp(s_config_battery, "battery_always")==0 || battery_level<=20) {
      draw_battery_filled(ctx, get_main_color(), get_background_color(), battery_level, 7, direction);
    }
    if (battery_plugged && battery_charging) {
      draw_battery_lighting(ctx, get_main_color(), direction);
    }
    else {
      draw_hide_battery_lighting(ctx, get_background_color(), direction);
    }
  }
}

static void update_layers_color() {
  text_layer_set_text_color(s_bottom_info_layer, get_main_color());
  text_layer_set_text_color(s_time_layer, get_main_color());
  #ifdef PBL_ROUND
    text_layer_set_text_color(s_temperature_layer, get_main_color());
  #endif
  layer_mark_dirty(s_background_layer);
  layer_mark_dirty(s_weather_image_layer);
  layer_mark_dirty(s_bluetooth_layer);
  layer_mark_dirty(s_battery_layer);
}

static void set_bottom_info() {
  static char date_weather_buffer[32];
  static char bufferMiddleStr[32];
  if (s_config_show_humidity) {
    snprintf(bufferMiddleStr, sizeof(bufferMiddleStr), "%d%%", s_humidity_buffer);
  }
  else {
    snprintf(bufferMiddleStr, sizeof(bufferMiddleStr), " %s ", s_day_of_week_buffer);
  }
  
  #if defined(PBL_ROUND)
    if (strlen(s_temperature_buffer)) {
      text_layer_set_text(s_temperature_layer, s_temperature_buffer);
    }
    if (!s_config_show_humidity) {
      snprintf(date_weather_buffer, sizeof(date_weather_buffer), " %s |%s", s_date_buffer, bufferMiddleStr);
    }
    else if (s_config_show_humidity && s_humidity_buffer) {
      snprintf(date_weather_buffer, sizeof(date_weather_buffer), "%s | %s", s_date_buffer, bufferMiddleStr);
    }
    else {
      snprintf(date_weather_buffer, sizeof(date_weather_buffer), "%s", s_date_buffer);
    }
  #else
    if (strlen(s_temperature_buffer) && (!s_config_show_humidity || (s_config_show_humidity && s_humidity_buffer))) {
      snprintf(date_weather_buffer, sizeof(date_weather_buffer), "%s |%s| %s", s_date_buffer, bufferMiddleStr, s_temperature_buffer);
    }
    else if (strlen(s_temperature_buffer)) {
      snprintf(date_weather_buffer, sizeof(date_weather_buffer), "%s | %s", s_date_buffer, s_temperature_buffer);
    }
    else if (!s_config_show_humidity || (s_config_show_humidity && s_humidity_buffer)) {
      snprintf(date_weather_buffer, sizeof(date_weather_buffer), "%s |%s", s_date_buffer, bufferMiddleStr);
    }
    else {
      snprintf(date_weather_buffer, sizeof(date_weather_buffer), "%s", s_date_buffer);
    }
  #endif
  
  text_layer_set_text(s_bottom_info_layer, date_weather_buffer);
}

unsigned int HexStringToUInt(char const *hexstring) {
  unsigned int result = 0;
  char const *c = hexstring;
  unsigned char thisC;

  while( (thisC = *c) != 0 ) {
    thisC = toupper(thisC);
    result <<= 4;

    if (isdigit(thisC)) {
      result += thisC - '0';
    }
    else if (isxdigit(thisC)) {
      result += thisC - 'A' + 10;
    }
    else {
      APP_LOG(APP_LOG_LEVEL_DEBUG, "ERROR: Unrecognised hex character \"%c\"", thisC);
      return 0;
    }
    ++c;
  }
  return result;  
}

static GColor get_main_color() {
  #if defined(PBL_BW)
    if (strcmp(s_config_screen_color, "FFFFFF")==0 || strlen(s_config_screen_color)==0) {
      return GColorBlack;
    }
    else {
      return GColorWhite;
    }
  #elif defined(PBL_COLOR)
    return GColorBlack;
  #endif
}

static GColor get_main_color_inverted() {
  #if defined(PBL_BW)
    if (strcmp(s_config_screen_color, "FFFFFF")==0 || strlen(s_config_screen_color)==0) {
      return GColorWhite;
    }
    else {
      return GColorBlack;
    }
  #elif defined(PBL_COLOR)
    return GColorWhite;
  #endif
}

static GColor get_middle_color() {
  #if defined(PBL_BW)
    return get_main_color();
  #elif defined(PBL_COLOR)
    return GColorFromHEX(0x333333);
  #endif
}

static GColor get_background_color() {
  #if defined(PBL_BW)
    if (strcmp(s_config_screen_color, "FFFFFF")==0 || strlen(s_config_screen_color)==0) {
      return GColorWhite;
    }
    else {
      return GColorBlack;
    }
  #elif defined(PBL_COLOR)
    if (strcmp(s_config_screen_color, "auto")==0 || strlen(s_config_screen_color)==0) {
      snprintf(s_config_screen_color, sizeof(s_config_screen_color), "%s", get_random_color());
    }
    return GColorFromHEX(HexStringToUInt(s_config_screen_color));
  #endif
}

static const char *get_random_color() {
  #if defined(PBL_BW)
    return "FFFFFF";
  #elif defined(PBL_COLOR)
    static const char *colors[] = {
      "55FF00", "00FF00", "55FF55", "00FF55", "00AA00",
      "00AA55", "55FFAA", "00FFAA", "55FFFF", "00FFFF",
      "00AAAA", "0055AA", "00AAFF", "55AAFF", "0055FF",
      "5500FF", "5555FF", "5555AA", "5500AA", "AA00FF",
      "AA55FF", "AA55AA", "AA00AA", "FF00AA", "FF00FF",
      "FF55FF", "FF55AA", "FF0055", "FF5555", "AA5555",
      "AA5500", "FF0000", "FF5500", "FFAA00", "FFAA55",
      "FFFF55", "FFFF00", "AAAA00"
    };
    return colors[rand() % ARRAY_LENGTH(colors)];
  #endif
}

static int16_t get_current_version() {
  return __pbl_app_info.process_version.major*10 + __pbl_app_info.process_version.minor;
}

static void draw_background(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  
  graphics_context_set_fill_color(ctx, get_background_color());
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);
  
  int line_indent = 4;
  int line_pos_y = 141+LAYER_VERTICAL_INDENT+LAYER_VERTICAL_INDENT/4;
  #if defined(PBL_ROUND)
    GBitmap *fb = graphics_capture_frame_buffer(ctx);
  
    for (int y = line_pos_y-2; y <= line_pos_y-1; y++) {
      // Get the min and max x values for this row
      GBitmapDataRowInfo info = gbitmap_get_data_row_info(fb, y);
      // Iterate over visible pixels in that row
      for (int x = info.min_x+line_indent; x < info.max_x-line_indent; x++) {
        memset(&info.data[x], get_main_color().argb, 1);
      }
    }
  
    // Release framebuffer
    graphics_release_frame_buffer(ctx, fb);
  #else
    line_pos_y += 1;
    graphics_context_set_stroke_color(ctx, get_main_color());
    graphics_draw_line(ctx, GPoint(line_indent, line_pos_y), GPoint(bounds.size.w-line_indent, line_pos_y));
    graphics_draw_line(ctx, GPoint(line_indent, line_pos_y+1), GPoint(bounds.size.w-line_indent, line_pos_y+1));
  #endif
}

void update_background_color() {
  if (persist_exists(KEY_MMS_SCREEN_COLOR)) {
    persist_read_string(KEY_MMS_SCREEN_COLOR, s_config_screen_color, sizeof(s_config_screen_color));
  }
  else {
    snprintf(s_config_screen_color, sizeof(s_config_screen_color), "%s", get_random_color());
  }
}
  
static void update_weather_image(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  int horizontal_correction = 0;
  int vertical_correction = 0;
  #ifdef PBL_ROUND
  horizontal_correction = 16;
  #endif
  
  if (200 == bounds.size.w) {
    horizontal_correction = 7;
    vertical_correction = 8;
  }
  
  int weather_id = persist_read_int(KEY_MMS_CONDITIONS);
  
  if (weather_id == 1010) {
    // CLEAN_SKY_DAY;
    draw_big_sun(layer, ctx, GPoint(0+horizontal_correction, 0+vertical_correction), get_main_color(), get_main_color_inverted());
  }
  else if (weather_id == 1020) {
    // CLEAN_SKY_NIGHT;
    draw_big_moon(layer, ctx, GPoint(0+horizontal_correction, 0+vertical_correction), get_main_color(), get_main_color_inverted());
  }
  else if (weather_id == 1110) {
    // FEW_CLOUDS_DAY;
    #ifdef PBL_ROUND
    horizontal_correction = 13;
    #endif
    if (200 == bounds.size.w) {
      horizontal_correction = 34;
    }
    draw_small_sun(layer, ctx, GPoint(79+horizontal_correction, 31), get_main_color(), get_main_color_inverted());
    if (200 == bounds.size.w) {
      horizontal_correction = 7;
    }
    draw_full_cloud(layer, ctx, GPoint(2+horizontal_correction, 36), get_main_color(), get_main_color_inverted());
  }
  else if (weather_id == 1120) {
    // FEW_CLOUDS_NIGHT;
    if (200 == bounds.size.w) {
      horizontal_correction = 38;
      vertical_correction = -2;
    }
    draw_small_moon(layer, ctx, GPoint(100+horizontal_correction, 27+vertical_correction), get_main_color(), get_main_color_inverted());
    if (200 == bounds.size.w) {
      horizontal_correction = 4;
    }
    draw_full_cloud(layer, ctx, GPoint(2+horizontal_correction, 36), get_main_color(), get_main_color_inverted());
  }
  else if (weather_id >= 200 && weather_id <= 232) {
    // RAIN_THUNDERSTORM;
    int rain_vertical_correction = 0;
    int rain_horizontal_correction = 0;
    if (200 == bounds.size.w) {
      rain_vertical_correction = 24;
      rain_horizontal_correction = 37;
    }
    draw_full_cloud(layer, ctx, GPoint(14+horizontal_correction, 2+vertical_correction), get_main_color(), get_main_color_inverted());
    if (200 == bounds.size.w) {
      draw_rain_column(layer, ctx, GPoint(0+horizontal_correction+rain_horizontal_correction, 72+vertical_correction+rain_vertical_correction), get_main_color());
    }
    draw_rain_column(layer, ctx, GPoint(35+horizontal_correction+rain_horizontal_correction, 72+vertical_correction+rain_vertical_correction), get_main_color());
    draw_rain_column(layer, ctx, GPoint(70+horizontal_correction+rain_horizontal_correction, 72+vertical_correction+rain_vertical_correction), get_main_color());
    draw_lighting(layer, ctx, GPoint(101+horizontal_correction+rain_horizontal_correction, 72+vertical_correction+rain_vertical_correction), get_main_color());
  }
  else if ((weather_id >= 300 && weather_id <= 504) || (weather_id >= 520 && weather_id <= 531)) {
    // RAIN;
    int rain_vertical_correction = 0;
    int rain_horizontal_correction = 0;
    if (200 == bounds.size.w) {
      rain_vertical_correction = 24;
      rain_horizontal_correction = 37;
    }
    draw_full_cloud(layer, ctx, GPoint(14+horizontal_correction, 2+vertical_correction), get_main_color(), get_main_color_inverted());
    if (200 == bounds.size.w) {
      draw_rain_column(layer, ctx, GPoint(0+horizontal_correction+rain_horizontal_correction, 72+vertical_correction+rain_vertical_correction), get_main_color());
    }
    draw_rain_column(layer, ctx, GPoint(35+horizontal_correction+rain_horizontal_correction, 72+vertical_correction+rain_vertical_correction), get_main_color());
    draw_rain_column(layer, ctx, GPoint(70+horizontal_correction+rain_horizontal_correction, 72+vertical_correction+rain_vertical_correction), get_main_color());
    draw_rain_column(layer, ctx, GPoint(105+horizontal_correction+rain_horizontal_correction, 72+vertical_correction+rain_vertical_correction), get_main_color());
  }
  else if (weather_id == 511 || (weather_id >= 611 && weather_id <= 616)) {
    // RAIN_SNOW;
    int rain_vertical_correction = 0;
    int rain_horizontal_correction = 0;
    if (200 == bounds.size.w) {
      rain_vertical_correction = 24;
      rain_horizontal_correction = 5;
    }
    draw_full_cloud(layer, ctx, GPoint(14+horizontal_correction, 2+vertical_correction), get_main_color(), get_main_color_inverted());
    draw_rain_column(layer, ctx, GPoint(35+horizontal_correction+rain_horizontal_correction, 72+vertical_correction+rain_vertical_correction), get_main_color());
    draw_rain_column(layer, ctx, GPoint(70+horizontal_correction+rain_horizontal_correction, 72+vertical_correction+rain_vertical_correction), get_main_color());
    draw_snowflake(layer, ctx, GPoint(92+horizontal_correction+rain_horizontal_correction, 72+vertical_correction+rain_vertical_correction), get_main_color());
    if (200 == bounds.size.w) {
      draw_rain_column(layer, ctx, GPoint(135+horizontal_correction+rain_horizontal_correction, 72+vertical_correction+rain_vertical_correction), get_main_color());
    }
  }
  else if ((weather_id >= 600 && weather_id <= 602) || (weather_id >= 620 && weather_id <= 622)) {
    // SNOW;
    int rain_vertical_correction = 0;
    int rain_horizontal_correction = 0;
    if (200 == bounds.size.w) {
      rain_vertical_correction = 24;
      rain_horizontal_correction = 3;
    }
    draw_full_cloud(layer, ctx, GPoint(14+horizontal_correction, 2+vertical_correction), get_main_color(), get_main_color_inverted());
    draw_snow_column(layer, ctx, GPoint(35+horizontal_correction+rain_horizontal_correction, 72+vertical_correction+rain_vertical_correction), get_main_color());
    draw_snow_column(layer, ctx, GPoint(60+horizontal_correction+rain_horizontal_correction, 72+vertical_correction+rain_vertical_correction), get_main_color());
    draw_snowflake(layer, ctx, GPoint(87+horizontal_correction+rain_horizontal_correction, 72+vertical_correction+rain_vertical_correction), get_main_color());
    if (200 == bounds.size.w) {
      draw_snow_column(layer, ctx, GPoint(130+horizontal_correction+rain_horizontal_correction, 72+vertical_correction+rain_vertical_correction), get_main_color());
    }
  }
  else if (weather_id >= 701 && weather_id <= 762) {
    // MIST_NIGHT;
    if (200 == bounds.size.w) {
      vertical_correction = 10;
    }
    draw_half_cloud(layer, ctx, GPoint(14+horizontal_correction, 9+vertical_correction), get_main_color(), get_main_color_inverted());
    draw_mist(layer, ctx, GPoint(14+horizontal_correction, 67+vertical_correction), get_main_color());
  }
  else if (weather_id == 771 || weather_id == 905 || (weather_id >= 952 && weather_id <= 957)) {
    // WIND;
    if (200 == bounds.size.w) {
      vertical_correction = 18;
    }
    draw_wind(layer, ctx, GPoint(12+horizontal_correction, 7+vertical_correction), get_main_color());
  }
  else if (weather_id == 781 || (weather_id >= 900 && weather_id <= 902) || (weather_id >= 958 && weather_id <= 962)) {
    // TORNADO;
    if (200 == bounds.size.w) {
      horizontal_correction = 22;
    }
    draw_tornado(layer, ctx, GPoint(39+horizontal_correction, 7+vertical_correction), get_main_color());
  }
  else if ((weather_id >= 802 && weather_id <= 804) || weather_id == 903 || weather_id == 951) {
    // CLOUDS;
    draw_full_cloud(layer, ctx, GPoint(14+horizontal_correction, 20+vertical_correction), get_main_color(), get_main_color_inverted());
  }
  else if (weather_id == 906) {
    // THUNDERSTORM;
    int rain_vertical_correction = 0;
    int rain_horizontal_correction = 0;
    if (200 == bounds.size.w) {
      rain_vertical_correction = 24;
      rain_horizontal_correction = 22;
    }
    draw_full_cloud(layer, ctx, GPoint(14+horizontal_correction, 2+vertical_correction), get_main_color(), get_main_color_inverted());
    draw_lighting(layer, ctx, GPoint(66+horizontal_correction+rain_horizontal_correction, 72+vertical_correction+rain_vertical_correction), get_main_color());
  }
  else {
      APP_LOG(APP_LOG_LEVEL_ERROR, "ERROR: Unrecognised weather id \"%d\"", weather_id);
  }
}

static void persist_read_and_set() {
  if (persist_exists(KEY_MMS_TEMPERATURE)) {
    persist_read_string(KEY_MMS_TEMPERATURE, s_temperature_buffer, sizeof(s_temperature_buffer));
  }
  if (persist_exists(KEY_MMS_BLUETOOTH)) {
    persist_read_string(KEY_MMS_BLUETOOTH, s_config_bluetooth, sizeof(s_config_bluetooth));
  }
  if (persist_exists(KEY_MMS_BATTERY)) {
    persist_read_string(KEY_MMS_BATTERY, s_config_battery, sizeof(s_config_battery));
  }
  if (persist_exists(KEY_MMS_SCREEN_COLOR)) {
    persist_read_string(KEY_MMS_SCREEN_COLOR, s_config_screen_color, sizeof(s_config_screen_color));
  }
  else {
    snprintf(s_config_screen_color, sizeof(s_config_screen_color), "%s", get_random_color());
  }
  if (persist_exists(KEY_MMS_HOURLY_BEEP)) {
    persist_read_string(KEY_MMS_HOURLY_BEEP, s_config_hourly_beep, sizeof(s_config_hourly_beep));
  }
  if (persist_exists(KEY_MMS_DND)) {
    persist_read_string(KEY_MMS_DND, s_config_dnd, sizeof(s_config_dnd));
  }
  if (persist_exists(KEY_MMS_DND_START)) {
    s_config_dnd_start = persist_read_int(KEY_MMS_DND_START);
  }
  if (persist_exists(KEY_MMS_DND_END)) {
    s_config_dnd_end = persist_read_int(KEY_MMS_DND_END);
  }
  if (persist_exists(KEY_MMS_HUMIDITY)) {
    s_humidity_buffer = persist_read_int(KEY_MMS_HUMIDITY);
  }
  hour_lead_zero = persist_exists(KEY_MMS_HOUR_LEAD_ZERO) ? persist_read_bool(KEY_MMS_HOUR_LEAD_ZERO) : true;
  s_config_show_humidity = persist_exists(KEY_MMS_SHOW_HUMIDITY) ? persist_read_bool(KEY_MMS_SHOW_HUMIDITY) : false;
}

static void make_beep() {
  // Vibe pattern
  static const uint32_t segments[] = {100,100,200};
  VibePattern pat = {
    .durations = segments,
    .num_segments = ARRAY_LENGTH(segments),
  };
  vibes_enqueue_custom_pattern(pat);
}

static bool is_silence(int32_t hour) {
  if (strcmp(s_config_dnd, "dnd_on") == 0) {
    if (s_config_dnd_start>s_config_dnd_end) {
      if (hour>=s_config_dnd_start || hour<=s_config_dnd_end) {
        return true;
      }
    }
    else if (s_config_dnd_start<s_config_dnd_end) {
      if (hour>=s_config_dnd_start && hour<=s_config_dnd_end) {
        return true;
      }
    }
  }

  return false;
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
  weather_out_of_date = true;
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
  weather_out_of_date = true;
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
  weather_out_of_date = false;
  
  update_background_color();
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
  
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  // Subscribe to Bluetooth updates
  bluetooth_connection_service_subscribe(bluetooth_handler);
  
  // Register battery handler
  battery_state_service_subscribe(battery_handler);
  
  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  
  // Open AppMessage
  app_message_open(APP_MESSAGE_INBOX_SIZE_MINIMUM, APP_MESSAGE_OUTBOX_SIZE_MINIMUM);
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