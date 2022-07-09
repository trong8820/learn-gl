#ifndef RAY3_H_
#define RAY3_H_

#include "vec3.h"

struct ray3
{
	vec3 origin{};
	vec3 direction{};

	// -- Implicit basic constructors --
	constexpr ray3() = default;
	constexpr ray3(ray3 const& v) = default;

	// -- Explicit basic constructors --
	constexpr ray3(const vec3& origin, const vec3& direction);

	vec3 at(float t) const;
};

#include "ray3.inl"

#endif // RAY3_H_
