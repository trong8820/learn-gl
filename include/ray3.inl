#include "ray3.h"

#include <math.h>

inline constexpr ray3::ray3(const vec3& origin, const vec3& direction) :
	origin(origin), direction(direction)
{
}

inline vec3 ray3::at(float t) const
{
	return origin + direction*t;
}
