#pragma once

namespace cj_math_helper
{
    constexpr float pi = 3.1415926535f;

    inline float clamp(float _value, float _min, float _max)
    {
        if (_value < _min)
        {
            return _min;
        }

        if (_value > _max)
        {
            return _max;
        }

        return _value;
    }

    inline float lerp(float _start, float _end, float _rate)
    {
        return _start + ((_end - _start) * _rate);
    }

    inline float degree_to_radian(float _degree)
    {
        return _degree * pi / 180.0f;
    }

    inline float radian_to_degree(float _radian)
    {
        return _radian * 180.0f / pi;
    }

    inline float safe_aspect(int _width, int _height)
    {
        if (_height <= 0)
        {
            return 1.0f;
        }

        return static_cast<float>(_width) / static_cast<float>(_height);
    }
}
