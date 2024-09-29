#include <psp2/display.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/threadmgr.h> // Include for delay function
#include <psp2/ctrl.h>             // For controller input
#include <psp2/net/net.h>          // For network functions
#include <psp2/net/netctl.h>
#include <psp2/sysmodule.h>
#include <stdlib.h>                // For calloc and free
#include <vita2d.h>
#include "font.h"
#include "network.h"
#include "json.h"

#define SCREEN_WIDTH 960
#define SCREEN_HEIGHT 544

#define TITLE_BAR_HEIGHT 30
#define CONTENT_AREA TITLE_BAR_HEIGHT
#define CONTENT_HEIGHT (SCREEN_HEIGHT - TITLE_BAR_HEIGHT)
#define ITEM_HEIGHT 40 // Height of each list item

#define BACKGROUND_IMAGE "ux0:data/wallpaper.png" // File name for the downloaded background image

int selected_item = 0;   // Index of the currently selected item
int delay_counter = 0;   // Counter to track delay
int INITIAL_DELAY = 200; // Initial delay in ms
int NORMAL_DELAY = 200;  // Normal delay in ms

int ITEMS_PER_PAGE = 10; // Number of items per page

ParsedJSON parsed_json;
uint32_t *clicked_times; // Array to track the time when items are clicked

vita2d_pgf *font;
vita2d_texture *background_texture; // Texture for the background image

int initNet()
{
    sceSysmoduleLoadModule(SCE_SYSMODULE_NET);
    SceNetInitParam initparam;
    int size = 1 * 1024 * 1024;
    initparam.memory = malloc(size);
    initparam.size = size;
    initparam.flags = 0;

    int result = sceNetInit(&initparam);
    if (result != 0 && result != SCE_NET_ERROR_EBUSY)
    {
        // Handle the error
        return result;
    }

    sceNetCtlInit();

    return 0;
}

void display_json_items(ParsedJSON parsed_json, int page, uint32_t current_time)
{
    int start_index = page * ITEMS_PER_PAGE;
    int end_index = start_index + ITEMS_PER_PAGE - 1;
    if (end_index >= parsed_json.count)
    {
        end_index = parsed_json.count - 1;
    }

    int y = CONTENT_AREA; // Start at CONTENT_AREA
    for (int i = start_index; i <= end_index; i++)
    {
        const char *item_name = parsed_json.items[i];

        // Determine the color based on the selection and clicked status
        uint32_t color;
        if (clicked_times[i] != 0 && (current_time - clicked_times[i] <= 1500000)) {
            color = RGBA8(0x00, 0xFF, 0x00, 0xFF); // Green if clicked within 1.5 seconds
        } else if (i == selected_item) {
            color = RGBA8(0xFF, 0x00, 0x00, 0xFF); // Red if selected
        } else {
            color = RGBA8(0xFF, 0xFF, 0xFF, 0xFF); // White otherwise
        }

        font_draw_string(10, y, color, item_name);
        y += ITEM_HEIGHT;
    }
}

void drawpage(int current_page, int total_pages, uint32_t current_time)
{
    vita2d_start_drawing();
    vita2d_clear_screen();

    // Draw the background texture in the content area if it's valid
    if (background_texture != NULL) {
        vita2d_draw_texture(background_texture, 0, CONTENT_AREA); // Draw at (0, CONTENT_AREA)
    }
    else {
        vita2d_draw_rectangle(0, CONTENT_AREA, SCREEN_WIDTH, CONTENT_HEIGHT, RGBA8(0xFF, 0x00, 0x00, 0xFF));
    }

    // Draw title bar
    vita2d_draw_rectangle(0, 0, SCREEN_WIDTH, TITLE_BAR_HEIGHT, RGBA8(0x00, 0x00, 0xFF, 0xFF));
    font_draw_string(10, 5, RGBA8(0xFF, 0xFF, 0xFF, 0xFF), "Yuki's First Vita Application");

    // Draw content area
    vita2d_draw_rectangle(0, TITLE_BAR_HEIGHT, SCREEN_WIDTH, CONTENT_HEIGHT, RGBA8(0x00, 0x00, 0x00, 0x55));

    display_json_items(parsed_json, current_page, current_time); // Display the current page

    // Draw footer bar
    vita2d_draw_rectangle(0, SCREEN_HEIGHT - TITLE_BAR_HEIGHT, SCREEN_WIDTH, TITLE_BAR_HEIGHT, RGBA8(0x00, 0x00, 0xFF, 0xFF));

    // Create the footer text
    char footer_text[20]; // Assuming the footer text won't exceed 20 characters
    snprintf(footer_text, sizeof(footer_text), "%d/%d", current_page + 1, total_pages);

    // Draw the footer text
    font_draw_string(10, SCREEN_HEIGHT - TITLE_BAR_HEIGHT + 5, RGBA8(0xFF, 0xFF, 0xFF, 0xFF), footer_text);
    vita2d_end_drawing();
    vita2d_swap_buffers();
}

int main()
{
    vita2d_init();
    initNet();
    font = vita2d_load_default_pgf();

    // Download and parse JSON
    const char *url = "http://sharkdash.blahaj.nl/vita/vita.json";
    const char *file_name = "ux0:data/downloaded.json";
    download_data(url, file_name);
    parsed_json = parse_json(file_name);

    // Allocate memory for clicked_times
    clicked_times = calloc(parsed_json.count, sizeof(uint32_t));

    download_data(parsed_json.background_url, BACKGROUND_IMAGE);
    background_texture = vita2d_load_PNG_file(BACKGROUND_IMAGE); // Load the background texture
    if (background_texture == NULL) {
        printf("Failed to load background texture.\n");
    }

    SceCtrlData pad;                                   // Declare the controller input variable
    sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG_WIDE); // Set D-pad mode

    int was_holding_dpad = 0;        // Track if the dpad was held previously
    unsigned int next_move_time = 0; // Time when the next move is allowed
    unsigned int current_delay = 0;  // Current delay (initial or normal)

    int total_pages = (parsed_json.count + 9) / 10; // Calculate the total number of pages
    int current_page = 0;                           // Initialize the current page to 0

    while (1)
    {
        sceCtrlPeekBufferPositive(0, &pad, 1);

        uint32_t current_time = sceKernelGetProcessTimeLow();

        if (pad.buttons & SCE_CTRL_SELECT)
        {
            break;
        }

        if (pad.buttons & SCE_CTRL_LTRIGGER)
        {
            selected_item = 0;
        }

        if (pad.buttons & SCE_CTRL_RTRIGGER)
        {
            selected_item = parsed_json.count - 1;
        }

        // Check for click action
        if (pad.buttons & SCE_CTRL_CROSS)
        {
            clicked_times[selected_item] = current_time;
        }

        if (pad.buttons & (SCE_CTRL_DOWN | SCE_CTRL_UP))
        {
            if (!was_holding_dpad)
            {
                selected_item = (selected_item + (pad.buttons & SCE_CTRL_DOWN ? 1 : -1) + parsed_json.count) % parsed_json.count;
                was_holding_dpad = 1;
                current_delay = INITIAL_DELAY;
                next_move_time = current_time + current_delay * 1000;

                // Update current_page if the selection goes off the current page
                if (selected_item < current_page * ITEMS_PER_PAGE || selected_item >= (current_page + 1) * ITEMS_PER_PAGE)
                {
                    current_page = selected_item / ITEMS_PER_PAGE;
                }
            }
            else if (current_time >= next_move_time)
            {
                selected_item = (selected_item + (pad.buttons & SCE_CTRL_DOWN ? 1 : -1) + parsed_json.count) % parsed_json.count;
                current_delay = NORMAL_DELAY;
                next_move_time = current_time + current_delay * 250;

                // Update current_page if the selection goes off the current page
                if (selected_item < current_page * ITEMS_PER_PAGE || selected_item >= (current_page + 1) * ITEMS_PER_PAGE)
                {
                    current_page = selected_item / ITEMS_PER_PAGE;
                }
            }
        } else if (pad.buttons & (SCE_CTRL_LEFT | SCE_CTRL_RIGHT))
        {
            if (!was_holding_dpad)
            {
                current_page = (current_page + (pad.buttons & SCE_CTRL_RIGHT ? 1 : -1) + total_pages) % total_pages;
                was_holding_dpad = 1;
                current_delay = INITIAL_DELAY;
                next_move_time = current_time + current_delay * 1000;
            }
            else if (current_time >= next_move_time)
            {
                current_page = (current_page + (pad.buttons & SCE_CTRL_RIGHT ? 1 : -1) + total_pages) % total_pages;
                current_delay = NORMAL_DELAY;
                next_move_time = current_time + current_delay * 250;
            }
        } else {
            was_holding_dpad = 0;
        }

        drawpage(current_page, total_pages, current_time);
    }

    // Free allocated resources
    free(clicked_times);

    vita2d_free_pgf(font);
    vita2d_fini();
    sceKernelExitProcess(0);

    return 0;
}
