#include "entry.h"

auto init() -> bool
{
	return true;
}

auto update() -> void
{

}

auto draw() -> void
{
	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(1.0f, 1.0f, 0.2f, 1.0f);
}

auto main() -> int
{
	return run();
}
