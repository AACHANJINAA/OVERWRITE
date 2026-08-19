#pragma once

struct VECTOR3
{
    float x;
    float y;
    float z;
};

struct COLORRGB
{
    float red;
    float green;
    float blue;
};

struct STLNODE
{
    const wchar_t* name;
    VECTOR3 position;
    bool is_bugged;
};
