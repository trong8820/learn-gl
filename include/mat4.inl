#include "mat4.h"

#include <math.h>

#include "vec3.h"

mat4 mat4::identity = mat4
{
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
};

inline mat4 mat4::translate(float x, float y, float z)
{
    return mat4
    {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        x, y, z, 1.0f
    };
}

inline mat4 mat4::rotate(float x, float y, float z, float angle)
{
    float s = sinf(angle / 2.0f);
    float qx = x * s;
    float qy = y * s;
    float qz = z * s;
    float qw = cosf(angle / 2.0f);

    float xx = qx * qx;
    float xy = qx * qy;
    float xz = qx * qz;
    float xw = qx * qw;

    float yy = qy * qy;
    float yz = qy * qz;
    float yw = qy * qw;

    float zz = qz * qz;
    float zw = qz * qw;

    return mat4
    {
        1.0f -  2.0f * ( yy + zz ),         2.0f * ( xy - zw ),         2.0f * ( xz + yw ), 0.0f,
                2.0f * ( xy + zw ), 1.0f -  2.0f * ( xx + zz ),         2.0f * ( yz - xw ), 0.0f,
                2.0f * ( xz - yw ),         2.0f * ( yz + xw ), 1.0f -  2.0f * ( xx + yy ), 0.0f,
                              0.0f,                       0.0f,                       0.0f, 1.0f
    };
}

inline mat4 mat4::scale(float x, float y, float z)
{
    return mat4
    {
        x, 0.0f, 0.0f, 0.0f,
        0.0f, y, 0.0f, 0.0f,
        0.0f, 0.0f, z, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
}

inline mat4 mat4::lookAt(vec3 pos, vec3 target, vec3 up)
{
    vec3 zaxis = (target - pos).normalize();
    vec3 xaxis = vec3::cross(zaxis, up).normalize();
    vec3 yaxis = vec3::cross(xaxis, zaxis);

    zaxis = -zaxis;

    return mat4
    {
        xaxis.x, yaxis.x, zaxis.x, 0.0f,
        xaxis.y, yaxis.y, zaxis.y, 0.0f,
        xaxis.z, yaxis.z, zaxis.z, 0.0f,
        -vec3::dot(xaxis, pos), -vec3::dot(yaxis, pos), -vec3::dot(zaxis, pos), 1.0f
    };
}

inline mat4 mat4::perspective(float fov, float aspect, float near, float far)
{
    float t = tanf(fov / 2.0f) * near;
    float b = -t;
    float r = t * aspect;
    float l = -r;

    return mat4
    {
        (2.0f*near)/(r-l), 0.0f, 0.0f, 0.0f,
        0.0f, (2.0f*near)/(t-b), 0.0f, 0.0f,
        (r+l)/(r-l), (t+b)/(t-b), -(far+near)/(far-near), -1.0f,
        0.0f, 0.0f, -(2.0f*far*near)/(far-near), 0.0f
    };
}

inline mat4 mat4::ortho(float width, float height, float near, float far)
{
    return mat4
    {
        2.0f/width, 0.0f, 0.0f, 0.0f,
        0.0f, 2.0f/height, 0.0f, 0.0f,
        0.0f, 0.0f, -2.0f/(far - near), 0.0f,
        0.0f, 0.0f, -(far + near)/(far - near), 1.0f
    };
}

inline constexpr mat4 operator*(mat4 const& lhs, mat4 const& rhs)
{
    return mat4
    {
        lhs.m[0] * rhs.m[0] + lhs.m[1] * rhs.m[4] + lhs.m[2] * rhs.m[8] +  lhs.m[3] * rhs.m[12],
        lhs.m[0] * rhs.m[1] + lhs.m[1] * rhs.m[5] + lhs.m[2] * rhs.m[9] +  lhs.m[3] * rhs.m[13],
        lhs.m[0] * rhs.m[2] + lhs.m[1] * rhs.m[6] + lhs.m[2] * rhs.m[10] + lhs.m[3] * rhs.m[14],
        lhs.m[0] * rhs.m[3] + lhs.m[1] * rhs.m[7] + lhs.m[2] * rhs.m[11] + lhs.m[3] * rhs.m[15],

        lhs.m[4] * rhs.m[0] + lhs.m[5] * rhs.m[4] + lhs.m[6] * rhs.m[8] +  lhs.m[7] * rhs.m[12],
        lhs.m[4] * rhs.m[1] + lhs.m[5] * rhs.m[5] + lhs.m[6] * rhs.m[9] + lhs.m[7] * rhs.m[13],
        lhs.m[4] * rhs.m[2] + lhs.m[5] * rhs.m[6] + lhs.m[6] * rhs.m[10] + lhs.m[7] * rhs.m[14],
        lhs.m[4] * rhs.m[3] + lhs.m[5] * rhs.m[7] + lhs.m[6] * rhs.m[11] + lhs.m[7] * rhs.m[15],

        lhs.m[8] * rhs.m[0] + lhs.m[9] * rhs.m[4] + lhs.m[10] * rhs.m[8] +  lhs.m[11] * rhs.m[12],
        lhs.m[8] * rhs.m[1] + lhs.m[9] * rhs.m[5] + lhs.m[10] * rhs.m[9] + lhs.m[11] * rhs.m[13],
        lhs.m[8] * rhs.m[2] + lhs.m[9] * rhs.m[6] + lhs.m[10] * rhs.m[10] + lhs.m[11] * rhs.m[14],
        lhs.m[8] * rhs.m[3] + lhs.m[9] * rhs.m[7] + lhs.m[10] * rhs.m[11] + lhs.m[11] * rhs.m[15],

        lhs.m[12] * rhs.m[0] + lhs.m[13] * rhs.m[4] + lhs.m[14] * rhs.m[8] +  lhs.m[15] * rhs.m[12],
        lhs.m[12] * rhs.m[1] + lhs.m[13] * rhs.m[5] + lhs.m[14] * rhs.m[9] + lhs.m[15] * rhs.m[13],
        lhs.m[12] * rhs.m[2] + lhs.m[13] * rhs.m[6] + lhs.m[14] * rhs.m[10] + lhs.m[15] * rhs.m[14],
        lhs.m[12] * rhs.m[3] + lhs.m[13] * rhs.m[7] + lhs.m[14] * rhs.m[11] + lhs.m[15] * rhs.m[15]
    };
}

inline constexpr vec4 operator*(mat4 const& lhs, vec4 const& rhs)
{
    return vec4
    {
        lhs.m[0] * rhs.x + lhs.m[1] * rhs.y + lhs.m[2] * rhs.z +  lhs.m[3] * rhs.w,

        lhs.m[4] * rhs.x + lhs.m[5] * rhs.y + lhs.m[6] * rhs.z +  lhs.m[7] * rhs.w,

        lhs.m[8] * rhs.x + lhs.m[9] * rhs.y + lhs.m[10] * rhs.z +  lhs.m[11] * rhs.w,

        lhs.m[12] * rhs.x + lhs.m[13] * rhs.y + lhs.m[14] * rhs.z +  lhs.m[15] * rhs.w
    };
}