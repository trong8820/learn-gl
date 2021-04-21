#ifndef ENTRY_H__
#define ENTRY_H__

#include <iostream>
#include <string>

#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

int gWidth{ 0 };
int gHeight{ 0 };

int run();

bool init();
void update();
void draw();

#endif // ENTRY_H__

