#include "../include/gos.h"
#include <GLFW/glfw3.h>

int init_gfx(GLFWwindow **window){
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    
    *window = glfwCreateWindow(CHUNK_SIZE*2, CHUNK_SIZE*2, 
                                    "Game of Sand", NULL, NULL);
    if (!window){
        fprintf( stderr, "Failed to create GLFW window.\n");
        goto err;
    }
    glfwMakeContextCurrent(*window);
    
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)){
        fprintf( stderr, "Failed to init GLAD.\n" );
        goto err;
    }
    // TODO: compile all shaders

    glClearColor(0.2, 0.2, 0.2, 1.0);
    glViewport(0, 0, CHUNK_SIZE*2, CHUNK_SIZE*2);
   
    // Event callbacks
    glfwSetKeyCallback(*window, input_key_callback);
    glfwSetCursorPosCallback(*window, input_cursor_pos_callback);
    glfwSetMouseButtonCallback(*window, input_mouse_button_callback);

    return 1;
err:
    glfwTerminate();
    return -1;
}

void close_glfw(GLFWwindow **window){
    // Free the callbacks
    glfwSetKeyCallback(*window, NULL);
    glfwSetCursorPosCallback(*window, NULL);
    glfwSetMouseButtonCallback(*window, NULL);

    // Free the window
    glfwDestroyWindow(*window);

    glfwTerminate();
}

int main() {
    // Initialize graphics
    GLFWwindow *window;
    if (!init_gfx(&window)){
        return -1;
    }
    
    struct input_globals *input = get_input_globals();

    // Compile and link shaders
    GLuint shader_program;
    shader_program_from_files(&shader_program, "assets/vert.glsl", 
                              "assets/frag.glsl");
    glUseProgram(shader_program);
    struct render_data r;
    renderer_init(&r, shader_program);
     
    struct universe u;
    u.grid = calloc(1, sizeof(struct chunk));
        
    printf("kind_sand: %d, color: %u\n", kind_sand, kinds[kind_sand].color);

    size_t fps = 0;
    double last_fps_post = 0.0;
    double frame_since_sim = 0.0;
    while (!glfwWindowShouldClose(window)){
        fps++;
        frame_since_sim++;
        if (glfwGetTime() - last_fps_post >= 1.0){
            last_fps_post = glfwGetTime();
            printf("[FPS] %lu\n", fps);
            fps = 0;
        }

        if (input->mouse_btns[GLFW_MOUSE_BUTTON_1] == GLFW_PRESS){
            for (int y = 0; y < 9; y++) {
                for (int x = 0; x < 9; x++){
                    struct uvec2 pos = input->mouse_pos;
                    pos.x /= 2; pos.y /= 2;
                    pos.x += x; pos.y += y;
                    size_t id = uvec2_to_id(&pos);
                    u.grid[0].mesh[id].kind = 1;
                    u.grid[0].mesh[id].data = 0;
                }
            }
        }

        if (input->key[GLFW_KEY_Q] == GLFW_PRESS){
            memset(&u.grid[0], 0, sizeof(struct chunk));
        }
        
        if (frame_since_sim >= 1.0/SIM_SPEED){
            frame_since_sim = 0.0;
            chunk_update(&u.grid[0]);
            renderer_load_chunk(&r, &u.grid[0]);
        }

        glClear(GL_COLOR_BUFFER_BIT);
        renderer_draw(&r);
        glfwSwapBuffers(window);
        
        glfwPollEvents();
    }
    
    close_glfw(&window);
    return 0;
}

