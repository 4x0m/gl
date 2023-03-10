#include "linalg.h"
#include "gl_utils.h"
#include "render2d.h"
#include <stdio.h>

void tick(float dt)
{
    // printf("Tick: %f	\n", dt);
    clear_screen(GREY);

    const rad rad_per_sec = DEG(90.f);
    static rad angle = 0.0f;
    angle += dt * rad_per_sec;
    angle = normalize(angle);

    float2 a = rotatef2(FLOAT2(-0.25f, -0.25f), angle);
    float2 b = rotatef2(FLOAT2( 0.25f, -0.25f), angle);
    float2 c = rotatef2(FLOAT2( 0.25f,  0.25f), angle);
    float2 d = rotatef2(FLOAT2(-0.25f,  0.25f), angle);
    draw_quad(a, b, c, d, GREEN);
}

int main(int argc, char *argv[])
{
    UNUSED(argc);
    UNUSED(argv);

    make_window(INT2(100, 100), INT2(800, 600), "2D Render Test");
    load_font("../ExportedFont.png");

    printf("Hello 2D Render Test!\n");

    main_loop(tick);

    teardown_window();

    return 0;
}
