#include "../include/gos.h"
#include <GLFW/glfw3.h>

static int brush_size = 4;
static int brush_kind = kind_sand;
static struct input_globals *input;

void on_scroll(){
    if (input->key[GLFW_KEY_SPACE]){
        brush_size = max(brush_size + input->scroll*3, 0);
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
    case GLFW_KEY_W:
    case GLFW_KEY_A:
    case GLFW_KEY_S:
    case GLFW_KEY_D:
        {
            vec2 *cam = render_camera();
            cam->x += ((key == GLFW_KEY_A) - (key == GLFW_KEY_D)) * 15;
            cam->y -= ((key == GLFW_KEY_W) - (key == GLFW_KEY_S)) * 15;
        }
        break;
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
    return;
}

void paint_at(struct universe *u, struct uvec2 *pos, 
              uint16_t kind, uint16_t flags){
    vec2 chunk_pos = { .x = pos->x / (CHUNK_SIZE*POINT_SIZE),
                       .y = pos->y / (CHUNK_SIZE*POINT_SIZE)};
    struct chunk *c = {0};
    for (size_t i = 0; i < u->num_chunks; i++){
        if (u->grid[i].pos.x == chunk_pos.x &&
            u->grid[i].pos.y == chunk_pos.y){
            c = &u->grid[i];
            break;
        }
    }
    if (!c){return;} 
    for (int y = 0; y < brush_size; y++){
        for (int x = 0; x < brush_size; x++){
            struct vec2 draw_pos = {
                .x = (int)(pos->x%(CHUNK_SIZE*POINT_SIZE))/2 + x - brush_size/2,
                .y = (int)(pos->y%(CHUNK_SIZE*POINT_SIZE))/2 + y - brush_size/2,
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
            chunk_set_cell(c, id, cell_data);
        }
    }
}

int main() {
    printf("Chunk bytes: %lu, Cell bytes: %lu\n", 
            sizeof(struct chunk), sizeof(struct cell));
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

    struct universe u = {0};
    u.grid = calloc(NUM_SCREEN_CHUNKS, sizeof(struct chunk));
    if (!u.grid){ return 1; }

    for (int i = 0; i < NUM_SCREEN_CHUNKS; ++i){
        vec2 pos = { .x = i % SCREEN_CHUNKS_X,
                     .y = i / SCREEN_CHUNKS_X };
        universe_spawn_chunk(&u, pos);
    }
    SET_FLAG(u.grid[0].flags, CHUNK_ACTIVE);

    size_t fps = 0;
    double last_fps_post = 0.0;
    double frame_since_sim = 0.0;
    while (!glfwWindowShouldClose(window)){
        double update_start = glfwGetTime();
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
            memset(u.grid, 0, sizeof(struct chunk)*NUM_SCREEN_CHUNKS);
            SET_FLAG(u.grid[0].flags, CHUNK_ACTIVE);
        }else if(input->key[GLFW_KEY_R] == GLFW_PRESS){
            SET_FLAG(u.grid[0].flags, CHUNK_ACTIVE);
        }
        
        if (frame_since_sim >= 1.0/SIM_SPEED){
            frame_since_sim = 0.0;
                double start = glfwGetTime(); 
            for (int i = 0; i < NUM_SCREEN_CHUNKS; i++){
                if (CHK_FLAG(u.grid[i].flags, CHUNK_ACTIVE)){
                    chunk_update(&u.grid[i]);
                }
            }
            if (fps==1){
                printf("Sim time per frame: %fms\n", 
                        (glfwGetTime()-start) * 1000);
            }

            if (input->mouse_btns[GLFW_MOUSE_BUTTON_1] == GLFW_PRESS){
                for (int i = 0; i < NUM_SCREEN_CHUNKS; i++){
                    paint_at(&u, &input->mouse_pos, 
                             brush_kind, 0); 
                }
            }
            if (input->mouse_btns[GLFW_MOUSE_BUTTON_2] == GLFW_PRESS){
                for (int i = 0; i < NUM_SCREEN_CHUNKS; i++){
                    paint_at(&u, &input->mouse_pos, 
                             kind_stone, CELL_STATIC);
                }

            }
            render_update_chunks(&u);
        }

        glClear(GL_COLOR_BUFFER_BIT);
       
        render_draw();
        if (!fps){
            printf("Update time: %fms\n", 
                    (glfwGetTime()-update_start) * 1000);
        }
        glfwSwapBuffers(window);
        


        glfwPollEvents();
    }
    
    gfx_terminate(&window);
    return 0;
}

