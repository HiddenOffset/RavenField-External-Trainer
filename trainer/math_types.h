#pragma once

// This stores a 2D position in client-area pixels.
struct Vec2
{
    float x;
    float y;
};

// This stores a 3D position in Ravenfield world or camera space.
struct Vec3
{
    float x;
    float y;
    float z;
};

// Unity stores Matrix4x4 fields in column-major field order in memory.
// The named members let the transform code use Unity's m00/m01/etc. notation directly.
struct UnityMatrix4x4
{
    float m00;
    float m10;
    float m20;
    float m30;

    float m01;
    float m11;
    float m21;
    float m31;

    float m02;
    float m12;
    float m22;
    float m32;

    float m03;
    float m13;
    float m23;
    float m33;
};

// A Unity Matrix4x4 must occupy exactly sixteen floats, or 0x40 bytes.
static_assert(sizeof(UnityMatrix4x4) == 0x40, "UnityMatrix4x4 must be 0x40 bytes.");
