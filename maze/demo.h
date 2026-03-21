#ifndef DEMO_H
#define DEMO_H

#include <gtk/gtk.h>

// متغير التحكم العالمي
static int is_demo_active = 0;

// ذكاء الكمبيوتر للوصول للمخرج
void run_demo_logic(GameState *gs) {
    if (!is_demo_active || gs->status != STATE_PLAYING) return;

    int target_x = COLS - 2; 
    int target_y = ROWS - 2;

    int dist[ROWS][COLS];
    Point parent[ROWS][COLS];
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) dist[r][c] = -1;
    }

    Point queue[ROWS * COLS];
    int head = 0, tail = 0;
    queue[tail++] = (Point){gs->px, gs->py};
    dist[gs->py][gs->px] = 0;

    int dx[] = {0, 0, 1, -1}, dy[] = {1, -1, 0, 0};
    while (head < tail) {
        Point curr = queue[head++];
        if (curr.x == target_x && curr.y == target_y) break;
        for (int i = 0; i < 4; i++) {
            int nx = curr.x + dx[i], ny = curr.y + dy[i];
            if (nx >= 0 && nx < COLS && ny >= 0 && ny < ROWS && gs->maze[ny][nx] != 1 && dist[ny][nx] == -1) {
                dist[ny][nx] = dist[curr.y][curr.x] + 1;
                parent[ny][nx] = curr;
                queue[tail++] = (Point){nx, ny};
            }
        }
    }

    if (dist[target_y][target_x] != -1) {
        Point p = (Point){target_x, target_y};
        while (parent[p.y][p.x].x != gs->px || parent[p.y][p.x].y != gs->py) {
            p = parent[p.y][p.x];
        }
        gs->px = p.x; 
        gs->py = p.y;
        if (gs->maze[gs->py][gs->px] == 2) {
            gs->status = STATE_WIN;
            is_demo_active = 0;
        }
    }
}

// دالة الزر لتفعيل الديمو
void on_demo_button_clicked(GtkWidget *widget, gpointer data) {
    GameState *gs = (GameState *)data;
    is_demo_active = 1;
    gs->is_revealed = 1; 
    gs->reveal_timer = 100;
    gtk_widget_grab_focus(gs->drawing_area); 
}

#endif
