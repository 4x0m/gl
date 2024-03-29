#include "render2d.h"

#include "gl_utils.h"
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_image.h>
#include <stdbool.h>

#define GL_VERSION_MAJOR 4
#define GL_VERSION_MINOR 6

#define PREALLOC_VERTICES 1024
#define PREALLOC_INDICES 1024

// Internal types
// typedef struct {
//     float2 pos;
//     float3 col;
// } vertex;

typedef GLuint glid;

typedef struct {
    float2 pos;
    float2 tex;
    float3 col;
} text_vertex;

typedef struct {
    glid shader;
    glid vao;
    glid vertex_buffer;
    glid index_buffer;
    bool is_indexed;
    u32 n_vertices;
    u32 n_indices;
} render_step;

typedef struct {
    glid shader;
    glid vao;
    glid font_texture;
    glid vertex_buffer;
    glid index_buffer;
    u32 n_vertices;
    u32 n_indices;
} text_render_step;

typedef struct {
    int max_fps;
} settings;

// Internal functions
static void do_render();

// Internal globals / state
static SDL_Window* g_window;
static SDL_GLContext g_glcontext;
static settings g_settings;
static render_step g_render_triangles;
static text_render_step g_render_text;

static void do_render()
{
    GL_CALL(glBindVertexArray(g_render_triangles.vao));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, g_render_triangles.vertex_buffer));
    GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_render_triangles.index_buffer));
    GL_CALL(glUseProgram(g_render_triangles.shader));
    GL_CALL(glDrawElements(GL_TRIANGLES, g_render_triangles.n_indices, GL_UNSIGNED_INT, 0));

    GL_CALL(glBindVertexArray(g_render_text.vao));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, g_render_text.vertex_buffer));
    GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_render_text.index_buffer));
    GL_CALL(glUseProgram(g_render_text.shader));
    GL_CALL(glDrawElements(GL_TRIANGLES, g_render_text.n_indices, GL_UNSIGNED_INT, 0));
}

void load_font(const char *bitmap_file)
{
    SDL_Surface *surface = IMG_Load(bitmap_file);
    if (surface == NULL)
    {
        printf("Failed to load font bitmap '%s': %s\n", bitmap_file, SDL_GetError());
        abort();
    }

    GL_CALL(glBindVertexArray(g_render_text.vao));
    GL_CALL(glGenTextures(1, &g_render_text.font_texture));
    GL_CALL(glBindTexture(GL_TEXTURE_2D, g_render_text.font_texture));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels));

    SDL_FreeSurface(surface);
}

void make_window(int2 top_left, int2 size, const char* title)
{
    SDL_Init(SDL_INIT_VIDEO);

    int imgFlags = IMG_INIT_JPG | IMG_INIT_PNG;
    if ( !(IMG_Init(imgFlags) & imgFlags))
    {
        printf( "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError() );
        abort();
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, GL_VERSION_MAJOR);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, GL_VERSION_MINOR);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetSwapInterval(0); // Toggle VSync (0 for off, 1 for on)
    g_window = SDL_CreateWindow(title, top_left.x, top_left.y, size.x, size.y, SDL_WINDOW_OPENGL);

    g_glcontext = SDL_GL_CreateContext(g_window);

    glewExperimental = true; // Enable modern OpenGL features
    // glewInit must be called _AFTER_ creating openGL context
    GLenum glew_err = glewInit();
    if (GLEW_OK != glew_err)
    {
        printf("glewInit() failed! Error: %s\n", glewGetErrorString(glew_err));
        abort();
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // =====================================================
    // =============== DEFAULT SETTINGS
    // =====================================================
    g_settings.max_fps = 60;

    // =====================================================
    // =============== SETUP TRIANGLE RENDER
    // =====================================================

    g_render_triangles.is_indexed = true;

    /// Create Vertex Array Object (VAO)
    // VAO stores vertex attribute configuration
    // Acts as a context for vertex attributes (e.g. position, color, ...) and buffers
    GL_CALL(glGenVertexArrays(1, &g_render_triangles.vao));
    GL_CALL(glBindVertexArray(g_render_triangles.vao));

    // Create Vertex Buffer Object
    GL_CALL(glGenBuffers(1, &g_render_triangles.vertex_buffer));

    // Create buffer (make active?) on GPU?
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, g_render_triangles.vertex_buffer));

    // Copy vertices to vertex buffer
    // Data is copied to active buffer (set with glBindBuffer)
    // Last parameter: usage of data
    //   - GL_STATIC_DRAW: uploaded once, drawn many times
    //   - GL_DYNAMIC_DRAW: vertex data created once, sometimes changes, drawn many times
    //   - GL_STREAM_DRAW: vertex data uploaded once, drawn once
    // Pre-allocate memory for vertices
    // TODO: use glBufferStorage for persistent mapping
    GL_CALL(glBufferData(GL_ARRAY_BUFFER, (sizeof(float2) + sizeof(float3)) * PREALLOC_VERTICES, NULL, GL_STREAM_DRAW));

    // Create Element Buffer Object + pre-allocate memory
    GL_CALL(glGenBuffers(1, &g_render_triangles.index_buffer));
    GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_render_triangles.index_buffer));
    // TODO: use glBufferStorage for persistent mapping
    GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * PREALLOC_INDICES, NULL, GL_STREAM_DRAW));

    // =====================================================
    // =============== SHADER
    // =====================================================

    // VERTEX SHADER
    // Processes vertices -> output vertex positions in device coordinates
    // Output: gl_Position (vec4 but 3d position with 1 in last entry. Why?)
    const char *vertexSource =
        "#version 150 core\n"
        "// input\n"
        "in vec2 position; // input 2d position of vertice\n"
        "in vec3 colorVertex; // input RGB color value of vertice\n"
        "//in vec2 texcoordVertex; // 2d texture coordinate\n"
        "// output\n"
        "flat out vec3 colorFragment; // output RGB color for fragment shader\n"
        "//out vec2 texcoordFragment; // 2d texture coord (rasterized)\n"
        "void main()\n"
        "{\n"
        "    colorFragment = colorVertex;\n"
        "    //texcoordFragment = texcoordVertex;\n"
        "    // Map 2d position of triangle vertice onto 3d space\n"
        "    vec2 positionOut = position;\n"
        "    // if (positionOut.y > 0)\n"
        "    //     positionOut.y *= -1;\n"
        "    gl_Position = vec4(positionOut, 0.0, 1.0);\n"
        "}\n";

    // FRAGMENT SHADER
    // Interpolate vertex shader result over pixels on screen covered by primitive
    // fragment: pixels covered by primitive
    // Output: outColor (color of pixel)
    const char *fragmentSource =
        "#version 150 core\n"
        "// Uniform argument: Input that's identical for all fragments / vertices\n"
        "// Ignored for now, just keeping this as an example\n"
        "uniform vec3 triangleColor;\n"
        "// Texture to draw, defaults to 0, so doesn't have to be set on host if only one texture\n"
        "//uniform sampler2D tex;\n"
        "flat in vec3 colorFragment;\n"
        "//in vec2 texcoordFragment;\n"
        "out vec4 outColor;\n"
        "void main()\n"
        "{\n"
        "    // outColor = texture(tex, texcoordFragment) * vec4(mix(colorFragment, triangleColor, 0.5), 1.0);\n"
        "    // outColor = texture(tex, texcoordFragment) * vec4(colorFragment, 1.0);\n"
        "    // outColor = texture(tex, texcoordFragment);\n"
        "    outColor = vec4(colorFragment, 1.0);\n"
        "}\n";

    // Create shader program
    g_render_triangles.shader = gl_compile_shader(vertexSource, fragmentSource, "outColor");

    // 6. Use program
    GL_CALL(glUseProgram(g_render_triangles.shader));

    // 7. Link vertex data + shader attributes
    GLint posAttrib = GL_CALL(glGetAttribLocation(g_render_triangles.shader, "position"));
    // Set attributes properties and BIND ACTIVE VBO TO THIS ATTRIBUTE
    // REQUIRES ACTIVE VAO
    GL_CALL(glEnableVertexAttribArray(posAttrib));
    GL_CALL(glVertexAttribPointer(
        posAttrib, // attribute index
        2, // size
        GL_FLOAT, // type
        GL_FALSE, // Normalise non float input???
        sizeof(float2), // stride (bytes between attributes)
        0 // offset (bytes) to first attribute
    ));
    GL_CALL(glVertexAttribDivisor(posAttrib, 0)); // 0: per vertex, 1: per instance

    // input colorVertex attribute from vertexShader
    // TODO: tightly pack colors as 4 byte iteger:
    //
    // https://stackoverflow.com/a/54658686
    // Aight, so this did the job for me:
    // Having managed data as int[] where it's tightly packed array of only colors
    // (where int format is: RGBA meaning 0xAABBGGRR),
    // then defining vertex attribute as:
    //  GL.VertexAttribPointer(index, 4, VertexAttribPointerType.UnsignedByte, true, sizeof(int), IntPtr.Zero)
    // and using it in shader as: in vec4 in_color;.


    GLint colorAttrib = GL_CALL(glGetAttribLocation(g_render_triangles.shader, "colorVertex"));
    GL_CALL(glEnableVertexAttribArray(colorAttrib));
    GL_CALL(glVertexAttribPointer(colorAttrib, 3, GL_FLOAT, GL_FALSE, sizeof(float3), (void*)(PREALLOC_VERTICES*sizeof(float2))));
    GL_CALL(glVertexAttribDivisor(colorAttrib, 0)); // 1: per instance, 0: per vertex (default

    // input texcoordVertex attribute from vertexShader
    // GLint texAttrib = GL_CALL(glGetAttribLocation(g_shader_triangle, "texcoordVertex"));
    // GL_CALL(glEnableVertexAttribArray(texAttrib));
    // GL_CALL(glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 7*sizeof(float), (void*)(5*sizeof(float))));

    // =====================================================
    // SETUP TEXT RENDERING
    // =====================================================

    GL_CALL(glGenVertexArrays(1, &g_render_text.vao));
    GL_CALL(glBindVertexArray(g_render_text.vao));

    // Create vertex buffer
    GL_CALL(glGenBuffers(1, &g_render_text.vertex_buffer));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, g_render_text.vertex_buffer));
    GL_CALL(glBufferData(GL_ARRAY_BUFFER, PREALLOC_VERTICES*sizeof(text_vertex), NULL, GL_DYNAMIC_DRAW));

    // Create index buffer
    GL_CALL(glGenBuffers(1, &g_render_text.index_buffer));
    GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_render_text.index_buffer));
    GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, PREALLOC_INDICES*sizeof(GLuint), NULL, GL_DYNAMIC_DRAW));

    const char *textVertexSource =
        "#version 330 core\n"
        "// input\n"
        "layout(location = 0) in vec2 position; // input 2d position of vertice\n"
        "layout(location = 1) in vec2 texcoordVertex; // 2d texture coordinate\n"
        "layout(location = 2) in vec3 colorVertex; // color of vertex\n"
        "// output\n"
        "out vec2 texcoordFragment; // 2d texture coord (rasterized)\n"
        "out vec3 colorFragment; // color of vertex (rasterized)\n"
        "void main()\n"
        "{\n"
        "    texcoordFragment = texcoordVertex;\n"
        "    colorFragment = colorVertex;\n"
        "    // Map 2d position of triangle vertice onto 3d space\n"
        "    gl_Position = vec4(position, 0.0, 1.0);\n"
        "}\n";

    const char *textFragmentSource =
        "#version 330 core\n"
        "// Texture to draw, defaults to 0, so doesn't have to be set on host if only one texture\n"
        "uniform sampler2D font_texture;\n"
        "in vec2 texcoordFragment;\n"
        "in vec3 colorFragment;\n"
        "out vec4 outColor;\n"
        "void main()\n"
        "{\n"
        "    outColor = vec4(colorFragment, 1.0) * texture(font_texture, texcoordFragment);\n"
        "}\n";

    g_render_text.shader = gl_compile_shader(textVertexSource, textFragmentSource, "outColor");

    GL_CALL(glUseProgram(g_render_text.shader));

    GLint pos_attrib_text = 0; // GL_CALL(glGetAttribLocation(g_render_text.shader, "position"));
    GL_CALL(glEnableVertexAttribArray(pos_attrib_text));
    GL_CALL(glVertexAttribPointer(pos_attrib_text, 2, GL_FLOAT, GL_FALSE, sizeof(text_vertex), 0));

    // GLint color_attrib_text = 1; // GL_CALL(glGetAttribLocation(g_render_text.shader, "colorVertex"));
    // // printf("color_attrib_text: %d\n", color_attrib_text);
    // GL_CALL(glEnableVertexAttribArray(color_attrib_text));
    // GL_CALL(glVertexAttribPointer(color_attrib_text, 3, GL_FLOAT, GL_FALSE, sizeof(float3), (void*)(sizeof(float2))));

    GLint texcoord_attrib_text = 1; //GL_CALL(glGetAttribLocation(g_render_text.shader, "texcoordVertex"));
    GL_CALL(glEnableVertexAttribArray(texcoord_attrib_text));
    GL_CALL(glVertexAttribPointer(texcoord_attrib_text, 2, GL_FLOAT, GL_FALSE, sizeof(text_vertex), (void*)(2*sizeof(float))));

    GLint color_attrib_text = 2; // GL_CALL(glGetAttribLocation(g_render_text.shader, "colorVertex"));
    GL_CALL(glEnableVertexAttribArray(color_attrib_text));
    GL_CALL(glVertexAttribPointer(color_attrib_text, 3, GL_FLOAT, GL_FALSE, sizeof(text_vertex), (void*)(4*sizeof(float))));
}

void teardown_window()
{
    glDeleteTextures(1, &g_render_text.font_texture);
    glDeleteBuffers(1, &g_render_text.index_buffer);
    glDeleteBuffers(1, &g_render_text.vertex_buffer);
    glDeleteVertexArrays(1, &g_render_text.vao);
    glDeleteProgram(g_render_text.shader);
    glDeleteBuffers(1, &g_render_triangles.index_buffer);
    glDeleteBuffers(1, &g_render_triangles.vertex_buffer);
    glDeleteVertexArrays(1, &g_render_triangles.vao);
    glDeleteProgram(g_render_triangles.shader);
    SDL_GL_DeleteContext(g_glcontext);
    SDL_DestroyWindow(g_window);
    SDL_Quit();
}

void main_loop(tick_func tick) {
    bool doRun = true;
    Uint64 prev_tick_end = 0;
    while (doRun)
    {
        // React to new events
        SDL_Event windowEvent;
        while (SDL_PollEvent(&windowEvent))
        {
            if (windowEvent.type == SDL_QUIT) doRun = false;
        }

        Uint64 tick_start = SDL_GetPerformanceCounter();
        if (prev_tick_end != 0) {
            float delta = (tick_start - prev_tick_end) / (float)SDL_GetPerformanceFrequency();
            tick(delta);
        }
        prev_tick_end = SDL_GetPerformanceCounter();
        // printf("indices: %zd", g_n_indices);
        // printf("; vertices: %zd\n", g_n_vertices);

        do_render();
        SDL_GL_SwapWindow(g_window); // Swap front- and backbuffer

        // TODO use SDL_Ticks64 instead?
        Uint64 tick_end = SDL_GetPerformanceCounter();

        if (g_settings.max_fps > 0) {
            float ms_per_frame = 1000.f / g_settings.max_fps;
            float ms_per_frame_actual = 1000.f * (tick_end - tick_start) / (float)SDL_GetPerformanceFrequency();
            if (ms_per_frame_actual < ms_per_frame) {

                SDL_Delay((u32)(ms_per_frame - ms_per_frame_actual));
            }
        }
    }
}

void clear_screen(float3 col)
{
    g_render_triangles.n_indices = 0;
    g_render_triangles.n_vertices = 0;
    g_render_text.n_indices = 0;
    g_render_text.n_vertices = 0;
    GL_CALL(glClearColor(col.x, col.y, col.z, 1.0f));
    GL_CALL(glClear(GL_COLOR_BUFFER_BIT));
}

void draw_rect(float2 top_left, float2 size, float3 col)
{
    float2 a = top_left;
    float2 b = {top_left.x + size.x, top_left.y};
    float2 c = {top_left.x + size.x, top_left.y - size.y};
    float2 d = {top_left.x, top_left.y - size.y};
    draw_quad(a, b, c, d, col);
}

void draw_quad(float2 a, float2 b, float2 c, float2 d, float3 col)
{
    // =====================================================
    // =============== VERTICES
    // =====================================================

    // TODO: Use named buffer subdata to avoid mapping/unmapping buffer
    GL_CALL(glBindVertexArray(g_render_triangles.vao));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, g_render_triangles.vertex_buffer));
    GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_render_triangles.index_buffer));

    // Check if vertex buffer is large enough
    i32 vertex_buffer_capacity;
    GL_CALL(glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &vertex_buffer_capacity));
    // printf("vertex_buffer_capacity: %zd\n", vertex_buffer_capacity);
    if (g_render_triangles.n_vertices + 4 > vertex_buffer_capacity / (5*sizeof(float))) {
        printf("Vertex buffer overflow, increase vertex buffer size\n");
        abort();
    }

    // Check if index buffer is large enough
    i32 index_buffer_capacity;
    GL_CALL(glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &index_buffer_capacity));
    if (g_render_triangles.n_indices + 6 > index_buffer_capacity / sizeof(GLuint)) {
        printf("Index buffer overflow, increase index buffer size\n");
        abort();
    }

    // Vertices (Eckpunkte) to draw rectangle from two triangles
    // Gives only 4 cornes of rectangle, as top-left and bottom-right vertice
    // are shared by both triangles, reuse of these points is done via 'elements' array
    // furter down, that maps vertices onto this array to allow reusing points
    // OpenGL coordinates range is [-1, 1] in x and y direction

    // vertex rect[4] = {
    //     { a, col, },
    //     { b, col, },
    //     { c, col, },
    //     { d, col, },
    // };
    // GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, g_render_triangles.n_vertices * sizeof(vertex), sizeof(rect), rect));
    float2 vertices[4] = {
        a, b, c, d,
    };
    GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, g_render_triangles.n_vertices * sizeof(float2), sizeof(vertices), vertices));

    float3 colors[4] = {
        col, col, col, col,
    };
    // float3 colors[1] = {
    //     col,
    // };
    // int n_triangles = g_render_triangles.n_vertices / 6;
    GLintptr offset = sizeof(float2) * PREALLOC_VERTICES + g_render_triangles.n_vertices * sizeof(float3);
    // printf("offset: %zd\n", offset);
    GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(colors), colors));

    GLuint nv = g_render_triangles.n_vertices;
    GLuint indices[6] = {
        nv + 0, nv + 1, nv + 2,
        nv + 2, nv + 3, nv + 0,
    };
    // GLuint indices[6] = {
    //     0, 1, 2, 2, 3, 0,
    // };
    GL_CALL(glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, g_render_triangles.n_indices * sizeof(GLuint), sizeof(indices), indices));

    g_render_triangles.n_vertices += 4;
    g_render_triangles.n_indices += 6;
}

// void draw_triangle(float2 a, float2 b, float2 c, float3 col)
// {
//     // TODO
// }

void draw_text(float2 pos, float size, float3 col, const char *text)
{
    const char TOP_LEFT = '!' - 1;
    const int CELLS_PER_ROW = 16;
    const int CELLS_PER_COLUMN = 8;
    const float CELL_WIDTH_UV = 1.0f / CELLS_PER_ROW / 2.f;
    const float CELL_HEIGHT_UV = 1.0f / CELLS_PER_COLUMN;

    size_t len = strlen(text);

    // TODO: check vertex / index buffer overflows

    for (size_t i = 0; i < len; i++) {
        char c = text[i];
        if (c == ' ') {
            continue;
        }

        float2 vertices[4] = {
            {pos.x + i * size, pos.y + size},  // top left
            {pos.x + i * size + size, pos.y + size}, // top right
            {pos.x + i * size + size, pos.y}, // bottom right
            {pos.x + i * size, pos.y}, // bottom left
        };

        c -= TOP_LEFT;
        float2 bitmapPos = FLOAT2(c % CELLS_PER_ROW, c / CELLS_PER_ROW);
        bitmapPos = divf2(bitmapPos, FLOAT2(CELLS_PER_ROW, CELLS_PER_COLUMN));

        float2 bitmap_vertices[4] = {
            {bitmapPos.x, bitmapPos.y},  // top left
            {bitmapPos.x + CELL_WIDTH_UV, bitmapPos.y}, // top right
            {bitmapPos.x + CELL_WIDTH_UV, bitmapPos.y + CELL_HEIGHT_UV}, // bottom right
            {bitmapPos.x, bitmapPos.y + CELL_HEIGHT_UV}, // bottom left
        };

        text_vertex vs[4] = {
            {vertices[0], bitmap_vertices[0], col},
            {vertices[1], bitmap_vertices[1], col},
            {vertices[2], bitmap_vertices[2], col},
            {vertices[3], bitmap_vertices[3], col},
        };

        size_t offset = g_render_text.n_vertices * sizeof(text_vertex);
        GL_CALL(glNamedBufferSubData(g_render_text.vertex_buffer, offset, sizeof(vs), vs));

        GLuint nv = g_render_text.n_vertices;
        GLuint indices[6] = {
            nv + 0, nv + 1, nv + 2,
            nv + 2, nv + 3, nv + 0,
        };
        GL_CALL(glNamedBufferSubData(g_render_text.index_buffer, g_render_text.n_indices * sizeof(GLuint), sizeof(indices), indices));

        g_render_text.n_vertices += 4;
        g_render_text.n_indices += 6;
    }
}

void draw_textf_i(float2 pos, float size, float3 col, const char* fmt, ...)
{
    char buf[256];
    va_list args;
    va_start(args, fmt);
    int bytes_written = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    if (bytes_written < 0) {
        printf("Error: vsnprintf failed\n");
        abort();
    }

    // fast path for small strings
    if (bytes_written < sizeof(buf)) {
        draw_text(pos, size, col, buf);
        return;
    }

    // allocate a buffer large enough to hold the string
    va_start(args, fmt);
    bytes_written = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    char *large_buf = malloc(bytes_written + 1);

    va_start(args, fmt);
    vsnprintf(large_buf, bytes_written + 1, fmt, args);
    va_end(args);

    draw_text(pos, size, col, large_buf);
    free(large_buf);
}
