#include "mat4.h"

#include <cmath>

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
        1.0f, 0.0f, 0.0f, x,
        0.0f, 1.0f, 0.0f, y,
        0.0f, 0.0f, 1.0f, z,
        0.0f, 0.0f, 0.0f, 1.0f
    };
}

inline mat4 mat4::rotate(float x, float y, float z, float angle)
{
    float s = std::sin(angle / 2.0f);
    float qx = x * s;
    float qy = y * s;
    float qz = z * s;
    float qw = std::cos(angle / 2.0f);

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
    vec3 zaxis = (pos - target).normalize();
    vec3 xaxis = vec3::cross(zaxis, up).normalize();
    vec3 yaxis = vec3::cross(xaxis, zaxis);

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
    float t = std::tan(fov / 2.0f) * near;
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