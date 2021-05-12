#ifndef MAT4_H_
#define MAT4_H_

struct mat4
{
    float m[16];

    static mat4 identity;

    static mat4 translate(float x, float y, float z);
    static mat4 rotate(float x, float y, float z, float angle);
    static mat4 scale(float x, float y, float z);

    static mat4 lookAt(vec3 pos, vec3 target, vec3 up);
    static mat4 perspective(float fov, float aspect, float near, float far);

};

constexpr mat4 operator*(mat4 const& lhs, mat4 const& rhs);

#include "mat4.inl"

#endif // MAT4_H_