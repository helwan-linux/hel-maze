/*
 * SMA CODING - Helwan Linux Maze (Ultimate Edition V1.0 Extended)
 * Developer: Saeed Badreldin
 * OS: Helwan Linux
 * Features: Dual AI, Bomb System (Lvl 8+), BFS Smart AI, Traps, Fog of War, Map Revealer.
 */

#include <gtk/gtk.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>


#define TILE_SIZE 30
#define ROWS 21 
#define COLS 21
#define SCORE_FILE ".helwan_maze_score"

typedef enum { 
    STATE_PLAYING, 
    STATE_WIN, 
    STATE_LOSE 
} GameStatus;

typedef struct { 
    int x; 
    int y; 
} Point;

typedef struct {
    int px, py;               // Player Position
    int ex, ey;               // Red Enemy Position
    int sx, sy;               // Shadow Enemy Position (Lvl 5+)
    int maze[ROWS][COLS];      
    int time_left;            
    int score;                
    int high_score;           
    int level;
    int bombs_count;          // Available Bombs
    int bx, by;               // Active Bomb Position
    int b_active;             // Bomb State (bool)
    int b_timer;              // Explosion Countdown
    int reveal_timer;         // Map Reveal Timer
    int is_revealed;          // Map Reveal State
    int fog_delay;            // عداد تأخير الضباب (3 ثواني) - مضاف جديد
    GameStatus status;        
    GtkWidget *window;
    GtkWidget *drawing_area;  
    GtkWidget *status_label;  
    guint timer_id;           
} GameState;

#include "demo.h"

// --- Sound Module (System Integrated) ---
void play_sound(int type) {
    if (type == 1) system("paplay /usr/share/sounds/freedesktop/stereo/button-pressed.oga &");
    else if (type == 2) system("paplay /usr/share/sounds/freedesktop/stereo/dialog-error.oga &");
    else if (type == 3) system("paplay /usr/share/sounds/freedesktop/stereo/complete.oga &");
    else if (type == 4) system("paplay /usr/share/sounds/freedesktop/stereo/power-manually-pulled.oga &");
}

// --- Intelligence Module (Generic BFS Pathfinding) ---
Point get_next_bfs_step_custom(GameState *gs, int start_x, int start_y) {
    int dist[ROWS][COLS];
    Point parent[ROWS][COLS];
    
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            dist[r][c] = -1;
        }
    }

    Point queue[ROWS * COLS];
    int head = 0;
    int tail = 0;

    queue[tail++] = (Point){start_x, start_y};
    dist[start_y][start_x] = 0;

    int dx[] = {0, 0, 1, -1};
    int dy[] = {1, -1, 0, 0};

    while (head < tail) {
        Point curr = queue[head++];
        if (curr.x == gs->px && curr.y == gs->py) break;

        for (int i = 0; i < 4; i++) {
            int nx = curr.x + dx[i];
            int ny = curr.y + dy[i];
            
            if (nx >= 0 && nx < COLS && ny >= 0 && ny < ROWS && 
                gs->maze[ny][nx] != 1 && dist[ny][nx] == -1) {
                dist[ny][nx] = dist[curr.y][curr.x] + 1;
                parent[ny][nx] = curr;
                queue[tail++] = (Point){nx, ny};
            }
        }
    }

    if (dist[gs->py][gs->px] != -1) {
        Point p = (Point){gs->px, gs->py};
        while (parent[p.y][p.x].x != start_x || parent[p.y][p.x].y != start_y) {
            p = parent[p.y][p.x];
        }
        return p;
    }
    return (Point){start_x, start_y};
}

// --- Persistence Logic ---
void load_high_score(GameState *gs) {
    char path[512];
    sprintf(path, "%s/%s", g_get_home_dir(), SCORE_FILE);
    FILE *f = fopen(path, "r");
    if (f) {
        if (fscanf(f, "%d", &gs->high_score) != 1) {
            gs->high_score = 0;
        }
        fclose(f);
    } else {
        gs->high_score = 0;
    }
}

void save_high_score(GameState *gs) {
    if (gs->score > gs->high_score) {
        gs->high_score = gs->score;
        char path[512];
        sprintf(path, "%s/%s", g_get_home_dir(), SCORE_FILE);
        FILE *f = fopen(path, "w");
        if (f) {
            fprintf(f, "%d", gs->high_score);
            fclose(f);
        }
    }
}

// --- Help & About ---
void show_help(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(data),
        GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
        "Game Instructions:\n\n"
        "• Movement: Arrow Keys.\n"
        "• Enemies: Red (Basic) & Orange (Lvl 5+ Master).\n"
        "• Bombs (Lvl 8+): Press [SPACE] to plant. Destroys walls 3x3!\n"
        "• Bonus: Cyan item reveals everything for 10 seconds!");
    gtk_window_set_title(GTK_WINDOW(dialog), "Manual");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

void show_about(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog = gtk_about_dialog_new();
    GdkPixbuf *logo = gdk_pixbuf_new_from_file("helwan-maze.png", NULL);
    gtk_about_dialog_set_logo(GTK_ABOUT_DIALOG(dialog), logo);
    gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(dialog), "Helwan Linux Maze");
    gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dialog), "1.0 Extended");
    gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(dialog), "© 2026 Saeed Badreldin");
    gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(dialog), "Official System Game for Helwan Linux.");
    gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(dialog), "https://helwan-linux.github.io/helwanlinux/");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

// --- UI Updates ---
void update_ui_label(GameState *gs) {
    char info[256];
    char reveal_status[64] = "";
    if (gs->is_revealed) {
        sprintf(reveal_status, " | 👁️ REVEAL: %ds", gs->reveal_timer);
    }
    sprintf(info, " 🕒 %ds | ⭐ %d | 🏆 Best: %d | 🛡️ Lvl: %d | 💣 %d%s", 
            gs->time_left, gs->score, gs->high_score, gs->level, gs->bombs_count, reveal_status);
    gtk_label_set_text(GTK_LABEL(gs->status_label), info);
}

// --- Maze Generation (Recursive DFS) ---
void generate_maze_dfs(GameState *gs, int r, int c) {
    int dirs[4][2] = {{0,2}, {0,-2}, {2,0}, {-2,0}};
    
    for (int i = 0; i < 4; i++) {
        int j = rand() % 4;
        int temp_r = dirs[i][0];
        int temp_c = dirs[i][1];
        dirs[i][0] = dirs[j][0];
        dirs[i][1] = dirs[j][1];
        dirs[j][0] = temp_r;
        dirs[j][1] = temp_c;
    }

    for (int i = 0; i < 4; i++) {
        int nr = r + dirs[i][0];
        int nc = c + dirs[i][1];
        
        if (nr > 0 && nr < ROWS-1 && nc > 0 && nc < COLS-1 && gs->maze[nr][nc] == 1) {
            gs->maze[r + dirs[i][0]/2][c + dirs[i][1]/2] = 0;
            gs->maze[nr][nc] = 0;
            generate_maze_dfs(gs, nr, nc);
        }
    }
}

// --- Game Initialization ---
void init_game(GameState *gs) {
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            gs->maze[r][c] = 1;
        }
    }

    gs->maze[1][1] = 0;
    generate_maze_dfs(gs, 1, 1);
    
    int items = 0;
    while (items < 8) {
        int rr = rand() % ROWS;
        int rc = rand() % COLS;
        if (gs->maze[rr][rc] == 0 && (rr != 1 || rc != 1)) { 
            gs->maze[rr][rc] = (items < 5) ? 3 : 4; 
            items++; 
        }
    }

    // Spawn Revealer Bonus (مضمون 100% في كل دور)
    int br, bc;
    do {
        br = rand() % ROWS;
        bc = rand() % COLS;
    } while (gs->maze[br][bc] != 0);
    gs->maze[br][bc] = 5; 
    
    gs->maze[ROWS-2][COLS-2] = 2; // Exit
    gs->px = 1; 
    gs->py = 1;
    gs->ex = COLS-2; 
    gs->ey = ROWS-2; 
    gs->sx = 1; 
    gs->sy = ROWS-2; 
    gs->b_active = 0;
    gs->is_revealed = 0;
    gs->reveal_timer = 0;
    gs->fog_delay = 3; // تهيئة الـ 3 ثواني تأخير للضباب

    if (gs->level <= 1) {
        gs->level = 1;
        load_high_score(gs);
        gs->bombs_count = 0;
    }
    
    if (gs->level >= 8 && gs->bombs_count < 3) {
        gs->bombs_count = 3;
    }

    gs->time_left = 60 - (gs->level * 2);
    if (gs->time_left < 10) gs->time_left = 10; 
    
    gs->status = STATE_PLAYING;
}

// --- Rendering Module ---
static gboolean on_draw(GtkWidget *widget, cairo_t *cr, gpointer data) {
    GameState *gs = (GameState *)data;
    
    // Background
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_paint(cr);
    
    // Fog of War Mask: يعمل فقط لو انتهى الـ fog_delay ولو مفيش مكافأة كشف مفعلة
    if (gs->fog_delay <= 0 && !gs->is_revealed) {
        cairo_save(cr);
        cairo_arc(cr, gs->px * TILE_SIZE + TILE_SIZE/2, gs->py * TILE_SIZE + TILE_SIZE/2, TILE_SIZE * 3.5, 0, 2 * G_PI);
        cairo_clip(cr); 
    }

    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            if (gs->maze[r][c] == 1) {
                cairo_set_source_rgb(cr, 0.05, 0.05, 0.1); 
            } else if (gs->maze[r][c] == 2) {
                cairo_set_source_rgb(cr, 0.1, 0.9, 0.1); 
            } else if (gs->maze[r][c] == 3) {
                cairo_set_source_rgb(cr, 1.0, 0.8, 0.0); 
            } else if (gs->maze[r][c] == 4) {
                cairo_set_source_rgb(cr, 0.5, 0.0, 0.5); 
            } else if (gs->maze[r][c] == 5) {
                cairo_set_source_rgb(cr, 0.0, 1.0, 1.0); // Cyan Bonus Color
            } else {
                cairo_set_source_rgb(cr, 0.2, 0.2, 0.25); 
            }
            cairo_rectangle(cr, c * TILE_SIZE, r * TILE_SIZE, TILE_SIZE, TILE_SIZE);
            cairo_fill(cr);
        }
    }

    if (gs->fog_delay <= 0 && !gs->is_revealed) {
        cairo_restore(cr);
    }

    // Render Bomb
    if (gs->b_active) {
        cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
        cairo_arc(cr, gs->bx * TILE_SIZE + TILE_SIZE/2, gs->by * TILE_SIZE + TILE_SIZE/2, TILE_SIZE/4, 0, 2 * G_PI);
        cairo_fill(cr);
    }

    // Render Player
    cairo_set_source_rgb(cr, 0.2, 0.6, 1.0); 
    cairo_arc(cr, gs->px * TILE_SIZE + TILE_SIZE/2, gs->py * TILE_SIZE + TILE_SIZE/2, TILE_SIZE/3, 0, 2 * G_PI);
    cairo_fill(cr);

    // Render Red Enemy
    cairo_set_source_rgb(cr, 1.0, 0.2, 0.2); 
    cairo_rectangle(cr, gs->ex * TILE_SIZE + 6, gs->ey * TILE_SIZE + 6, TILE_SIZE - 12, TILE_SIZE - 12);
    cairo_fill(cr);

    // Render Shadow Enemy
    if (gs->level >= 5) {
        cairo_set_source_rgb(cr, 1.0, 0.5, 0.0); 
        cairo_rectangle(cr, gs->sx * TILE_SIZE + 6, gs->sy * TILE_SIZE + 6, TILE_SIZE - 12, TILE_SIZE - 12);
        cairo_fill(cr);
    }

    // Overlay Screens
    if (gs->status != STATE_PLAYING) {
        cairo_set_source_rgba(cr, 0, 0, 0, 0.85);
        cairo_rectangle(cr, 0, 0, COLS * TILE_SIZE, ROWS * TILE_SIZE);
        cairo_fill(cr);
        cairo_set_source_rgb(cr, 1, 1, 1);
        cairo_set_font_size(cr, 28);
        cairo_move_to(cr, 40, ROWS * TILE_SIZE / 2);
        if (gs->status == STATE_WIN) {
            cairo_show_text(cr, "LEVEL CLEAR! [ENTER]");
        } else {
            cairo_show_text(cr, "MISSION FAILED! [ENTER]");
        }
    }
    return FALSE;
}

// --- Input Management ---
static gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer data) {
    GameState *gs = (GameState *)data;
    
    // عند ضغط Escape: يتم إنهاء وضع الديمو وبدء لعبة جديدة تماماً للاعب
    if (event->keyval == GDK_KEY_Escape) {
        if (is_demo_active) {
            is_demo_active = 0;    // إيقاف المحاكي
            gs->level = 1;         // العودة للمستوى الأول
            gs->score = 0;         // تصفير النقاط
            init_game(gs);         // توليد متاهة جديدة وتصفير الأعداء
            gtk_widget_queue_draw(gs->drawing_area); // تحديث الشاشة
        }
        return TRUE; 
    }
    
    if (gs->status != STATE_PLAYING) {
        if (event->keyval == GDK_KEY_Return || event->keyval == GDK_KEY_KP_Enter) {
            if (gs->status == STATE_WIN) { 
                gs->level++; 
                save_high_score(gs); 
                play_sound(3); 
            } else { 
                save_high_score(gs); 
                gs->level = 1; 
                gs->score = 0; 
            }
            init_game(gs);
            gtk_widget_queue_draw(gs->drawing_area);
        }
        return FALSE;
    }

    // Bomb Deployment
    if (event->keyval == GDK_KEY_space && gs->level >= 8 && gs->bombs_count > 0 && !gs->b_active) {
        gs->bx = gs->px; 
        gs->by = gs->py; 
        gs->b_active = 1; 
        gs->b_timer = 3;
        gs->bombs_count--; 
        play_sound(1);
    }

    // Player Movement
    int nx = gs->px;
    int ny = gs->py;
    
    if (event->keyval == GDK_KEY_Up) ny--;
    else if (event->keyval == GDK_KEY_Down) ny++;
    else if (event->keyval == GDK_KEY_Left) nx--;
    else if (event->keyval == GDK_KEY_Right) nx++;

    if (ny >= 0 && ny < ROWS && nx >= 0 && nx < COLS && gs->maze[ny][nx] != 1) {
        if (gs->maze[ny][nx] == 3) {
            gs->score += 10; 
            gs->time_left += 5; 
            gs->maze[ny][nx] = 0;
            play_sound(1); 
            save_high_score(gs);
        } else if (gs->maze[ny][nx] == 4) {
            gs->time_left -= 5; 
            gs->maze[ny][nx] = 0; 
            play_sound(2);
        } else if (gs->maze[ny][nx] == 5) {
            gs->is_revealed = 1;
            gs->reveal_timer = 10;
            gs->maze[ny][nx] = 0;
            play_sound(1);
        }
        
        gs->px = nx; 
        gs->py = ny;
        
        if (gs->maze[ny][nx] == 2) { 
            gs->status = STATE_WIN; 
            save_high_score(gs); 
            play_sound(3); 
        }
        
        // Final Collision Check
        if ((gs->px == gs->ex && gs->py == gs->ey) || 
            (gs->level >= 5 && gs->px == gs->sx && gs->py == gs->sy)) {
            gs->status = STATE_LOSE; 
            save_high_score(gs); 
            play_sound(2);
        }
        gtk_widget_queue_draw(gs->drawing_area);
    }
    return TRUE;
}

// --- Main Loop Logic ---
static gboolean update_game_logic(gpointer data) {
    GameState *gs = (GameState *)data;
    if (gs->status != STATE_PLAYING) return TRUE;
    
    if (is_demo_active) run_demo_logic(gs);

    // Time Management
    if (--gs->time_left <= 0) { 
        gs->status = STATE_LOSE; 
        save_high_score(gs); 
        play_sound(2); 
    }

    // Fog Delay (New Logic for 3s delay)
    if (gs->fog_delay > 0) {
        gs->fog_delay--;
    }

    // Map Reveal Timer
    if (gs->is_revealed && --gs->reveal_timer <= 0) {
        gs->is_revealed = 0;
    }

    // Bomb Management
    if (gs->b_active && --gs->b_timer <= 0) {
        play_sound(4);
        for (int r = gs->by - 1; r <= gs->by + 1; r++) {
            for (int c = gs->bx - 1; c <= gs->bx + 1; c++) {
                if (r > 0 && r < ROWS - 1 && c > 0 && c < COLS - 1) {
                    gs->maze[r][c] = 0; // Clear walls
                }
            }
        }
        gs->b_active = 0;
    }

    // Difficulty Factor
    int chance = 45 + (gs->level * 7); 
    if (chance > 88) chance = 88; 

    // Red Enemy AI
    if ((rand() % 100) < chance) {
        Point next = get_next_bfs_step_custom(gs, gs->ex, gs->ey);
        gs->ex = next.x; 
        gs->ey = next.y;
    }

    // Shadow Enemy AI (Lvl 5+)
    if (gs->level >= 5 && (rand() % 100) < (chance - 10)) {
        Point next = get_next_bfs_step_custom(gs, gs->sx, gs->sy);
        gs->sx = next.x; 
        gs->sy = next.y;
    }

    // Collision Check after Move
    if ((gs->px == gs->ex && gs->py == gs->ey) || 
        (gs->level >= 5 && gs->px == gs->sx && gs->py == gs->sy)) {
        gs->status = STATE_LOSE; 
        save_high_score(gs); 
        play_sound(2);
    }

    update_ui_label(gs);
    gtk_widget_queue_draw(gs->drawing_area);
    return TRUE;
}

// --- System Entry Point ---
int main(int argc, char *argv[]) {
    srand(time(NULL));
    gtk_init(&argc, &argv);

    GameState *gs = g_new0(GameState, 1);
    init_game(gs);

    gs->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_icon_from_file(GTK_WINDOW(gs->window), "helwan-maze.png", NULL);
    gtk_window_set_title(GTK_WINDOW(gs->window), "Helwan Linux Maze - Ultimate 1.0");
    gtk_window_set_resizable(GTK_WINDOW(gs->window), FALSE);
    g_signal_connect(gs->window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(gs->window), vbox);

    // Menu System
    GtkWidget *menubar = gtk_menu_bar_new();
    GtkWidget *opt_menu_item = gtk_menu_item_new_with_label("Options");
    GtkWidget *opt_menu = gtk_menu_new();
    GtkWidget *help_item = gtk_menu_item_new_with_label("How to Play");
    GtkWidget *about_item = gtk_menu_item_new_with_label("About");

    g_signal_connect(help_item, "activate", G_CALLBACK(show_help), gs->window);
    g_signal_connect(about_item, "activate", G_CALLBACK(show_about), gs->window);

    gtk_menu_shell_append(GTK_MENU_SHELL(opt_menu), help_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(opt_menu), about_item);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(opt_menu_item), opt_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), opt_menu_item);
    gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, FALSE, 0);

    // Status UI
    gs->status_label = gtk_label_new("");
    update_ui_label(gs);
    gtk_box_pack_start(GTK_BOX(vbox), gs->status_label, FALSE, FALSE, 10);
	
	// --- Add Demo Button below status_label ---
	GtkWidget *demo_btn = gtk_button_new_with_label("Start Auto Demo");
	g_signal_connect(demo_btn, "clicked", G_CALLBACK(on_demo_button_clicked), gs);
	gtk_box_pack_start(GTK_BOX(vbox), demo_btn, FALSE, FALSE, 5);


    // Drawing Canvas
    gs->drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(gs->drawing_area, COLS * TILE_SIZE, ROWS * TILE_SIZE);
    gtk_box_pack_start(GTK_BOX(vbox), gs->drawing_area, TRUE, TRUE, 0);

    // Signal Connections
    g_signal_connect(gs->drawing_area, "draw", G_CALLBACK(on_draw), gs);
    g_signal_connect(gs->window, "key-press-event", G_CALLBACK(on_key_press), gs);

    // Game Timer
    g_timeout_add_seconds(1, update_game_logic, gs);

    gtk_widget_show_all(gs->window);
    gtk_main();

    // Resource Cleanup
    save_high_score(gs);
    g_free(gs);
    return 0;
}
