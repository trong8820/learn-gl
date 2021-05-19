#include "vec2.h"

#include <math.h>

inline constexpr vec2::vec2(float x, float y) :
    x(x), y(y)
{
}

inline float vec2::dot(vec2 const& lhs, vec2 const& rhs)
{
    return lhs.x*rhs.x + lhs.y*rhs.y;
}

inline vec2 vec2::normalize()
{
    float length = sqrtf(x*x + y*y);
    return vec2(x/length, y/length);
}

inline constexpr vec2 operator+(vec2 const& v)
{
    return v;
}

inline constexpr vec2 operator-(vec2 const& v)
{
    return { -v.x, -v.y};
}

inline constexpr bool operator==(vec2 const& lhs, vec2 const& rhs)
{
    return (lhs.x == rhs.x && lhs.y == rhs.y);
}

inline constexpr bool operator!=(vec2 const& lhs, vec2 const& rhs)
{
    return (lhs.x != rhs.x || lhs.y != rhs.y);
}

inline constexpr vec2 operator+(vec2 const& lhs, vec2 const& rhs)
{
    return { lhs.x + rhs.x, lhs.y + rhs.y};
}

inline constexpr vec2 operator-(vec2 const& lhs, vec2 const& rhs)
{
    return { lhs.x - rhs.x, lhs.y - rhs.y};
}

inline constexpr vec2 operator*(vec2 const& lhs, vec2 const& rhs)
{
    return { lhs.x * rhs.x, lhs.y * rhs.y};
}

inline constexpr vec2 operator*(vec2 const& lhs, float const& rhs)
{
    return { lhs.x * rhs, lhs.y * rhs};
}