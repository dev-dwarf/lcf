#include "lcf/lcf.h"
#include "lcf/lcf.c"

#include "raylib.h"

#pragma comment(lib, "raylib.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "shell32.lib")

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 450

typedef struct {
    s32 score;
    str display;
} Command;

s32 order_command(const void *a, const void *b) {
    Command *c = (Command *)a;
    Command *d = (Command *)b;
    s32 out = d->score - c->score;
    if (!out) {
        out = c - d;
    }
    return out;
}

int main() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);  
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "test");
    SetTargetFPS(60);

    s32 font_size = 20;
    Font font = LoadFontEx("../dd-1/assets/BerkeleyMono-Regular.ttf", font_size, 0, 0);

    Command command[] = {
        (Command){ .display = strl("File: New")},
        (Command){ .display = strl("File: Open")},
        (Command){ .display = strl("File: Save")},
        (Command){ .display = strl("File: Save As")},
        (Command){ .display = strl("File: Close")},
        (Command){ .display = strl("Edit: Undo")},
        (Command){ .display = strl("Edit: Redo")},
        (Command){ .display = strl("Edit: Cut")},
        (Command){ .display = strl("Edit: Copy")},
        (Command){ .display = strl("Edit: Paste")},
        (Command){ .display = strl("Edit: Delete")},
        (Command){ .display = strl("Edit: Select All")},
        (Command){ .display = strl("Edit: Find")},
        (Command){ .display = strl("Edit: Replace")},
        (Command){ .display = strl("Edit: Go to Line")},
        (Command){ .display = strl("Edit: Indent")},
        (Command){ .display = strl("Edit: Unindent")},
        (Command){ .display = strl("Edit: Comment")},
        (Command){ .display = strl("Edit: Uncomment")},
        (Command){ .display = strl("View: Toggle Sidebar")},
        (Command){ .display = strl("View: Toggle Status Bar")},
        (Command){ .display = strl("View: Toggle Full Screen")},
        (Command){ .display = strl("View: Zoom In")},
        (Command){ .display = strl("View: Zoom Out")},
        (Command){ .display = strl("View: Reset Zoom")},
        (Command){ .display = strl("Navigation: Next Tab")},
        (Command){ .display = strl("Navigation: Previous Tab")},
        (Command){ .display = strl("Navigation: Next Split")},
        (Command){ .display = strl("Navigation: Previous Split")},
        (Command){ .display = strl("Navigation: Toggle Sidebar")},
        (Command){ .display = strl("Navigation: Toggle Outline")},
        (Command){ .display = strl("Navigation: Toggle Explorer")},
        (Command){ .display = strl("Navigation: Toggle Console")},
        (Command){ .display = strl("Navigation: Toggle Terminal")},
        (Command){ .display = strl("Navigation: Toggle Preview")},
        (Command){ .display = strl("Navigation: Toggle Toolbar")},
        (Command){ .display = strl("Navigation: Toggle Minimap")},
        (Command){ .display = strl("Navigation: Go to Definition")},
        (Command){ .display = strl("Navigation: Go Back")},
        (Command){ .display = strl("Navigation: Go Forward")},
        (Command){ .display = strl("Navigation: Toggle Code Fold")},
        (Command){ .display = strl("Navigation: Fold All")},
        (Command){ .display = strl("Navigation: Unfold All")},
        (Command){ .display = strl("Tools: Run")},
        (Command){ .display = strl("Tools: Build")},
        (Command){ .display = strl("Tools: Debug")},
        (Command){ .display = strl("Tools: Format Document")},
        (Command){ .display = strl("Tools: Toggle Terminal")},
        (Command){ .display = strl("Tools: Toggle Output Pane")},
    };
    s32 commands = ARRAY_LENGTH(command);

    char buf[256];
    str search = (str){.str=buf, .len=0};
    s32 search_range = commands;
    s32 last_len = 0;
    s32 timer = 0;
    s32 held = 0;

    while (!WindowShouldClose()) {
        /* Input */
        {
            if (IsKeyDown(KEY_BACKSPACE)) {
                if (--timer < 0 && search.len > 0) {
                    timer = 8 - MIN(held*0.5, 7);
                    search.len--;
                    held++;
                }
            } else {
                timer = 0;
                held = 0;
            }
            
            char c;
            while (c = GetCharPressed()) {
                search.str[search.len++] = c;
            }
            search.str[search.len] = 0;
        }

        /* Fuzzy Find */
        if (last_len != search.len) {
            if (last_len > search.len) {
                search_range = commands, search_range;
            }
            last_len = search.len;
            
            for (s32 j = 0; j < search_range; j++) {
                Command *c = command + j;
                c->score = 0;
                
                s32 run = 0;
                char *p = c->display.str;
                char *q = search.str;
                while (*p && *q) {
                    if (char_lower(*p) == char_lower(*q)) {
                        run = MAX(100, run+100);
                        c->score = CLAMPBOTTOM(c->score, 0);
                        q++;
                    } else {
                        run = MIN(-10, run-10);
                        ++p;
                    }
                    c->score += run;

                    while (*p == ' ') *p++;
                    while (*q == ' ') *q++;
                }

                c->score -= c->display.len;
            }
            
            qsort(command, search_range, sizeof(Command), order_command);

            if (search.len > 3) {
                s32 threshold = MIN(0.4, search.len*0.05)*command[0].score;
                for (s32 j = 0; j < search_range; j++) {
                    if (command[j].score < threshold) {
                        search_range = j;
                        break;
                    }
                }
            }
        }
    
        BeginDrawing();
		ClearBackground(WHITE);

        Vector2 size = MeasureTextEx(font, search.str, font_size, 0);
        size.x = GetScreenWidth() - 75;
		DrawTextEx(font, search.str, (Vector2){(GetScreenWidth()-size.x)*0.5, GetScreenHeight()*0.5}, font_size, 0, BLACK);

        Color c = BLACK;
        for (s32 j = 0; j < search_range; j++) {
            c.a = 0xFF * ((f32)(search_range-j))/search_range;

            char buf[128];
            stbsp_sprintf(buf, "%d: %.*s", command[j].score, command[j].display.len, command[j].display.str);
            DrawTextEx(font, buf, (Vector2){(GetScreenWidth()-size.x)*0.5, GetScreenHeight()*0.5 + (j+1)*size.y}, font_size, 0, c);
        }

        DrawFPS(16, 16);
        EndDrawing();
    }

	
    CloseWindow(); 

    return 0;
}
