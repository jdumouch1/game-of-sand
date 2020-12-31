#include "../include/gos.h"
#include <GLFW/glfw3.h>

static int brush_size = 4;
static int brush_kind = kind_sand;
static struct input_globals *input;
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
    glfwSetScrollCallback(*window, input_scroll_callback);
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

void on_scroll(){
    struct input_globals *input = get_input_globals();
    brush_size += input->scroll * 2;
    brush_size = max(0, brush_size);
    printf("brush_size: %d\n", brush_size);
}

void on_key(int key, int action){
    if (action != GLFW_PRESS){return;}
    switch (key){
    case GLFW_KEY_R:
        {
            enum render_mode rm = renderer_get_mode()+1;
            if (rm >= 4){rm = 0;}
            renderer_set_mode(rm);

            printf("Render mode: ");
            switch (rm){
            case render_kinds:
                printf("default\n");
                break;
            case render_flags:
                printf("flags\n");
                break;
            case render_density:
                printf("density\n");
                break;
            case render_velocity:
                printf("velocity\n");
                break;


            default:
                printf("Invalid.\n");
            }
        }
        break;
    case GLFW_KEY_0:
        brush_kind = kind_air;
        goto print_brush;
    case GLFW_KEY_1:
        brush_kind = kind_sand;
        goto print_brush;
    case GLFW_KEY_2:
        brush_kind = kind_water;
        goto print_brush;
    case GLFW_KEY_3:
        brush_kind = kind_gas;
        goto print_brush;
    default:
        break;
    }

    return;
print_brush:
    printf("Brush kind: %s\n", kinds[brush_kind].name);
}

void paint_at(struct chunk *c, struct uvec2 *pos, 
              uint16_t kind, uint16_t data){
    for (size_t y = 0; y < brush_size; y++){
        for (size_t x = 0; x < brush_size; x++){
            struct vec2 draw_pos = {
                .x = (int)(pos->x/2) + x - brush_size/2,
                .y = (int)(pos->y/2) + y - brush_size/2,
            };
            if (draw_pos.x < 0 || draw_pos.x > CHUNK_SIZE-1
                || draw_pos.y < 0 || draw_pos.y > CHUNK_SIZE-1) { 
                continue; 
            }
            size_t id = draw_pos.x + (draw_pos.y << CHUNK_SCALE);
            set_cell(c, id, kind, data);
        }
    }
}

int main() {
    srand(5138008);
    // Initialize graphics
    GLFWwindow *window;
    if (!init_gfx(&window)){
        return -1;
    }
    
    input = get_input_globals();
    input_add_hook(&on_scroll, hook_scroll);   
    input_add_hook(&on_key, hook_key);
    // Compile and link shaders
    GLuint shader_program;
    shader_program_from_files(&shader_program, "assets/vert.glsl", 
                              "assets/frag.glsl");
    glUseProgram(shader_program);
    struct render_data r;
    renderer_init(&r, shader_program);

    struct universe u;
    u.grid = calloc(1, sizeof(struct chunk));
    u.grid[0].flags = 1; 
    printf("kind_sand: %d, color: %u\n", kind_sand, kinds[kind_sand].color);

    int access_order[CHUNK_AREA];
    for (size_t i = 0; i < CHUNK_AREA; i++){
        access_order[i] = i;
    }
    shuffle(access_order, CHUNK_AREA);
    
    size_t fps = 0;
    double last_fps_post = 0.0;
    double frame_since_sim = 0.0;
    while (!glfwWindowShouldClose(window)){
        fps++;
        frame_since_sim++;
        if (glfwGetTime() - last_fps_post >= 1.0){
            last_fps_post = glfwGetTime();
            //printf("[FPS] %lu\n", fps);
            fps = 0;
        }
        
        struct uvec2 pos = input->mouse_pos;
        pos.x/=2; pos.y/=2;

        if (input->mouse_btns[GLFW_MOUSE_BUTTON_1] == GLFW_PRESS){
            paint_at(&u.grid[0], &input->mouse_pos, brush_kind, 0);   
        }
        if (input->mouse_btns[GLFW_MOUSE_BUTTON_2] == GLFW_PRESS){
            paint_at(&u.grid[0], &input->mouse_pos, 
                     brush_kind, CELL_STATIC_FLAG);

        }


        if (input->key[GLFW_KEY_Q] == GLFW_PRESS){
            memset(&u.grid[0], 0, sizeof(struct chunk));
            u.grid[0].flags = 1;
        }else if(input->key[GLFW_KEY_R] == GLFW_PRESS){
            u.grid[0].flags = 1;
        }
        
        if (frame_since_sim >= 1.0/SIM_SPEED){
            frame_since_sim = 0.0;
            if (u.grid[0].flags){
                shuffle(access_order, CHUNK_SIZE);
                chunk_update(&u.grid[0], access_order);
                renderer_load_chunk(&r, &u.grid[0]);
            }
        }

        glClear(GL_COLOR_BUFFER_BIT);
        renderer_draw(&r);
        glfwSwapBuffers(window);
        
        glfwPollEvents();
    }
    
    close_glfw(&window);
    return 0;
}

