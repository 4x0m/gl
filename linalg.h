#ifndef LINALG_H
#define LINALG_H

#include <math.h>

#define PI 3.14159265358979323846f
#define DEG_TO_RAD 0.0174532925f // pi / 180.0f
#define RAD_TO_DEG 57.2957795f // 180.0f / pi

typedef float rad;
#define RAD(x) (x)
#define DEG(x) ((x) * DEG_TO_RAD)

inline rad normalize(rad angle) {
    angle = fmodf(angle, 2.0f * PI);
    if (angle < 0.0f) {
        angle += 2.0f * PI;
    }
    return angle;
}

typedef struct {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
} rgba;

typedef struct {
    int x;
    int y;
} int2;
#define INT2(x, y) ((int2){x, y})

typedef struct {
    float x;
    float y;
} float2;
#define FLOAT2(x, y) ((float2){x, y})

typedef struct {
    float x;
    float y;
    float z;
} float3;
#define FLOAT3(x, y, z) ((float3){x, y, z})
#define RGB(r, g, b) ((float3){r, g, b})
#define R(col) (col.x)
#define G(col) (col.y)
#define B(col) (col.z)
#define BLACK RGB(0.0f, 0.0f, 0.0f)
#define WHITE RGB(1.0f, 1.0f, 1.0f)
#define RED RGB(1.0f, 0.0f, 0.0f)
#define GREEN RGB(0.0f, 1.0f, 0.0f)
#define BLUE RGB(0.0f, 0.0f, 1.0f)
#define GREY RGB(0.5f, 0.5f, 0.5f)

inline float2 bcastf2(float a) {
    return FLOAT2(a, a);
}

inline float2 addf2(float2 a, float2 b) {
    return FLOAT2(a.x + b.x, a.y + b.y);
}

inline float2 subf2(float2 a, float2 b) {
    return FLOAT2(a.x - b.x, a.y - b.y);
}

inline float2 mulf2(float2 a, float2 b) {
    return FLOAT2(a.x * b.x, a.y * b.y);
}

inline float2 divf2(float2 a, float2 b) {
    return FLOAT2(a.x / b.x, a.y / b.y);
}

inline float2 rotatef2(float2 a, rad angle) {
    float s = sinf(angle);
    float c = cosf(angle);
    return FLOAT2(a.x * c - a.y * s, a.x * s + a.y * c);
}

inline float2 rotate_aroundf2(float2 a, float2 origin, rad angle) {
    return addf2(rotatef2(subf2(a, origin), angle), origin);
}

inline float3 bcastf3(float a) {
    return FLOAT3(a, a, a);
}

inline float3 addf3(float3 a, float3 b) {
    return FLOAT3(a.x + b.x, a.y + b.y, a.z + b.z);
}

inline float3 subf3(float3 a, float3 b) {
    return (float3){a.x - b.x, a.y - b.y, a.z - b.z};
}

inline float3 mulf3(float3 a, float3 b) {
    return (float3){a.x * b.x, a.y * b.y, a.z * b.z};
}

inline float3 divf3(float3 a, float3 b) {
    return (float3){a.x / b.x, a.y / b.y, a.z / b.z};
}

inline float3 mulf3s(float3 a, float b) {
    return (float3){a.x * b, a.y * b, a.z * b};
}

inline float3 divf3s(float3 a, float b) {
    return (float3){a.x / b, a.y / b, a.z / b};
}

inline float3 cosf3(float3 a) {
    return (float3){cosf(a.x), cosf(a.y), cosf(a.z)};
}

#endif // LINALG_H
