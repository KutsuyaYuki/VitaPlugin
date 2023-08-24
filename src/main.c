#include <psp2/display.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/threadmgr.h> // Include for delay function
#include <psp2/ctrl.h> // Add this for controller input
#include <psp2/net/net.h> // Add this for network functions
#include <psp2/net/netctl.h>
#include <psp2/sysmodule.h>

#include <vita2d.h>
#include "font.h"
#include "network.h"
#include "json.h"

#define SCREEN_WIDTH 960
#define SCREEN_HEIGHT 544

#define TITLE_BAR_HEIGHT 30
#define CONTENT_AREA TITLE_BAR_HEIGHT + 30
#define CONTENT_HEIGHT (SCREEN_HEIGHT - TITLE_BAR_HEIGHT)
#define ITEM_HEIGHT 40 // Height of each list item

int selected_item = 0; // Index of the currently selected item
int delay_counter = 0; // Counter to track delay
int INITIAL_DELAY = 200; // Initial delay in ms
int NORMAL_DELAY = 50; // Normal delay in ms

ParsedJSON parsed_json;

vita2d_pgf *font;

int initNet() {
    sceSysmoduleLoadModule(SCE_SYSMODULE_NET);
    SceNetInitParam initparam;
    int size = 1 * 1024 * 1024;
    initparam.memory = malloc(size);
    initparam.size = size;
    initparam.flags = 0;

    int result = sceNetInit(&initparam);
    if (result != 0 && result != SCE_NET_ERROR_EBUSY) {
        // Handle the error
        return result;
    }

    sceNetCtlInit();

    return 0;
}

void display_json_items(ParsedJSON parsed_json, int page) {
    int ITEMS_PER_PAGE = 10;
    int start_index = page * ITEMS_PER_PAGE;
    int end_index = start_index + ITEMS_PER_PAGE - 1;
    if (end_index >= parsed_json.count) {
        end_index = parsed_json.count - 1;
    }

    int y = CONTENT_AREA; // Start at CONTENT_AREA
    for (int i = start_index; i <= end_index; i++) {
        const char *item_name = parsed_json.items[i];

        // Determine the color based on the selection
        uint32_t color = (i == selected_item) ? RGBA8(0xFF, 0x00, 0x00, 0xFF) : RGBA8(0xFF, 0xFF, 0xFF, 0xFF);

        font_draw_string(10, y, color, item_name);
        y += ITEM_HEIGHT;
    }
}

void mainloop()
{
    vita2d_start_drawing();
    vita2d_clear_screen();

    // Draw title bar
    vita2d_draw_rectangle(0, 0, SCREEN_WIDTH, TITLE_BAR_HEIGHT, RGBA8(0x00, 0x00, 0xFF, 0xFF));
    font_draw_string(10, 5, RGBA8(0xFF, 0xFF, 0xFF, 0xFF), "Yuki's First Vita Application");

    // Draw content area
    vita2d_draw_rectangle(0, TITLE_BAR_HEIGHT, SCREEN_WIDTH, CONTENT_HEIGHT, RGBA8(0x00, 0x00, 0x00, 0xFF));

    display_json_items(parsed_json, 0); // Displaying page 0

    vita2d_end_drawing();
    vita2d_swap_buffers();
}

int main() {
    vita2d_init();
    initNet();
    font = vita2d_load_default_pgf();

    // Download and parse JSON
    const char *url = "http://sharkdash.blahaj.nl/vita.json";
    const char *file_name = "ux0:data/downloaded.json";
    download_data(url, file_name);
    parsed_json = parse_json(file_name);

    SceCtrlData pad; // Declare the controller input variable
    sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG_WIDE); // Set D-pad mode

    int was_holding_dpad = 0; // Track if the dpad was held previously

    while (1) {
    sceCtrlPeekBufferPositive(0, &pad, 1); // Get controller input

    if ((pad.buttons & SCE_CTRL_DOWN) || (pad.buttons & SCE_CTRL_UP)) {
        if (!was_holding_dpad) {
            // If dpad was not previously held, apply initial delay
            sceKernelDelayThread(INITIAL_DELAY * 1000); // Delay in microseconds
            was_holding_dpad = 1;
        } else {
            // If dpad was previously held, apply normal delay
            sceKernelDelayThread(NORMAL_DELAY * 1000); // Delay in microseconds
        }

        if (pad.buttons & SCE_CTRL_DOWN) {
            selected_item = (selected_item + 1) % parsed_json.count;
        }
        if (pad.buttons & SCE_CTRL_UP) {
            selected_item = (selected_item - 1 + parsed_json.count) % parsed_json.count;
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