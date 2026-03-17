/*
 * SMA CODING - Helwan Linux Maze (Full Pro Edition)
 * شرح كافة التفاصيل للمبتدئين فوق كل سطر
 */

#include <gtk/gtk.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

// حجم المربع الواحد بالبكسل
#define TILE_SIZE 30
// عدد صفوف المتاهة
#define ROWS 21 
// عدد أعمدة المتاهة
#define COLS 21

// تعريف حالات اللعبة لسهولة التحكم في المنطق البرمجي
typedef enum { STATE_PLAYING, STATE_WIN, STATE_LOSE } GameStatus;

// الهيكل اللي بيشيل كل أسرار اللعبة
typedef struct {
    int px, py;               // إحداثيات اللاعب
    int maze[ROWS][COLS];      // مصفوفة الخريطة
    int time_left;            // العداد الزمني
    int score;                // عداد النقاط
    GameStatus status;        // الحالة الحالية
    GtkWidget *drawing_area;  // منطقة الرسم
    GtkWidget *status_label;  // نص العدادات العلوي
    guint timer_id;           // رقم العداد
} GameState;

// دالة تحديث شكل العداد (الوقت والسكور)
void update_ui_label(GameState *gs) {
    char info[100];
    sprintf(info, " 🕒 Time: %d s | ⭐ Score: %d ", gs->time_left, gs->score);
    gtk_label_set_text(GTK_LABEL(gs->status_label), info);
}

// حفر المتاهة بطريقة DFS لضمان الحل
void generate_maze_dfs(GameState *gs, int r, int c) {
    int dirs[4][2] = {{0,2}, {0,-2}, {2,0}, {-2,0}};
    for (int i = 0; i < 4; i++) {
        int j = rand() % 4;
        int temp[2] = {dirs[i][0], dirs[i][1]};
        dirs[i][0] = dirs[j][0]; dirs[i][1] = dirs[j][1];
        dirs[j][0] = temp[0]; dirs[j][1] = temp[1];
    }
    for (int i = 0; i < 4; i++) {
        int nr = r + dirs[i][0], nc = c + dirs[i][1];
        if (nr > 0 && nr < ROWS-1 && nc > 0 && nc < COLS-1 && gs->maze[nr][nc] == 1) {
            gs->maze[r + dirs[i][0]/2][c + dirs[i][1]/2] = 0;
            gs->maze[nr][nc] = 0;
            generate_maze_dfs(gs, nr, nc);
        }
    }
}

// تصفير اللعبة وتوزيع الجوائز والهدف
void init_game(GameState *gs) {
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++) gs->maze[r][c] = 1;
    gs->maze[1][1] = 0;
    generate_maze_dfs(gs, 1, 1);
    int coins = 0;
    while(coins < 5) {
        int rr = rand() % ROWS, rc = rand() % COLS;
        if(gs->maze[rr][rc] == 0) { gs->maze[rr][rc] = 3; coins++; }
    }
    gs->maze[ROWS-2][COLS-2] = 2; 
    gs->px = 1; gs->py = 1;
    gs->time_left = 60;
    gs->score = 0;
    gs->status = STATE_PLAYING;
}

// رسم المربعات واللاعب وشاشات النهاية
static gboolean on_draw(GtkWidget *widget, cairo_t *cr, gpointer data) {
    GameState *gs = (GameState *)data;
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            if (gs->maze[r][c] == 1) cairo_set_source_rgb(cr, 0.1, 0.1, 0.1); 
            else if (gs->maze[r][c] == 2) cairo_set_source_rgb(cr, 0.1, 0.8, 0.1); 
            else if (gs->maze[r][c] == 3) cairo_set_source_rgb(cr, 1.0, 0.8, 0.0); 
            else cairo_set_source_rgb(cr, 0.9, 0.9, 0.9); 
            cairo_rectangle(cr, c * TILE_SIZE, r * TILE_SIZE, TILE_SIZE, TILE_SIZE);
            cairo_fill(cr);
        }
    }
    cairo_set_source_rgb(cr, 0.1, 0.5, 0.9);
    cairo_arc(cr, gs->px * TILE_SIZE + TILE_SIZE/2, gs->py * TILE_SIZE + TILE_SIZE/2, TILE_SIZE/3, 0, 2 * G_PI);
    cairo_fill(cr);
    if (gs->status != STATE_PLAYING) {
        cairo_set_source_rgba(cr, 0, 0, 0, 0.8);
        cairo_rectangle(cr, 0, 0, COLS * TILE_SIZE, ROWS * TILE_SIZE);
        cairo_fill(cr);
        cairo_set_source_rgb(cr, 1, 1, 1);
        cairo_set_font_size(cr, 40);
        cairo_move_to(cr, 100, ROWS * TILE_SIZE / 2);
        cairo_show_text(cr, gs->status == STATE_WIN ? "YOU WIN! 🎉" : "GAME OVER 💀");
    }
    return FALSE;
}

// تحريك اللاعب والتقاط الجوائز
static gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer data) {
    GameState *gs = (GameState *)data;
    if (gs->status != STATE_PLAYING) return FALSE;
    int nx = gs->px, ny = gs->py;
    switch (event->keyval) {
        case GDK_KEY_Up: ny--; break;
        case GDK_KEY_Down: ny++; break;
        case GDK_KEY_Left: nx--; break;
        case GDK_KEY_Right: nx++; break;
    }
    if (gs->maze[ny][nx] != 1) {
        if (gs->maze[ny][nx] == 3) {
            gs->score += 10;
            gs->maze[ny][nx] = 0;
            update_ui_label(gs);
        }
        gs->px = nx; gs->py = ny;
        if (gs->maze[ny][nx] == 2) gs->status = STATE_WIN;
        gtk_widget_queue_draw(gs->drawing_area);
    }
    return TRUE;
}

// تحديث العداد كل ثانية
static gboolean update_timer(gpointer data) {
    GameState *gs = (GameState *)data;
    if (gs->status != STATE_PLAYING) return FALSE;
    if (--gs->time_left <= 0) {
        gs->status = STATE_LOSE;
        gtk_widget_queue_draw(gs->drawing_area);
        update_ui_label(gs);
        return FALSE;
    }
    update_ui_label(gs);
    return TRUE;
}

int main(int argc, char *argv[]) {
    srand(time(NULL));
    gtk_init(&argc, &argv);

    GameState *gs = g_new0(GameState, 1);
    init_game(gs);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Helwan Linux Maze (SMA)");

    // --- الجزء الخاص بالأيقونة لتجنب الخطأ ---
    // هيحاول يفتحها من الفولدر الحالي الأول (أسهل لك في التجربة)
    GError *error = NULL;
    if (!gtk_window_set_icon_from_file(GTK_WINDOW(window), "./helwan-maze.png", &error)) {
        // لو فشل، هيحاول يفتحها من مسار السيستم
        g_clear_error(&error);
        gtk_window_set_icon_from_file(GTK_WINDOW(window), "/usr/share/icons/hicolor/512x512/apps/helwan-maze.png", NULL);
    }

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    gs->status_label = gtk_label_new("");
    update_ui_label(gs);
    gtk_box_pack_start(GTK_BOX(vbox), gs->status_label, FALSE, FALSE, 10);

    gs->drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(gs->drawing_area, COLS * TILE_SIZE, ROWS * TILE_SIZE);
    gtk_box_pack_start(GTK_BOX(vbox), gs->drawing_area, TRUE, TRUE, 0);

    g_signal_connect(gs->drawing_area, "draw", G_CALLBACK(on_draw), gs);
    g_signal_connect(window, "key-press-event", G_CALLBACK(on_key_press), gs);

    gs->timer_id = g_timeout_add_seconds(1, update_timer, gs);

    gtk_widget_show_all(window);
    gtk_main();

    g_free(gs);
    return 0;
}
