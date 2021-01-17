#include "../../include/gfx/gfx.h"

int gfx_init(GLFWwindow **window){
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    
    *window = glfwCreateWindow(RESOLUTION_X, RESOLUTION_Y, 
                              "Cascadia",
                              FULLSCREEN ? glfwGetPrimaryMonitor() : NULL,
                              NULL);
    if (!window){
        fprintf( stderr, "Failed to create GLFW window.\n");
        goto err;
    }
    glfwMakeContextCurrent(*window);
    
    if (!gladLoadGL(glfwGetProcAddress)){
        fprintf( stderr, "Failed to init GLAD.\n" );
        goto err;
    }

    glClearColor(0.2, 0.2, 0.2, 1.0);
    glPointSize((GLfloat) POINT_SIZE);
    glViewport(0, 0, RESOLUTION_X, RESOLUTION_Y);
    
    // Input callbacks
    const struct input_callbacks *callbacks = input_get_callbacks();
    glfwSetKeyCallback(*window, callbacks->key);
    glfwSetCursorPosCallback(*window, callbacks->cursor_pos);
    glfwSetMouseButtonCallback(*window, callbacks->mouse_button);
    glfwSetScrollCallback(*window, callbacks->input_scroll);
    
    // Initialize renderers
    if (!render_init()){
        fprintf( stderr, "Failed to intialize renderer\n");
        goto err;
    }

    return 1;
err:
    glfwTerminate();
    return 0;
}

void gfx_terminate(GLFWwindow **window){
    // Free the callbacks
    glfwSetKeyCallback(*window, NULL);
    glfwSetCursorPosCallback(*window, NULL);
    glfwSetMouseButtonCallback(*window, NULL);

    // Free the window
    glfwDestroyWindow(*window);

    glfwTerminate();
}

