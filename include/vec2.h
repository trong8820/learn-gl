#ifndef VEC2_H_
#define VEC2_H_

struct vec2
{
	float x{};
	float y{};

	// -- Implicit basic constructors --
	constexpr vec2() = default;
	constexpr vec2(vec2 const& v) = default;

	// -- Explicit basic constructors --
	constexpr vec2(float x, float y);

	static float dot(vec2 const& lhs, vec2 const& rhs);

	vec2 normalize() const;
};

constexpr vec2 operator+(vec2 const& v);
constexpr vec2 operator-(vec2 const& v);
constexpr bool operator==(vec2 const& lhs, vec2 const& rhs);
constexpr bool operator!=(vec2 const& lhs, vec2 const& rhs);
constexpr vec2 operator+(vec2 const& lhs, vec2 const& rhs);
constexpr vec2 operator-(vec2 const& lhs, vec2 const& rhs);
constexpr vec2 operator*(vec2 const& lhs, vec2 const& rhs);

constexpr vec2 operator*(vec2 const& lhs, float const& rhs);

#include "vec2.inl"

#endif // VEC2_H_
