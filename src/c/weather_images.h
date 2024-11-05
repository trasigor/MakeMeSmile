#pragma once

#include <pebble.h>

void draw_big_sun(Layer *layer, GContext *ctx, GPoint move_to, GColor color_main, GColor color_main_inverted);
void draw_big_moon(Layer *layer, GContext *ctx, GPoint move_to, GColor color_main, GColor color_main_inverted);
void draw_full_cloud(Layer *layer, GContext *ctx, GPoint move_to, GColor color_main, GColor color_main_inverted);
void draw_small_sun(Layer *layer, GContext *ctx, GPoint move_to, GColor color_main, GColor color_main_inverted);
void draw_small_moon(Layer *layer, GContext *ctx, GPoint move_to, GColor color_main, GColor color_main_inverted);
void draw_rain_column(Layer *layer, GContext *ctx, GPoint move_to, GColor color_main);
void draw_lighting(Layer *layer, GContext *ctx, GPoint move_to, GColor color_main);
void draw_snowflake(Layer *layer, GContext *ctx, GPoint move_to, GColor color_main);
void draw_snow_column(Layer *layer, GContext *ctx, GPoint move_to, GColor color_main);
void draw_half_cloud(Layer *layer, GContext *ctx, GPoint move_to, GColor color_main, GColor color_main_inverted);
void draw_mist(Layer *layer, GContext *ctx, GPoint move_to, GColor color_main);
void draw_wind(Layer *layer, GContext *ctx, GPoint move_to, GColor color_main);
void draw_tornado(Layer *layer, GContext *ctx, GPoint move_to, GColor color_main);
