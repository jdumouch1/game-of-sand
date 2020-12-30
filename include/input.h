#pragma once
#include "sim.h"
#include <stdio.h>
#include <GLFW/glfw3.h>

enum input_hook {
    hook_mouse_btn,
    hook_scroll,
};

struct input_globals {
    struct uvec2 mouse_pos;
    int mouse_btns[3];
    int key[512];
    int scroll;
};


struct input_globals *get_input_globals();


void input_key_callback(GLFWwindow *window, int key, int scancode,
                        int action, int mods);

void input_cursor_pos_callback(GLFWwindow *window, double xpos, double ypos);

void input_mouse_button_callback(GLFWwindow *window, int button,
                                 int action, int mods);
void input_scroll_callback(GLFWwindow *window, double xoffset, double yoffset);



void input_add_hook(void (*fn)(), enum input_hook);
