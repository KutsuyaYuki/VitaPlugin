#include <psp2/display.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/ctrl.h>
#include <vita2d.h>
#include "font.h"

vita2d_pgf *font;

#define SCREEN_WIDTH 960
#define SCREEN_HEIGHT 544

#define TITLE_BAR_HEIGHT 30
#define CONTENT_HEIGHT (SCREEN_HEIGHT - TITLE_BAR_HEIGHT)

#define ITEM_HEIGHT 40

#define ITEMS_PER_PAGE (CONTENT_HEIGHT / ITEM_HEIGHT)

#define DPAD_DELAY 50 // Delay for D-pad scrolling in milliseconds

int selected_item = 0;
int current_page = 0;
int total_items = 20;

int was_holding_left = 0;
int was_holding_right = 0;

void mainloop()
{
    vita2d_start_drawing();
    vita2d_clear_screen();

    // Draw title bar
    vita2d_draw_rectangle(0, 0, SCREEN_WIDTH, TITLE_BAR_HEIGHT, RGBA8(0x00, 0x00, 0xFF, 0xFF));
    font_draw_string(10, 5, RGBA8(0xFF, 0xFF, 0xFF, 0xFF), "Yuki's First Vita Application");

    // Draw content area
    vita2d_draw_rectangle(0, TITLE_BAR_HEIGHT, SCREEN_WIDTH, CONTENT_HEIGHT, RGBA8(0x00, 0x00, 0x00, 0xFF));

    // Draw list items for the current page
    int start_item = current_page * ITEMS_PER_PAGE;
    int end_item = start_item + ITEMS_PER_PAGE;
    if (end_item > total_items)
    {
        end_item = total_items;
    }

    for (int i = start_item; i < end_item; i++)
    {
        int index_on_page = i - start_item;
        int y = TITLE_BAR_HEIGHT + index_on_page * ITEM_HEIGHT;

        if (i == selected_item)
        {
            vita2d_draw_rectangle(0, y, SCREEN_WIDTH, ITEM_HEIGHT, RGBA8(0xFF, 0xFF, 0xFF, 0xFF));
            char item_text[32];
            sprintf(item_text, "Item %d (Selected)", i);
            font_draw_string(10, y + 10, RGBA8(0x00, 0x00, 0x00, 0xFF), item_text);
        }
        else
        {
            char item_text[16];
            sprintf(item_text, "Item %d", i);
            font_draw_string(10, y + 10, RGBA8(0xFF, 0xFF, 0xFF, 0xFF), item_text);
        }
    }

    vita2d_end_drawing();
    vita2d_swap_buffers();
}

int main() {
    vita2d_init();
    font = vita2d_load_default_pgf();

    SceCtrlData pad;
    sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG_WIDE);

    while (1) {
        sceCtrlPeekBufferPositive(0, &pad, 1);

        // Check for left analog stick input
        if (pad.ly < 128 - 40) { // Analog stick up
            selected_item--;
            if (selected_item < 0) {
                selected_item = total_items - 1;
            }
            current_page = selected_item / ITEMS_PER_PAGE;
        }
        else if (pad.ly > 128 + 40) { // Analog stick down
            selected_item++;
            if (selected_item >= total_items) {
                selected_item = 0;
            }
            current_page = selected_item / ITEMS_PER_PAGE;
        }

        if (pad.buttons & SCE_CTRL_DOWN) {
            selected_item++;
            if (selected_item >= total_items) {
                selected_item = 0;
            }
            current_page = selected_item / ITEMS_PER_PAGE;
            sceKernelDelayThread(DPAD_DELAY * 1000); // Apply 50ms delay for D-pad down
        }
        else if (pad.buttons & SCE_CTRL_UP) {
            selected_item--;
            if (selected_item < 0) {
                selected_item = total_items - 1;
            }
            current_page = selected_item / ITEMS_PER_PAGE;
            sceKernelDelayThread(DPAD_DELAY * 1000); // Apply 50ms delay for D-pad up
        }
// Handling D-pad left
        if ((pad.buttons & SCE_CTRL_LEFT) && !was_holding_left) {
            current_page--;
            if (current_page < 0) {
                current_page = (total_items - 1) / ITEMS_PER_PAGE;
            }
            selected_item = current_page * ITEMS_PER_PAGE;
            was_holding_left = 1;
        }
        
        // Handling D-pad right
        if ((pad.buttons & SCE_CTRL_RIGHT) && !was_holding_right) {
            current_page++;
            if (current_page * ITEMS_PER_PAGE >= total_items) {
                current_page = 0;
            }
            selected_item = current_page * ITEMS_PER_PAGE;
            was_holding_right = 1;
        }

        if (!(pad.buttons & SCE_CTRL_LEFT)) {
            was_holding_left = 0;
        }

        if (!(pad.buttons & SCE_CTRL_RIGHT)) {
            was_holding_right = 0;
        }

        mainloop();
    }

    vita2d_free_pgf(font);
    vita2d_fini();
    sceKernelExitProcess(0);

    return 0;
}