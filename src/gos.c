#include "../include/gos.h"

static int brush_size = 15;
static int brush_kind = kind_sand;
static struct input_globals *input;

void on_scroll(){
    if (input->key[GLFW_KEY_SPACE]){
        brush_size = max(brush_size + input->scroll*5, 0);
        printf("Brush size: %d\n", brush_size);
    }
    else {
        const int num_kinds = sizeof(kinds)/sizeof(struct kind_property);
        brush_kind += (int)input->scroll;
        if (brush_kind < 0){
            brush_kind = num_kinds-1;
        }
        else if (brush_kind >= num_kinds){
            brush_kind = 0;
        }

        printf("Brush: %s\n", kinds[brush_kind].name);
    }
}

void on_key(int key, int action){
    if (!(action & (GLFW_PRESS | GLFW_REPEAT))){return;}
    switch (key){
    case GLFW_KEY_LEFT_BRACKET:
        brush_size = max(0, brush_size - 
                (1 + (4*(input->key_modifier&GLFW_MOD_SHIFT))));
        printf("Brush size: %d\n", brush_size);
        break;
    case GLFW_KEY_RIGHT_BRACKET:
        brush_size = max(0, brush_size + 
                (1 + (4*(input->key_modifier&GLFW_MOD_SHIFT))));
        printf("Brush size: %d\n", brush_size);
        break;
    /*case GLFW_KEY_R:
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
        break;*/
    case GLFW_KEY_0:
        brush_kind = 0;
        goto print_brush;
    case GLFW_KEY_1:
        brush_kind = 1;
        goto print_brush;
    case GLFW_KEY_2:
        brush_kind = 2;
        goto print_brush;
    case GLFW_KEY_3:
        brush_kind = 3;
        goto print_brush;
    case GLFW_KEY_4:
        brush_kind = 4;
        goto print_brush;
    default:
        break;
    }

    return;
print_brush:
    printf("Brush kind: %s\n", kinds[brush_kind].name);
}

void paint_at(struct chunk *c, struct uvec2 *pos, 
              uint16_t kind, uint16_t flags){
    for (int y = 0; y < brush_size; y++){
        for (int x = 0; x < brush_size; x++){
            struct vec2 draw_pos = {
                .x = (int)(pos->x/2) + x - brush_size/2,
                .y = (int)(pos->y/2) + y - brush_size/2,
            };
            if (draw_pos.x < 0 || draw_pos.x > CHUNK_SIZE-1
                || draw_pos.y < 0 || draw_pos.y > CHUNK_SIZE-1) { 
                continue; 
            }
            if (rand() % 100 < 50 && !flags) {continue;}
            size_t id = draw_pos.x + (draw_pos.y << CHUNK_SCALE);
            struct cell cell_data = {
                .kind = kind,
                .flags = flags | (rand()%2 && kind==kind_sand ? CELL_ALTERNATE : 0),
                .speed = 6,
            };
            if (rand()%2){
                SET_FLAG(cell_data.flags, CELL_BIAS);
            }
            set_cell(c, id, cell_data);
        }
    }
}

int main() {
    printf("Chunk size: %d, Point size: %d\n", CHUNK_SIZE, POINT_SIZE);
    printf("Displayed chunks: (%d, %d) [total: %d]\n", 
            SCREEN_CHUNKS_X, SCREEN_CHUNKS_Y, NUM_SCREEN_CHUNKS);

    srand(5138008);
    // Initialize graphics
    GLFWwindow *window;
    if (!gfx_init(&window)){
        return 1;
    }
    
    input_add_hook(&on_scroll, hook_scroll);   
    input_add_hook(&on_key, hook_key);
    input = input_get_globals();
    
    // Compile and link shaders

    struct universe u;
    u.grid = calloc(1, sizeof(struct chunk));
    u.grid[0].pos = (vec2) { .x = 0, .y = 0 };
    SET_FLAG(u.grid[0].flags, CHUNK_ACTIVE);

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

        if (input->key[GLFW_KEY_Q] == GLFW_PRESS){
            memset(&u.grid[0], 0, sizeof(struct chunk));
            SET_FLAG(u.grid[0].flags, CHUNK_ACTIVE);
        }else if(input->key[GLFW_KEY_R] == GLFW_PRESS){
            SET_FLAG(u.grid[0].flags, CHUNK_ACTIVE);
        }
        
        if (frame_since_sim >= 1.0/SIM_SPEED){
            frame_since_sim = 0.0;
            if (CHK_FLAG(u.grid[0].flags, CHUNK_ACTIVE)){
                chunk_update(&u.grid[0]);
            }

            if (input->mouse_btns[GLFW_MOUSE_BUTTON_1] == GLFW_PRESS){
                paint_at(&u.grid[0], &input->mouse_pos, 
                         brush_kind, 0); 
            }
            if (input->mouse_btns[GLFW_MOUSE_BUTTON_2] == GLFW_PRESS){
                paint_at(&u.grid[0], &input->mouse_pos, 
                         kind_stone, CELL_STATIC);

            }
            renderer_update_chunks(&u);
        }

        glClear(GL_COLOR_BUFFER_BIT);
       
        renderer_draw(CellRenderer);

        glfwSwapBuffers(window);
        
        glfwPollEvents();
    }
    
    gfx_terminate(&window);
    return 0;
}

