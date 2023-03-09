#ifndef RENDER2D_H
#define RENDER2D_H

#include "linalg.h"

typedef void (*tick_func)(float dt);

void load_font(const char *bitmap_file);
void make_window(int2 top_left, int2 size, const char* title);
void teardown_window();

void main_loop(tick_func tick);

void clear_screen(float3 col);
void draw_rect(float2 top_left, float2 size, float3 col);
void draw_quad(float2 a, float2 b, float2 c, float2 d, float3 col);
void draw_triangle(float2 a, float2 b, float2 c, float3 col);

void draw_text(float2 pos, float size, const char* text, float3 col);

#endif // RENDER2D_H