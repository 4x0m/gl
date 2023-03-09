#include "linalg.h"
#include "render2d.h"
#include <stdio.h>

void tick(float dt)
{
    // printf("Tick: %f	\n", dt);
    clear_screen(GREY);
    draw_rect(FLOAT2(0.0f, 0.0f), FLOAT2(0.8f, 0.8f), RED);
    draw_rect(FLOAT2(-0.8f, 0.8f), FLOAT2(0.8f, 0.8f), RED);
    draw_rect(FLOAT2(-0.8f, 0.0f), FLOAT2(0.8f, 0.8f), BLUE);
    draw_rect(FLOAT2(0.0f, 0.8f), FLOAT2(0.8f, 0.8f), BLUE);

    const rad rad_per_sec = DEG(90.f);
    static rad angle = 0.0f;
    angle += dt * rad_per_sec;
    angle = normalize(angle);

    float2 a = rotatef2(FLOAT2(-0.25f, -0.25f), angle);
    float2 b = rotatef2(FLOAT2( 0.25f, -0.25f), angle);
    float2 c = rotatef2(FLOAT2( 0.25f,  0.25f), angle);
    float2 d = rotatef2(FLOAT2(-0.25f,  0.25f), angle);
    draw_quad(a, b, c, d, GREEN);

    draw_text(bcastf2(-0.8f), 0.075f, "Hello world!", WHITE);
    draw_text(FLOAT2(0, 0.8f), 0.075f, "Meep Moop", WHITE);
}

int main(int argc, char *argv[])
{
    make_window(INT2(100, 100), INT2(800, 600), "2D Render Test");
    load_font("../ExportedFont.bmp");

    printf("Hello 2D Render Test!\n");

    main_loop(tick);

    teardown_window();

    return 0;
}