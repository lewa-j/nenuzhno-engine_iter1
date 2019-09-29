
#pragma once

#define IN_PRESS 0
#define IN_RELEASE 1
#define IN_MOVE 2

#ifdef ANDROID
#define IN_KEY_W 1
#define IN_KEY_S 2
#define IN_KEY_A 3
#define IN_KEY_D 4

#define IN_KEY_PRESS 0
#define IN_KEY_RELEASE 1

#else
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#define IN_KEY_W GLFW_KEY_W
#define IN_KEY_S GLFW_KEY_S
#define IN_KEY_A GLFW_KEY_A
#define IN_KEY_D GLFW_KEY_D

#define IN_KEY_PRESS GLFW_PRESS
#define IN_KEY_RELEASE GLFW_RELEASE
#endif

