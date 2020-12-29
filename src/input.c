#include "../include/input.h"

static struct input_globals input;
struct input_globals *get_input_globals(){
    return &input;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
void input_key_callback(GLFWwindow *window, int key, 
                        int scancode, int action, int mods){
    
    if (action == GLFW_PRESS){
        if (key == GLFW_KEY_ESCAPE){
            glfwSetWindowShouldClose(window, 1);
            return;
        }
    }

    input.key[key] = action;
}

void input_cursor_pos_callback(GLFWwindow *window, double xpos, double ypos){
    input.mouse_pos.x = (uint16_t) xpos;
    input.mouse_pos.y = (uint16_t) CHUNK_SIZE-ypos;
}

void input_mouse_button_callback(GLFWwindow *window, int button, 
                                 int action, int mods){
    if (button > 2) {return;}
    input.mouse_btns[button] = action;
}

#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#pragma GCC diagnostic pop


