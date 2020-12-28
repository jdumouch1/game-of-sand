#include "../include/gos.h"
#include <GLFW/glfw3.h>

int init_gfx(GLFWwindow **window){
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    
    *window = glfwCreateWindow(CHUNK_SIZE, CHUNK_SIZE, 
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
    glViewport(0, 0, CHUNK_SIZE, CHUNK_SIZE);
   
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


    struct uvec2 pos = { .x = CHUNK_SIZE/2, .y = CHUNK_SIZE/2 };
    for (size_t i = 0; i < 10; i++){
        pos.y++;
        u.grid[0].mesh[uvec2_to_id(&pos)].kind = 200;
    }


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
                    if (rand() % 10) { continue; }
                    struct uvec2 pos = input->mouse_pos;
                    pos.x+=x; pos.y+=y;
                    size_t id = uvec2_to_id(&pos);
                    u.grid[0].mesh[id].kind = (rand()+50) % 255;
                }
            }
        }

        if (input->key[GLFW_KEY_Q] == GLFW_PRESS){
            memset(&u.grid[0], 0, sizeof(struct chunk));
        }
        
        if (frame_since_sim >= 1/SIM_SPEED){
            frame_since_sim = 0.0;
            chunk_update(&u.grid[0]);
            renderer_load_chunk(&r, &u.grid[0]);
        }

        glClear(GL_COLOR_BUFFER_BIT);
        renderer_draw(&r);
        glfwSwapBuffers(window);
        
        glfwWaitEventsTimeout(1.0/60.0);
    }
    
    close_glfw(&window);
    return 0;
}
