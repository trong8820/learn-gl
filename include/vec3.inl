#include "vec3.h"

#include <cmath>

inline constexpr vec3::vec3(float x, float y, float z) :
    x(x), y(y), z(z)
{
}

inline vec3 vec3::cross(vec3 const& lhs, vec3 const& rhs)
{
    return vec3
    (
        lhs.y*rhs.z - lhs.z*rhs.y,
        lhs.z*rhs.x - lhs.x*rhs.z,
        lhs.x*rhs.y - lhs.y*rhs.x
    );
}

inline float vec3::dot(vec3 const& lhs, vec3 const& rhs)
{
    return lhs.x*rhs.x + lhs.y*rhs.y + lhs.z*rhs.z;
}

inline vec3 vec3::normalize()
{
    float length = std::sqrt(x*x + y*y + z*z);
    return vec3(x/length, y/length, z/length);
}

inline constexpr vec3 operator+(vec3 const& v)
{
    return v;
}

inline constexpr vec3 operator-(vec3 const& v)
{
    return { -v.x, -v.y, -v.z };
}

inline constexpr bool operator==(vec3 const& lhs, vec3 const& rhs)
{
    return (lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z);
}

inline constexpr bool operator!=(vec3 const& lhs, vec3 const& rhs)
{
    return (lhs.x != rhs.x || lhs.y != rhs.y || lhs.z != rhs.z);
}

inline constexpr vec3 operator+(vec3 const& lhs, vec3 const& rhs)
{
    return { lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z };
}

inline constexpr vec3 operator-(vec3 const& lhs, vec3 const& rhs)
{
    return { lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z };
}

inline constexpr vec3 operator*(vec3 const& lhs, vec3 const& rhs)
{
    return { lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z };
}

inline constexpr vec3 operator*(vec3 const& lhs, float const& rhs)
{
    return { lhs.x * rhs, lhs.y * rhs, lhs.z * rhs };
}