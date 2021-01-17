#pragma once
#include "sim/sim.h"
#include <stdio.h>
#include <GLFW/glfw3.h>

enum input_hook {
    hook_mouse_btn,
    hook_scroll,
    hook_key,
};

struct input_globals {
    struct uvec2 mouse_pos;
    int mouse_btns[3];
    int key[512];
    int key_modifier;
    int scroll;
};

struct input_callbacks {
    void (*key) (GLFWwindow*, int, int, int, int);
    void (*cursor_pos) (GLFWwindow*, double, double);
    void (*mouse_button) (GLFWwindow*, int, int, int);
    void (*input_scroll) (GLFWwindow*, double, double);
};

const struct input_callbacks *input_get_callbacks();
struct input_globals *input_get_globals();
void input_add_hook(void (*fn)(), enum input_hook);
