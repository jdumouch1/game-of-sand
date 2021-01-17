#include "../include/input.h"

struct input_hooks {
    void (**mouse_btn_hook)(void*, void*);
    int num_mb_hooks;

    void (**scroll_hook)(void*, void*);
    int num_scroll_hooks;

    void (**key_hook)(int, int);
    int num_key_hooks;
};

static struct input_globals globals;
static struct input_hooks input_hooks = {
    .mouse_btn_hook = NULL,
    .num_mb_hooks = 0,

    .scroll_hook = NULL,
    .num_scroll_hooks = 0,

    .key_hook = NULL,
    .num_key_hooks = 0,
};


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

    globals.key_modifier = mods;
    globals.key[key] = action;

    for (int i = 0; i < input_hooks.num_key_hooks; i++){
        input_hooks.key_hook[i](key, action);
    }
}

void input_cursor_pos_callback(GLFWwindow *window, double xpos, double ypos){
    globals.mouse_pos.x = (uint16_t) xpos; 
    globals.mouse_pos.y = (uint16_t) (((float) RESOLUTION_Y) - ypos);
}

void input_mouse_button_callback(GLFWwindow *window, int button, 
                                 int action, int mods){
    if (button > 2) {return;}
    globals.mouse_btns[button] = action;

    for (int i = 0; i < input_hooks.num_mb_hooks; i++){
        input_hooks.mouse_btn_hook[i]((void*)0, (void*)0);
    }
}

void input_scroll_callback(GLFWwindow *window, double xoffset, double yoffset){
    globals.scroll = (int) yoffset;
    for (int i = 0; i < input_hooks.num_scroll_hooks; i++){
        input_hooks.scroll_hook[i]((void*)0, (void*)0);
    }
}

static const struct input_callbacks callbacks = {
    .key = &input_key_callback,
    .cursor_pos = &input_cursor_pos_callback,
    .mouse_button = &input_mouse_button_callback,
    .input_scroll = &input_scroll_callback,
};

struct input_globals *input_get_globals(){
    return &globals;
}
const struct input_callbacks *input_get_callbacks(){
    return &callbacks;
}
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#pragma GCC diagnostic pop

void input_add_hook(void (*fn)(void*, void*), enum input_hook hook_type){
    int *count;
    void (***hooks)(void*, void*);

    switch (hook_type){
        case hook_mouse_btn:
            count = &input_hooks.num_mb_hooks;
            hooks = &input_hooks.mouse_btn_hook;
            break;
        case hook_scroll:
            count = &input_hooks.num_scroll_hooks;
            hooks = &input_hooks.scroll_hook;
            break;
        case hook_key:
            count = &input_hooks.num_key_hooks;
            hooks = &input_hooks.key_hook;
            break;
        default:
            fprintf(stderr, "Invalid hook_type provided, no action taken.\n");
            return;
    }

    void (**funcs)(void*, void*) = calloc((*count)+1, sizeof(fn));
    if (count){
        memcpy(funcs, *hooks, sizeof(fn)* (*count));
        free(*hooks);
    }
    *hooks = funcs;
    *hooks[*count] = fn;
    *count += 1;


}


