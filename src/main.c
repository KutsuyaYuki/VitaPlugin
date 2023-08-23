#include <psp2/display.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/threadmgr.h> // Include for delay function
#include <psp2/ctrl.h> // Add this for controller input

#include <vita2d.h>

#include "font.h"

vita2d_pgf *font;

#define SCREEN_WIDTH 960
#define SCREEN_HEIGHT 544

#define TITLE_BAR_HEIGHT 30
#define CONTENT_HEIGHT (SCREEN_HEIGHT - TITLE_BAR_HEIGHT)

#define ITEM_HEIGHT 40 // Height of each list item

int selected_item = 0; // Index of the currently selected item
int delay_counter = 0; // Counter to track delay
int initial_delay = 200; // Initial delay in ms
int normal_delay = 50; // Normal delay in ms

void mainloop()
{
    vita2d_start_drawing();
    vita2d_clear_screen();
    
    // Draw title bar
    vita2d_draw_rectangle(0, 0, SCREEN_WIDTH, TITLE_BAR_HEIGHT, RGBA8(0x00, 0x00, 0xFF, 0xFF));
    font_draw_string(10, 5, RGBA8(0xFF, 0xFF, 0xFF, 0xFF), "Yuki's First Vita Application");
    
    // Draw content area
    vita2d_draw_rectangle(0, TITLE_BAR_HEIGHT, SCREEN_WIDTH, CONTENT_HEIGHT, RGBA8(0x00, 0x00, 0x00, 0xFF));
    
    // Draw list items
    int num_items = CONTENT_HEIGHT / ITEM_HEIGHT;
    for (int i = 0; i < num_items; i++) {
        int y = TITLE_BAR_HEIGHT + i * ITEM_HEIGHT;
        if (i == selected_item) {
            vita2d_draw_rectangle(0, y, SCREEN_WIDTH, ITEM_HEIGHT, RGBA8(0xFF, 0xFF, 0xFF, 0xFF));
            char item_text[32]; // Buffer for formatted string
            sprintf(item_text, "Item %d (Selected)", i);
            font_draw_string(10, y + 10, RGBA8(0x00, 0x00, 0x00, 0xFF), item_text);
        } else {
            char item_text[16]; // Buffer for formatted string
            sprintf(item_text, "Item %d", i);
            font_draw_string(10, y + 10, RGBA8(0xFF, 0xFF, 0xFF, 0xFF), item_text);
        }
    }
    
    vita2d_end_drawing();
    vita2d_swap_buffers();
}

int main()
{
    vita2d_init();
    font = vita2d_load_default_pgf();

    SceCtrlData pad; // Declare the controller input variable
    sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG_WIDE); // Set D-pad mode

    int was_holding_dpad = 0; // Track if the dpad was held previously

    while (1) {
        sceCtrlPeekBufferPositive(0, &pad, 1); // Get controller input

        if ((pad.buttons & SCE_CTRL_DOWN) || (pad.buttons & SCE_CTRL_UP)) {
            if (!was_holding_dpad) {
                // If dpad was not previously held, apply initial delay
                sceKernelDelayThread(initial_delay * 1000); // Delay in microseconds
                was_holding_dpad = 1;
            } else {
                // If dpad was previously held, apply normal delay
                sceKernelDelayThread(normal_delay * 1000); // Delay in microseconds
            }

            if (pad.buttons & SCE_CTRL_DOWN) {
                selected_item = (selected_item + 1) % (CONTENT_HEIGHT / ITEM_HEIGHT);
            }
            if (pad.buttons & SCE_CTRL_UP) {
                selected_item = (selected_item - 1 + (CONTENT_HEIGHT / ITEM_HEIGHT)) % (CONTENT_HEIGHT / ITEM_HEIGHT);
            }
        } else {
            // If dpad is not held, reset the holding state
            was_holding_dpad = 0;
        }

        mainloop();
    }

    vita2d_free_pgf(font);
    vita2d_fini();
    sceKernelExitProcess(0);

    return 0;
}
