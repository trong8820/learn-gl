#ifndef VEC4_H_
#define VEC4_H_

struct vec4
{
	float x{};
	float y{};
	float z{};
	float w{};

	// -- Implicit basic constructors --
	constexpr vec4() = default;
	constexpr vec4(vec4 const& v) = default;

	// -- Explicit basic constructors --
	constexpr vec4(float x, float y, float z, float w);
};

#include "vec4.inl"

#endif // VEC4_H_
