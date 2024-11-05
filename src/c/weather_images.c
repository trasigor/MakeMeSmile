#include <pebble.h>
  
#include "weather_images.h"

static const GPoint px_shift[] = {
  {0, 0},
  {1, 0},
  {-1, 0},
  {0, 1},
  {0, -1},
  {1, 1},
  {-1, 1},
  {1, -1},
  {-1, -1},
};

static const GPathInfo BIG_OCTAGON_PATH_INFO = {
  .num_points = 8,
  
  #if PBL_DISPLAY_WIDTH == 200
  // 36 - side size - 36x0.7=25
  .points = (GPoint []) {{25, 0}, {61, 0}, {86, 25}, {86, 61}, {61, 86}, {25, 86}, {0, 61}, {0, 25}}
  #else
  // 27 - side size - 27x0.7=19
  .points = (GPoint []) {{19, 0}, {46, 0}, {65, 19}, {65, 46}, {46, 65}, {19, 65}, {0, 46}, {0, 19}}
  #endif
};
static const GPathInfo GLASSES_PATH_INFO = {
  .num_points = 12,
  
  #if PBL_DISPLAY_WIDTH == 200
  .points = (GPoint []) {
    {13, 28},
    {16, 43}, {17, 44},
    {37, 44}, {38, 43},
    {42, 30}, {43, 30},
    {47, 43}, {48, 44},
    {68, 44}, {69, 43},
    {72, 28},
  }
  #else
  .points = (GPoint []) {
    {10, 22},
    {12, 33}, {13, 34},
    {28, 34}, {29, 33},
    {32, 24}, {33, 24},
    {36, 33}, {37, 34},
    {52, 34}, {53, 33},
    {55, 22},
  }
  #endif
};
void draw_big_sun(Layer *layer, GContext *ctx, GPoint move_to, GColor color_main, GColor color_main_inverted) {
  #if PBL_DISPLAY_WIDTH == 200
  GPoint move_to_center = GPoint(50, 20);
  #else
  GPoint move_to_center = GPoint(40, 18);
  #endif
  int correction_X = move_to.x+move_to_center.x;
  int correction_Y = move_to.y+move_to_center.y;
  static GPath *s_path = NULL;
  s_path = gpath_create(&BIG_OCTAGON_PATH_INFO);
  gpath_move_to(s_path, GPoint(correction_X, correction_Y));
  
  // Fill the path:
  graphics_context_set_fill_color(ctx, color_main_inverted);
  gpath_draw_filled(ctx, s_path);
  
  // Stroke the path:
  graphics_context_set_stroke_color(ctx, color_main);
  int max_points = ARRAY_LENGTH(px_shift);
  for (int i=0; i<max_points; i++) {
    correction_X = move_to.x+move_to_center.x+px_shift[i].x;
    correction_Y = move_to.y+move_to_center.y+px_shift[i].y;
    gpath_move_to(s_path, GPoint(correction_X, correction_Y));
    gpath_draw_outline(ctx, s_path);

    #if PBL_DISPLAY_WIDTH == 200
    // 7 & 14 - points diff
    // 3 & 6 - indent
    graphics_draw_line(ctx, GPoint(22+correction_X, -6+correction_Y), GPoint(15+correction_X, -20+correction_Y));
    graphics_draw_line(ctx, GPoint(64+correction_X, -6+correction_Y), GPoint(71+correction_X, -20+correction_Y));
    graphics_draw_line(ctx, GPoint(92+correction_X, 22+correction_Y), GPoint(106+correction_X, 15+correction_Y));
    graphics_draw_line(ctx, GPoint(92+correction_X, 64+correction_Y), GPoint(106+correction_X, 71+correction_Y));
    graphics_draw_line(ctx, GPoint(22+correction_X, 92+correction_Y), GPoint(15+correction_X, 106+correction_Y));
    graphics_draw_line(ctx, GPoint(64+correction_X, 92+correction_Y), GPoint(71+correction_X, 106+correction_Y));
    graphics_draw_line(ctx, GPoint(-6+correction_X, 22+correction_Y), GPoint(-20+correction_X, 15+correction_Y));
    graphics_draw_line(ctx, GPoint(-6+correction_X, 64+correction_Y), GPoint(-20+correction_X, 71+correction_Y));

    //mouth
    graphics_draw_line(ctx, GPoint(31+correction_X, 65+correction_Y), GPoint(49+correction_X, 65+correction_Y));
    graphics_draw_line(ctx, GPoint(49+correction_X, 65+correction_Y), GPoint(59+correction_X, 58+correction_Y));
    #else
    // 5 & 10 - points diff
    // 3 & 6 - indent
    graphics_draw_line(ctx, GPoint(16+correction_X, -6+correction_Y), GPoint(11+correction_X, -16+correction_Y));
    graphics_draw_line(ctx, GPoint(49+correction_X, -6+correction_Y), GPoint(54+correction_X, -16+correction_Y));
    graphics_draw_line(ctx, GPoint(71+correction_X, 16+correction_Y), GPoint(81+correction_X, 11+correction_Y));
    graphics_draw_line(ctx, GPoint(71+correction_X, 49+correction_Y), GPoint(81+correction_X, 54+correction_Y));
    graphics_draw_line(ctx, GPoint(16+correction_X, 71+correction_Y), GPoint(11+correction_X, 81+correction_Y));
    graphics_draw_line(ctx, GPoint(49+correction_X, 71+correction_Y), GPoint(54+correction_X, 81+correction_Y));
    graphics_draw_line(ctx, GPoint(-6+correction_X, 16+correction_Y), GPoint(-16+correction_X, 11+correction_Y));
    graphics_draw_line(ctx, GPoint(-6+correction_X, 49+correction_Y), GPoint(-16+correction_X, 54+correction_Y));

    //mouth
    graphics_draw_line(ctx, GPoint(24+correction_X, 49+correction_Y), GPoint(37+correction_X, 49+correction_Y));
    graphics_draw_line(ctx, GPoint(37+correction_X, 49+correction_Y), GPoint(44+correction_X, 44+correction_Y));
    #endif
  }
  
  correction_X = move_to.x+move_to_center.x;
  correction_Y = move_to.y+move_to_center.y;
  
  //mouth
  #if PBL_DISPLAY_WIDTH == 200
  graphics_draw_line(ctx, GPoint(31-2+correction_X, 65+correction_Y), GPoint(49+correction_X, 65+correction_Y));
  #else
  graphics_draw_line(ctx, GPoint(24-2+correction_X, 49+correction_Y), GPoint(37+correction_X, 49+correction_Y));
  #endif
  
  s_path = gpath_create(&GLASSES_PATH_INFO);
  gpath_move_to(s_path, GPoint(correction_X, correction_Y));
  // Fill the path:
  graphics_context_set_fill_color(ctx, color_main);
  gpath_draw_filled(ctx, s_path);
  
  gpath_destroy(s_path);
}

static const GPathInfo BIG_MOON_PATH_INFO = {
  .num_points = 8,
  
  #if PBL_DISPLAY_WIDTH == 200
    // 36 - side size - 36x0.7=25
    .points = (GPoint []) {{25, 0},
    {25, 40}, {46, 61},
    {86, 61}, {61, 86}, {25, 86}, {0, 61}, {0, 25}}
  #else
    // 27 - side size - 27x0.7=19
    .points = (GPoint []) {{19, 0},
    {19, 30}, {35, 46},
    {65, 46}, {46, 65}, {19, 65}, {0, 46}, {0, 19}}
  #endif
};
void draw_big_moon(Layer *layer, GContext *ctx, GPoint move_to, GColor color_main, GColor color_main_inverted) {
  #if PBL_DISPLAY_WIDTH == 200
  GPoint move_to_center = GPoint(50, 20);
  #else
  GPoint move_to_center = GPoint(40, 18);
  #endif
  int correction_X = move_to.x+move_to_center.x;
  int correction_Y = move_to.y+move_to_center.y;
  static GPath *s_path = NULL;
  s_path = gpath_create(&BIG_MOON_PATH_INFO);
  gpath_move_to(s_path, GPoint(correction_X, correction_Y));
  
  // Fill the path:
  graphics_context_set_fill_color(ctx, color_main_inverted);
  gpath_draw_filled(ctx, s_path);
  
  // Stroke the path:
  graphics_context_set_stroke_color(ctx, color_main);
  int max_points = ARRAY_LENGTH(px_shift);
  for (int i=0; i<max_points; i++) {
    correction_X = move_to.x+move_to_center.x+px_shift[i].x;
    correction_Y = move_to.y+move_to_center.y+px_shift[i].y;
    gpath_move_to(s_path, GPoint(correction_X, correction_Y));
    gpath_draw_outline(ctx, s_path);

    #if PBL_DISPLAY_WIDTH == 200
      //eye
      graphics_draw_line(ctx, GPoint(14+correction_X, 38+correction_Y), GPoint(14+correction_X, 47+correction_Y));
  
      //mouth
      graphics_draw_line(ctx, GPoint(46+correction_X, 61+correction_Y), GPoint(39+correction_X, 68+correction_Y));
      graphics_draw_line(ctx, GPoint(39+correction_X, 68+correction_Y), GPoint(25+correction_X, 68+correction_Y));
    #else
      //eye
      graphics_draw_line(ctx, GPoint(10+correction_X, 28+correction_Y), GPoint(10+correction_X, 36+correction_Y));
  
      //mouth
      graphics_draw_line(ctx, GPoint(35+correction_X, 46+correction_Y), GPoint(29+correction_X, 52+correction_Y));
      graphics_draw_line(ctx, GPoint(29+correction_X, 52+correction_Y), GPoint(19+correction_X, 52+correction_Y));
    #endif
  }
  
  correction_X = move_to.x+move_to_center.x;
  correction_Y = move_to.y+move_to_center.y;
  
  #if PBL_DISPLAY_WIDTH == 200
    //eye
    graphics_draw_line(ctx, GPoint(14+correction_X, 38-2+correction_Y), GPoint(14+correction_X, 47+2+correction_Y));
  
    //mouth
    graphics_draw_line(ctx, GPoint(39+correction_X, 68+correction_Y), GPoint(25-2+correction_X, 68+correction_Y));
  #else
    //eye
    graphics_draw_line(ctx, GPoint(10+correction_X, 28-2+correction_Y), GPoint(10+correction_X, 36+2+correction_Y));
  
    //mouth
    graphics_draw_line(ctx, GPoint(29+correction_X, 52+correction_Y), GPoint(19-2+correction_X, 52+correction_Y));
  #endif
  
  gpath_destroy(s_path);
}

static const GPathInfo FULL_CLOUD_PATH_INFO = {
  .num_points = 14,
  
  #if PBL_DISPLAY_WIDTH == 200
  // 27 - side size - 27x0.7=19
  .points = (GPoint []) {
    {19, 84}, {0, 65}, {0, 38}, {19, 19}, {46, 19}, {65, 38}, {46, 19},
    {65, 0}, {92, 0}, {111, 19}, {138, 19}, {157, 38}, {157, 65}, {138, 84}
  }
  #else
  // 20 - side size - 20x0.7=14
  .points = (GPoint []) {
    {14, 62}, {0, 48}, {0, 28}, {14, 14}, {34, 14}, {48, 28}, {34, 14},
    {48, 0}, {68, 0}, {82, 14}, {102, 14}, {116, 28}, {116, 48}, {102, 62}
  }
  #endif
};
void draw_full_cloud(Layer *layer, GContext *ctx, GPoint move_to, GColor color_main, GColor color_main_inverted) {
  int correction_X = 0;
  int correction_Y = 0;
  static GPath *s_path = NULL;
  s_path = gpath_create(&FULL_CLOUD_PATH_INFO);
  gpath_move_to(s_path, move_to);
  
  // Fill the path:
  graphics_context_set_fill_color(ctx, color_main_inverted);
  gpath_draw_filled(ctx, s_path);
  
  // Stroke the path:
  graphics_context_set_stroke_color(ctx, color_main);
  int max_points = ARRAY_LENGTH(px_shift);
  for (int i=0; i<max_points; i++) {
    correction_X = move_to.x+px_shift[i].x;
    correction_Y = move_to.y+px_shift[i].y;
    gpath_move_to(s_path, GPoint(correction_X, correction_Y));
    gpath_draw_outline(ctx, s_path);

    #if PBL_DISPLAY_WIDTH == 200
      //eyes
      graphics_draw_line(ctx, GPoint(95+correction_X, 36+correction_Y), GPoint(95+correction_X, 47+correction_Y));
      graphics_draw_line(ctx, GPoint(121+correction_X, 36+correction_Y), GPoint(121+correction_X, 47+correction_Y));
  
      //mouth
      graphics_draw_line(ctx, GPoint(120+correction_X, 66+correction_Y), GPoint(103+correction_X, 66+correction_Y));
      graphics_draw_line(ctx, GPoint(103+correction_X, 66+correction_Y), GPoint(96+correction_X, 60+correction_Y));
    #else
      //eyes
      graphics_draw_line(ctx, GPoint(70+correction_X, 25+correction_Y), GPoint(70+correction_X, 33+correction_Y));
      graphics_draw_line(ctx, GPoint(90+correction_X, 25+correction_Y), GPoint(90+correction_X, 33+correction_Y));
  
      //mouth
      graphics_draw_line(ctx, GPoint(89+correction_X, 49+correction_Y), GPoint(77+correction_X, 49+correction_Y));
      graphics_draw_line(ctx, GPoint(77+correction_X, 49+correction_Y), GPoint(71+correction_X, 43+correction_Y));
    #endif
  }
  
  #if PBL_DISPLAY_WIDTH == 200
    //eyes
    graphics_draw_line(ctx, GPoint(95+move_to.x, 36+move_to.y-2), GPoint(95+move_to.x, 47+move_to.y+2));
    graphics_draw_line(ctx, GPoint(121+move_to.x, 36+move_to.y-2), GPoint(121+move_to.x, 47+move_to.y+2));
  
    //mouth
    graphics_draw_line(ctx, GPoint(120+move_to.x+2, 66+move_to.y), GPoint(103+move_to.x-2, 66+move_to.y));
  #else
    //eyes
    graphics_draw_line(ctx, GPoint(70+move_to.x, 25+move_to.y-2), GPoint(70+move_to.x, 33+move_to.y+2));
    graphics_draw_line(ctx, GPoint(90+move_to.x, 25+move_to.y-2), GPoint(90+move_to.x, 33+move_to.y+2));
  
    //mouth
    graphics_draw_line(ctx, GPoint(89+move_to.x+2, 49+move_to.y), GPoint(77+move_to.x-2, 49+move_to.y));
  #endif
  
  gpath_destroy(s_path);
}

static const GPathInfo SMALL_OCTAGON_PATH_INFO = {
  .num_points = 8,
  
  #if PBL_DISPLAY_WIDTH == 200
  // 27 - side size - 27x0.7=19
  .points = (GPoint []) {{19, 0}, {46, 0}, {65, 19}, {65, 46}, {46, 65}, {19, 65}, {0, 46}, {0, 19}}
  #else
  // 20 - side size - 20x0.7=14
  .points = (GPoint []) {{14, 0}, {34, 0}, {48, 14}, {48, 34}, {34, 48}, {14, 48}, {0, 34}, {0, 14}}
  #endif
};
void draw_small_sun(Layer *layer, GContext *ctx, GPoint move_to, GColor color_main, GColor color_main_inverted) {
  static GPath *s_path = NULL;
  s_path = gpath_create(&SMALL_OCTAGON_PATH_INFO);
  gpath_move_to(s_path, move_to);
  
  // Fill the path:
  graphics_context_set_fill_color(ctx, color_main_inverted);
  gpath_draw_filled(ctx, s_path);
  
  int correction_X = 0;
  int correction_Y = 0;
  
  // Stroke the path:
  graphics_context_set_stroke_color(ctx, color_main);
  int max_points = ARRAY_LENGTH(px_shift);
  for (int i=0; i<max_points; i++) {
    correction_X = move_to.x+px_shift[i].x;
    correction_Y = move_to.y+px_shift[i].y;
    gpath_move_to(s_path, GPoint(correction_X, correction_Y));
    gpath_draw_outline(ctx, s_path);

    #if PBL_DISPLAY_WIDTH == 200
    // 4 & 8 - points diff
    // 3 & 6 - indent
    graphics_draw_line(ctx, GPoint(16+correction_X, -6+correction_Y), GPoint(12+correction_X, -12+correction_Y));
    graphics_draw_line(ctx, GPoint(49+correction_X, -6+correction_Y), GPoint(53+correction_X, -12+correction_Y));
    graphics_draw_line(ctx, GPoint(71+correction_X, 16+correction_Y), GPoint(79+correction_X, 12+correction_Y));
    graphics_draw_line(ctx, GPoint(71+correction_X, 49+correction_Y), GPoint(79+correction_X, 53+correction_Y));
    // others not visible
    #else
    // 3 & 6 - points diff
    // 3 & 6 - indent
    graphics_draw_line(ctx, GPoint(11+correction_X, -6+correction_Y), GPoint(8+correction_X, -12+correction_Y));
    graphics_draw_line(ctx, GPoint(37+correction_X, -6+correction_Y), GPoint(40+correction_X, -12+correction_Y));
    graphics_draw_line(ctx, GPoint(54+correction_X, 11+correction_Y), GPoint(60+correction_X, 8+correction_Y));
    graphics_draw_line(ctx, GPoint(54+correction_X, 37+correction_Y), GPoint(60+correction_X, 40+correction_Y));
    // others not visible
    #endif
  }
  
  gpath_destroy(s_path);
}

static const GPathInfo SMALL_MOON_PATH_INFO = {
  .num_points = 8,
  
  #if PBL_DISPLAY_WIDTH == 200
  // 23 - side size - 23x0.7=16
  .points = (GPoint []) {{16, 0}, {16, 23}, {32, 39}, {55, 39}, {39, 55}, {16, 55}, {0, 39}, {0, 16}}
  #else
  // 17 - side size - 17x0.7=12
  .points = (GPoint []) {{12, 0}, {12, 17}, {24, 29}, {41, 29}, {29, 41}, {12, 41}, {0, 29}, {0, 12}}
  #endif
};
void draw_small_moon(Layer *layer, GContext *ctx, GPoint move_to, GColor color_main, GColor color_main_inverted) {
  static GPath *s_path = NULL;
  s_path = gpath_create(&SMALL_MOON_PATH_INFO);
  gpath_move_to(s_path, move_to);
  
  // Fill the path:
  graphics_context_set_fill_color(ctx, color_main_inverted);
  gpath_draw_filled(ctx, s_path);
  
  // Stroke the path:
  graphics_context_set_stroke_color(ctx, color_main);
  int max_points = ARRAY_LENGTH(px_shift);
  for (int i=0; i<max_points; i++) {
    gpath_move_to(s_path, GPoint(move_to.x+px_shift[i].x, move_to.y+px_shift[i].y));
    gpath_draw_outline(ctx, s_path);
  }
  
  gpath_destroy(s_path);
}

void draw_rain_column(Layer *layer, GContext *ctx, GPoint move_to, GColor color_main) {
  int correction_X = 0;
  int correction_Y = 0;
  graphics_context_set_stroke_color(ctx, color_main);
  int max_points = ARRAY_LENGTH(px_shift);
  for (int i=0; i<max_points; i++) {
    correction_X = move_to.x+px_shift[i].x;
    correction_Y = move_to.y+px_shift[i].y;
    graphics_draw_line(ctx, GPoint(4+correction_X, 0+correction_Y), GPoint(0+correction_X, 8+correction_Y));
    graphics_draw_line(ctx, GPoint(4+correction_X, 18+correction_Y), GPoint(0+correction_X, 26+correction_Y));
  }
}

void draw_lighting(Layer *layer, GContext *ctx, GPoint move_to, GColor color_main) {
  int correction_X = 0;
  int correction_Y = 0;
  graphics_context_set_stroke_color(ctx, color_main);
  int max_points = ARRAY_LENGTH(px_shift);
  for (int i=0; i<max_points; i++) {
    correction_X = move_to.x+px_shift[i].x;
    correction_Y = move_to.y+px_shift[i].y;
    graphics_draw_line(ctx, GPoint(8+correction_X, 0+correction_Y), GPoint(0+correction_X, 13+correction_Y));
    graphics_draw_line(ctx, GPoint(0+correction_X, 13+correction_Y), GPoint(8+correction_X, 13+correction_Y));
    graphics_draw_line(ctx, GPoint(8+correction_X, 13+correction_Y), GPoint(0+correction_X, 26+correction_Y));
  }
}

void draw_snowflake(Layer *layer, GContext *ctx, GPoint move_to, GColor color_main) {
  int correction_X = 0;
  int correction_Y = 0;
  graphics_context_set_stroke_color(ctx, color_main);
  int max_points = ARRAY_LENGTH(px_shift);
  for (int i=0; i<max_points; i++) {
    correction_X = move_to.x+px_shift[i].x;
    correction_Y = move_to.y+px_shift[i].y;
    
    // big radius 11
    // small radius 4
    graphics_draw_line(ctx, GPoint(13+correction_X, 3+correction_Y), GPoint(13+correction_X, 25+correction_Y));

    graphics_draw_line(ctx, GPoint(4+correction_X, 9+correction_Y), GPoint(22+correction_X, 19+correction_Y));

    graphics_draw_line(ctx, GPoint(4+correction_X, 19+correction_Y), GPoint(22+correction_X, 9+correction_Y));

    graphics_draw_line(ctx, GPoint(13+correction_X, 3+correction_Y), GPoint(10+correction_X, 0+correction_Y));
    graphics_draw_line(ctx, GPoint(13+correction_X, 3+correction_Y), GPoint(16+correction_X, 0+correction_Y));
    graphics_draw_line(ctx, GPoint(13+correction_X, 25+correction_Y), GPoint(10+correction_X, 27+correction_Y));
    graphics_draw_line(ctx, GPoint(13+correction_X, 25+correction_Y), GPoint(16+correction_X, 27+correction_Y));

    graphics_draw_line(ctx, GPoint(4+correction_X, 9+correction_Y), GPoint(3+correction_X, 5+correction_Y));
    graphics_draw_line(ctx, GPoint(4+correction_X, 9+correction_Y), GPoint(0+correction_X, 10+correction_Y));
    graphics_draw_line(ctx, GPoint(22+correction_X, 19+correction_Y), GPoint(27+correction_X, 18+correction_Y));
    graphics_draw_line(ctx, GPoint(22+correction_X, 19+correction_Y), GPoint(23+correction_X, 23+correction_Y));

    graphics_draw_line(ctx, GPoint(22+correction_X, 9+correction_Y), GPoint(26+correction_X, 10+correction_Y));
    graphics_draw_line(ctx, GPoint(22+correction_X, 9+correction_Y), GPoint(23+correction_X, 5+correction_Y));
    graphics_draw_line(ctx, GPoint(4+correction_X, 19+correction_Y), GPoint(0+correction_X, 18+correction_Y));
    graphics_draw_line(ctx, GPoint(4+correction_X, 19+correction_Y), GPoint(3+correction_X, 23+correction_Y));
  }
}

void draw_snow_column(Layer *layer, GContext *ctx, GPoint move_to, GColor color_main) {
  graphics_context_set_fill_color(ctx, color_main);
  graphics_fill_circle(ctx, GPoint(12+move_to.x, 2+move_to.y), 2);
  graphics_fill_circle(ctx, GPoint(0+move_to.x, 14+move_to.y), 2);
  graphics_fill_circle(ctx, GPoint(12+move_to.x, 26+move_to.y), 2);
}

static const GPathInfo HALF_CLOUD_PATH_INFO = {
  .num_points = 12,
  
  #if PBL_DISPLAY_WIDTH == 200
  // 27 - side size - 27x0.7=19
  .points = (GPoint []) {
    {0, 65}, {0, 38}, {19, 19}, {46, 19}, {59, 32}, {46, 19},
    {65, 0}, {92, 0}, {111, 19}, {138, 19}, {157, 38}, {157, 65}
  }
  #else
  // 20 - side size - 20x0.7=14
  .points = (GPoint []) {
    {0, 48}, {0, 28}, {14, 14}, {34, 14}, {44, 24}, {34, 14},
    {48, 0}, {68, 0}, {82, 14}, {102, 14}, {116, 28}, {116, 48}
  }
  #endif
};
void draw_half_cloud(Layer *layer, GContext *ctx, GPoint move_to, GColor color_main, GColor color_main_inverted) {
  int correction_X = 0;
  int correction_Y = 0;
  static GPath *s_path = NULL;
  s_path = gpath_create(&HALF_CLOUD_PATH_INFO);
  gpath_move_to(s_path, move_to);
  
  // Fill the path:
  graphics_context_set_fill_color(ctx, color_main_inverted);
  gpath_draw_filled(ctx, s_path);
  
  // Stroke the path:
  graphics_context_set_stroke_color(ctx, color_main);
  int max_points = ARRAY_LENGTH(px_shift);
  for (int i=0; i<max_points; i++) {
    correction_X = move_to.x+px_shift[i].x;
    correction_Y = move_to.y+px_shift[i].y;
    gpath_move_to(s_path, GPoint(correction_X, correction_Y));
    gpath_draw_outline(ctx, s_path);

    #if PBL_DISPLAY_WIDTH == 200
      //eyes
      graphics_draw_line(ctx, GPoint(70+correction_X, 17+correction_Y), GPoint(70+correction_X, 24+correction_Y));
      graphics_draw_line(ctx, GPoint(90+correction_X, 17+correction_Y), GPoint(90+correction_X, 24+correction_Y));
  
      //mouth
      graphics_draw_line(ctx, GPoint(72+correction_X, 43+correction_Y), GPoint(85+correction_X, 43+correction_Y));
      graphics_draw_line(ctx, GPoint(85+correction_X, 43+correction_Y), GPoint(89+correction_X, 39+correction_Y));
    #else
      //eyes
      graphics_draw_line(ctx, GPoint(52+correction_X, 12+correction_Y), GPoint(52+correction_X, 18+correction_Y));
      graphics_draw_line(ctx, GPoint(68+correction_X, 12+correction_Y), GPoint(68+correction_X, 18+correction_Y));
  
      //mouth
      graphics_draw_line(ctx, GPoint(53+correction_X, 32+correction_Y), GPoint(63+correction_X, 32+correction_Y));
      graphics_draw_line(ctx, GPoint(63+correction_X, 32+correction_Y), GPoint(67+correction_X, 28+correction_Y));
    #endif
  }
  
  #if PBL_DISPLAY_WIDTH == 200
    //eyes
    graphics_draw_line(ctx, GPoint(70+move_to.x, 17+move_to.y-2), GPoint(70+move_to.x, 24+move_to.y+2));
    graphics_draw_line(ctx, GPoint(90+move_to.x, 17+move_to.y-2), GPoint(90+move_to.x, 24+move_to.y+2));
  
    //mouth
    graphics_draw_line(ctx, GPoint(72+move_to.x-2, 43+move_to.y), GPoint(85+move_to.x+2, 43+move_to.y));
  #else
    //eyes
    graphics_draw_line(ctx, GPoint(52+move_to.x, 12+move_to.y-2), GPoint(52+move_to.x, 18+move_to.y+2));
    graphics_draw_line(ctx, GPoint(68+move_to.x, 12+move_to.y-2), GPoint(68+move_to.x, 18+move_to.y+2));
  
    //mouth
    graphics_draw_line(ctx, GPoint(53+move_to.x-2, 32+move_to.y), GPoint(63+move_to.x+2, 32+move_to.y));
  #endif
  
  gpath_destroy(s_path);
}

void draw_mist(Layer *layer, GContext *ctx, GPoint move_to, GColor color_main) {
  int correction_X = 0;
  int correction_Y = 0;
  graphics_context_set_stroke_color(ctx, color_main);
  int max_points = ARRAY_LENGTH(px_shift);
  for (int i=0; i<max_points; i++) {
    correction_X = move_to.x+px_shift[i].x;
    correction_Y = move_to.y+px_shift[i].y;
    #if PBL_DISPLAY_WIDTH == 200
      graphics_draw_line(ctx, GPoint(12+correction_X, 20+correction_Y), GPoint(145+correction_X, 20+correction_Y));
      graphics_draw_line(ctx, GPoint(4+correction_X, 32+correction_Y), GPoint(153+correction_X, 32+correction_Y));
      graphics_draw_line(ctx, GPoint(22+correction_X, 44+correction_Y), GPoint(135+correction_X, 44+correction_Y));
    #else
      graphics_draw_line(ctx, GPoint(12+correction_X, 2+correction_Y), GPoint(104+correction_X, 2+correction_Y));
      graphics_draw_line(ctx, GPoint(4+correction_X, 14+correction_Y), GPoint(112+correction_X, 14+correction_Y));
      graphics_draw_line(ctx, GPoint(22+correction_X, 26+correction_Y), GPoint(94+correction_X, 26+correction_Y));
    #endif
  }
  
  #if PBL_DISPLAY_WIDTH == 200
    graphics_draw_line(ctx, GPoint(12+move_to.x-2, 20+move_to.y), GPoint(145+move_to.x+2, 20+move_to.y));
    graphics_draw_line(ctx, GPoint(4+move_to.x-2, 32+move_to.y), GPoint(153+move_to.x+2, 32+move_to.y));
    graphics_draw_line(ctx, GPoint(22+move_to.x-2, 44+move_to.y), GPoint(135+move_to.x+2, 44+move_to.y));
  #else
    graphics_draw_line(ctx, GPoint(12+move_to.x-2, 2+move_to.y), GPoint(104+move_to.x+2, 2+move_to.y));
    graphics_draw_line(ctx, GPoint(4+move_to.x-2, 14+move_to.y), GPoint(112+move_to.x+2, 14+move_to.y));
    graphics_draw_line(ctx, GPoint(22+move_to.x-2, 26+move_to.y), GPoint(94+move_to.x+2, 26+move_to.y));
  #endif
}

void draw_wind(Layer *layer, GContext *ctx, GPoint move_to, GColor color_main) {
  int correction_X = 0;
  int correction_Y = 0;
  graphics_context_set_stroke_color(ctx, color_main);
  int max_points = ARRAY_LENGTH(px_shift);
  for (int i=0; i<max_points; i++) {
    correction_X = move_to.x+px_shift[i].x;
    correction_Y = move_to.y+px_shift[i].y;
    #if PBL_DISPLAY_WIDTH == 200
      graphics_draw_line(ctx, GPoint(2+correction_X, 16+correction_Y), GPoint(124+correction_X, 16+correction_Y));
      graphics_draw_line(ctx, GPoint(124+correction_X, 16+correction_Y), GPoint(134+correction_X, 6+correction_Y));
      graphics_draw_line(ctx, GPoint(134+correction_X, 6+correction_Y), GPoint(124+correction_X, -4+correction_Y));
      graphics_draw_line(ctx, GPoint(124+correction_X, -4+correction_Y), GPoint(114+correction_X, 6+correction_Y));
  
      graphics_draw_line(ctx, GPoint(18+correction_X, 28+correction_Y), GPoint(102+correction_X, 28+correction_Y));
      graphics_draw_line(ctx, GPoint(102+correction_X, 28+correction_Y), GPoint(122+correction_X, 38+correction_Y));
  
      graphics_draw_line(ctx, GPoint(0+correction_X, 40+correction_Y), GPoint(84+correction_X, 40+correction_Y));
      graphics_draw_line(ctx, GPoint(84+correction_X, 40+correction_Y), GPoint(104+correction_X, 50+correction_Y));
      graphics_draw_line(ctx, GPoint(104+correction_X, 50+correction_Y), GPoint(140+correction_X, 50+correction_Y));
      graphics_draw_line(ctx, GPoint(140+correction_X, 50+correction_Y), GPoint(160+correction_X, 40+correction_Y));
  
      graphics_draw_line(ctx, GPoint(2+correction_X, 60+correction_Y), GPoint(67+correction_X, 60+correction_Y));
      graphics_draw_line(ctx, GPoint(67+correction_X, 60+correction_Y), GPoint(87+correction_X, 70+correction_Y));
      graphics_draw_line(ctx, GPoint(87+correction_X, 70+correction_Y), GPoint(87+correction_X, 82+correction_Y));
      graphics_draw_line(ctx, GPoint(87+correction_X, 82+correction_Y), GPoint(77+correction_X, 92+correction_Y));
      graphics_draw_line(ctx, GPoint(77+correction_X, 92+correction_Y), GPoint(67+correction_X, 82+correction_Y));
      graphics_draw_line(ctx, GPoint(67+correction_X, 82+correction_Y), GPoint(75+correction_X, 74+correction_Y));
  
      graphics_draw_line(ctx, GPoint(100+correction_X, 60+correction_Y), GPoint(150+correction_X, 60+correction_Y));
      graphics_draw_line(ctx, GPoint(150+correction_X, 60+correction_Y), GPoint(160+correction_X, 70+correction_Y));
      graphics_draw_line(ctx, GPoint(160+correction_X, 70+correction_Y), GPoint(160+correction_X, 82+correction_Y));
      graphics_draw_line(ctx, GPoint(160+correction_X, 82+correction_Y), GPoint(150+correction_X, 92+correction_Y));
      graphics_draw_line(ctx, GPoint(150+correction_X, 92+correction_Y), GPoint(140+correction_X, 82+correction_Y));
      graphics_draw_line(ctx, GPoint(140+correction_X, 82+correction_Y), GPoint(150+correction_X, 72+correction_Y));
    #else
      graphics_draw_line(ctx, GPoint(2+correction_X, 16+correction_Y), GPoint(92+correction_X, 16+correction_Y));
      graphics_draw_line(ctx, GPoint(92+correction_X, 16+correction_Y), GPoint(100+correction_X, 8+correction_Y));
      graphics_draw_line(ctx, GPoint(100+correction_X, 8+correction_Y), GPoint(92+correction_X, 0+correction_Y));
      graphics_draw_line(ctx, GPoint(92+correction_X, 0+correction_Y), GPoint(84+correction_X, 8+correction_Y));
  
      graphics_draw_line(ctx, GPoint(18+correction_X, 28+correction_Y), GPoint(76+correction_X, 28+correction_Y));
      graphics_draw_line(ctx, GPoint(76+correction_X, 28+correction_Y), GPoint(92+correction_X, 36+correction_Y));
  
      graphics_draw_line(ctx, GPoint(0+correction_X, 40+correction_Y), GPoint(62+correction_X, 40+correction_Y));
      graphics_draw_line(ctx, GPoint(62+correction_X, 40+correction_Y), GPoint(78+correction_X, 48+correction_Y));
      graphics_draw_line(ctx, GPoint(78+correction_X, 48+correction_Y), GPoint(104+correction_X, 48+correction_Y));
      graphics_draw_line(ctx, GPoint(104+correction_X, 48+correction_Y), GPoint(120+correction_X, 40+correction_Y));
  
      graphics_draw_line(ctx, GPoint(2+correction_X, 60+correction_Y), GPoint(50+correction_X, 60+correction_Y));
      graphics_draw_line(ctx, GPoint(50+correction_X, 60+correction_Y), GPoint(66+correction_X, 68+correction_Y));
      graphics_draw_line(ctx, GPoint(66+correction_X, 68+correction_Y), GPoint(66+correction_X, 78+correction_Y));
      graphics_draw_line(ctx, GPoint(66+correction_X, 78+correction_Y), GPoint(58+correction_X, 86+correction_Y));
      graphics_draw_line(ctx, GPoint(58+correction_X, 86+correction_Y), GPoint(50+correction_X, 78+correction_Y));
      graphics_draw_line(ctx, GPoint(50+correction_X, 78+correction_Y), GPoint(58+correction_X, 70+correction_Y));
  
      graphics_draw_line(ctx, GPoint(75+correction_X, 60+correction_Y), GPoint(112+correction_X, 60+correction_Y));
      graphics_draw_line(ctx, GPoint(112+correction_X, 60+correction_Y), GPoint(120+correction_X, 68+correction_Y));
      graphics_draw_line(ctx, GPoint(120+correction_X, 68+correction_Y), GPoint(120+correction_X, 78+correction_Y));
      graphics_draw_line(ctx, GPoint(120+correction_X, 78+correction_Y), GPoint(112+correction_X, 86+correction_Y));
      graphics_draw_line(ctx, GPoint(112+correction_X, 86+correction_Y), GPoint(104+correction_X, 78+correction_Y));
      graphics_draw_line(ctx, GPoint(104+correction_X, 78+correction_Y), GPoint(112+correction_X, 70+correction_Y));
    #endif
  }
  
  #if PBL_DISPLAY_WIDTH == 200
    graphics_draw_line(ctx, GPoint(2+move_to.x-2, 16+move_to.y), GPoint(124+move_to.x, 16+move_to.y));
    graphics_draw_line(ctx, GPoint(18+move_to.x-2, 28+move_to.y), GPoint(102+move_to.x, 28+move_to.y));
    graphics_draw_line(ctx, GPoint(0+move_to.x-2, 40+move_to.y), GPoint(84+move_to.x, 40+move_to.y));
    graphics_draw_line(ctx, GPoint(2+move_to.x-2, 60+move_to.y), GPoint(67+move_to.x, 60+move_to.y));
    graphics_draw_line(ctx, GPoint(100+move_to.x-2, 60+move_to.y), GPoint(150+move_to.x, 60+move_to.y));
  #else
    graphics_draw_line(ctx, GPoint(2+move_to.x-2, 16+move_to.y), GPoint(92+move_to.x, 16+move_to.y));
    graphics_draw_line(ctx, GPoint(18+move_to.x-2, 28+move_to.y), GPoint(76+move_to.x, 28+move_to.y));
    graphics_draw_line(ctx, GPoint(0+move_to.x-2, 40+move_to.y), GPoint(62+move_to.x, 40+move_to.y));
    graphics_draw_line(ctx, GPoint(2+move_to.x-2, 60+move_to.y), GPoint(50+move_to.x, 60+move_to.y));
    graphics_draw_line(ctx, GPoint(75+move_to.x-2, 60+move_to.y), GPoint(112+move_to.x, 60+move_to.y));
  #endif
}

void draw_tornado(Layer *layer, GContext *ctx, GPoint move_to, GColor color_main) {
  int correction_X = 0;
  int correction_Y = 0;
  graphics_context_set_stroke_color(ctx, color_main);
  int max_points = ARRAY_LENGTH(px_shift);
  for (int i=0; i<max_points; i++) {
    correction_X = move_to.x+px_shift[i].x;
    correction_Y = move_to.y+px_shift[i].y;
    #if PBL_DISPLAY_WIDTH == 200
      graphics_draw_line(ctx, GPoint(20+correction_X, 0+correction_Y), GPoint(0+correction_X, 14+correction_Y));
      graphics_draw_line(ctx, GPoint(0+correction_X, 14+correction_Y), GPoint(0+correction_X, 34+correction_Y));
      graphics_draw_line(ctx, GPoint(0+correction_X, 34+correction_Y), GPoint(20+correction_X, 48+correction_Y));
      graphics_draw_line(ctx, GPoint(20+correction_X, 48+correction_Y), GPoint(74+correction_X, 48+correction_Y));
      graphics_draw_line(ctx, GPoint(74+correction_X, 48+correction_Y), GPoint(84+correction_X, 38+correction_Y));
      graphics_draw_line(ctx, GPoint(84+correction_X, 38+correction_Y), GPoint(84+correction_X, 24+correction_Y));
      graphics_draw_line(ctx, GPoint(84+correction_X, 24+correction_Y), GPoint(74+correction_X, 14+correction_Y));
      graphics_draw_line(ctx, GPoint(74+correction_X, 14+correction_Y), GPoint(30+correction_X, 14+correction_Y));
      graphics_draw_line(ctx, GPoint(30+correction_X, 14+correction_Y), GPoint(18+correction_X, 22+correction_Y));
  
      graphics_draw_line(ctx, GPoint(2+correction_X, 50+correction_Y), GPoint(20+correction_X, 62+correction_Y));
      graphics_draw_line(ctx, GPoint(20+correction_X, 62+correction_Y), GPoint(68+correction_X, 62+correction_Y));
  
      graphics_draw_line(ctx, GPoint(4+correction_X, 66+correction_Y), GPoint(21+correction_X, 76+correction_Y));
      graphics_draw_line(ctx, GPoint(21+correction_X, 76+correction_Y), GPoint(53+correction_X, 76+correction_Y));
  
      graphics_draw_line(ctx, GPoint(4+correction_X, 81+correction_Y), GPoint(18+correction_X, 90+correction_Y));
      graphics_draw_line(ctx, GPoint(18+correction_X, 90+correction_Y), GPoint(40+correction_X, 90+correction_Y));
  
      graphics_draw_line(ctx, GPoint(1+correction_X, 93+correction_Y), GPoint(4+correction_X, 98+correction_Y));
      graphics_draw_line(ctx, GPoint(4+correction_X, 98+correction_Y), GPoint(12+correction_X, 102+correction_Y));
      graphics_draw_line(ctx, GPoint(12+correction_X, 102+correction_Y), GPoint(20+correction_X, 102+correction_Y));
  
      graphics_draw_line(ctx, GPoint(-2+correction_X, 107+correction_Y), GPoint(2+correction_X, 110+correction_Y));
    #else
      graphics_draw_line(ctx, GPoint(16+correction_X, 0+correction_Y), GPoint(0+correction_X, 12+correction_Y));
      graphics_draw_line(ctx, GPoint(0+correction_X, 12+correction_Y), GPoint(0+correction_X, 28+correction_Y));
      graphics_draw_line(ctx, GPoint(0+correction_X, 28+correction_Y), GPoint(16+correction_X, 40+correction_Y));
      graphics_draw_line(ctx, GPoint(16+correction_X, 40+correction_Y), GPoint(56+correction_X, 40+correction_Y));
      graphics_draw_line(ctx, GPoint(56+correction_X, 40+correction_Y), GPoint(64+correction_X, 32+correction_Y));
      graphics_draw_line(ctx, GPoint(64+correction_X, 32+correction_Y), GPoint(64+correction_X, 20+correction_Y));
      graphics_draw_line(ctx, GPoint(64+correction_X, 20+correction_Y), GPoint(56+correction_X, 12+correction_Y));
      graphics_draw_line(ctx, GPoint(56+correction_X, 12+correction_Y), GPoint(26+correction_X, 12+correction_Y));
      graphics_draw_line(ctx, GPoint(26+correction_X, 12+correction_Y), GPoint(18+correction_X, 18+correction_Y));
  
      graphics_draw_line(ctx, GPoint(2+correction_X, 41+correction_Y), GPoint(16+correction_X, 50+correction_Y));
      graphics_draw_line(ctx, GPoint(16+correction_X, 50+correction_Y), GPoint(52+correction_X, 50+correction_Y));
  
      graphics_draw_line(ctx, GPoint(4+correction_X, 53+correction_Y), GPoint(17+correction_X, 60+correction_Y));
      graphics_draw_line(ctx, GPoint(17+correction_X, 60+correction_Y), GPoint(44+correction_X, 60+correction_Y));
  
      graphics_draw_line(ctx, GPoint(4+correction_X, 64+correction_Y), GPoint(14+correction_X, 70+correction_Y));
      graphics_draw_line(ctx, GPoint(14+correction_X, 70+correction_Y), GPoint(30+correction_X, 70+correction_Y));
  
      graphics_draw_line(ctx, GPoint(1+correction_X, 72+correction_Y), GPoint(4+correction_X, 77+correction_Y));
      graphics_draw_line(ctx, GPoint(4+correction_X, 77+correction_Y), GPoint(10+correction_X, 80+correction_Y));
      graphics_draw_line(ctx, GPoint(10+correction_X, 80+correction_Y), GPoint(16+correction_X, 80+correction_Y));
  
      graphics_draw_line(ctx, GPoint(-1+correction_X, 84+correction_Y), GPoint(3+correction_X, 87+correction_Y));
    #endif
  }
  
  #if PBL_DISPLAY_WIDTH == 200
    graphics_draw_line(ctx, GPoint(20+move_to.x, 62+move_to.y), GPoint(68+move_to.x+2, 62+move_to.y));
    graphics_draw_line(ctx, GPoint(21+move_to.x, 76+move_to.y), GPoint(53+move_to.x+2, 76+move_to.y));
    graphics_draw_line(ctx, GPoint(18+move_to.x, 90+move_to.y), GPoint(40+move_to.x+2, 90+move_to.y));
    graphics_draw_line(ctx, GPoint(12+move_to.x, 102+move_to.y), GPoint(20+move_to.x+2, 102+move_to.y));
  #else
    graphics_draw_line(ctx, GPoint(16+move_to.x, 50+move_to.y), GPoint(52+move_to.x+2, 50+move_to.y));
    graphics_draw_line(ctx, GPoint(17+move_to.x, 60+move_to.y), GPoint(44+move_to.x+2, 60+move_to.y));
    graphics_draw_line(ctx, GPoint(14+move_to.x, 70+move_to.y), GPoint(30+move_to.x+2, 70+move_to.y));
    graphics_draw_line(ctx, GPoint(10+move_to.x, 80+move_to.y), GPoint(16+move_to.x+2, 80+move_to.y));
  #endif
}
