#ifndef VEC3_H_
#define VEC3_H_

struct vec3
{
	float x{};
	float y{};
	float z{};

	// -- Implicit basic constructors --
	constexpr vec3() = default;
	constexpr vec3(vec3 const& v) = default;

	// -- Explicit basic constructors --
	constexpr vec3(float x, float y, float z);

	static vec3 cross(vec3 const& lhs, vec3 const& rhs);
	static float dot(vec3 const& lhs, vec3 const& rhs);

	vec3 normalize();
};

constexpr vec3 operator+(vec3 const& v);
constexpr vec3 operator-(vec3 const& v);
constexpr bool operator==(vec3 const& lhs, vec3 const& rhs);
constexpr bool operator!=(vec3 const& lhs, vec3 const& rhs);
constexpr vec3 operator+(vec3 const& lhs, vec3 const& rhs);
constexpr vec3 operator-(vec3 const& lhs, vec3 const& rhs);
constexpr vec3 operator*(vec3 const& lhs, vec3 const& rhs);

constexpr vec3 operator*(vec3 const& lhs, float const& rhs);

#include "vec3.inl"

#endif // VEC3_H_
