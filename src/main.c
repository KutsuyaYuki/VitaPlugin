#include <psp2/display.h>
#include <psp2/kernel/processmgr.h>

#include <vita2d.h>

#include "font.h"

vita2d_pgf *font;

#define SCREEN_WIDTH 960
#define SCREEN_HEIGHT 544

#define TITLE_BAR_HEIGHT 30
#define CONTENT_HEIGHT (SCREEN_HEIGHT - TITLE_BAR_HEIGHT)

void mainloop()
{
    vita2d_start_drawing();
    vita2d_clear_screen();
    
    // Draw title bar
    vita2d_draw_rectangle(0, 0, SCREEN_WIDTH, TITLE_BAR_HEIGHT, RGBA8(0x00, 0x00, 0xFF, 0xFF));
    font_draw_string(10, 5, RGBA8(0xFF, 0xFF, 0xFF, 0xFF), "Yuki's First Vita Application");
    
    // Draw content area
    vita2d_draw_rectangle(0, TITLE_BAR_HEIGHT, SCREEN_WIDTH, CONTENT_HEIGHT, RGBA8(0x00, 0x00, 0x00, 0xFF));
    font_draw_string(10, TITLE_BAR_HEIGHT + 10, RGBA8(0xFF, 0xFF, 0xFF, 0xFF), "Content Area");
    
    vita2d_end_drawing();
    vita2d_swap_buffers();
}

int main()
{
    vita2d_init();
    font = vita2d_load_default_pgf();

    while (1) {
        mainloop();
    }

    vita2d_free_pgf(font);
    vita2d_fini();
    sceKernelExitProcess(0);

    return 0;
}
