#pragma once
#include "sim.h"
#include <stdio.h>
#include <GLFW/glfw3.h>

struct input_globals {
    struct uvec2 mouse_pos;
    int mouse_btns[3];
    int key[512];
};

void input_key_callback(GLFWwindow *window, int key, int scancode,
                        int action, int mods);


void input_cursor_pos_callback(GLFWwindow *window, double xpos, double ypos);

void input_mouse_button_callback(GLFWwindow *window, int button,
                                 int action, int mods);

struct input_globals *get_input_globals();
