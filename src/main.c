#include <psp2/display.h>
#include <psp2/kernel/processmgr.h>

#include <vita2d.h>

#include "font.h"


vita2d_pgf *font;

int mainloop()
{
  vita2d_start_drawing();
  vita2d_clear_screen();

  font_draw_string(100, 100, RGBA8(0xFF, 0xFF, 0xFF, 0xFF), "Yuki is een meem");

  vita2d_end_drawing();
  vita2d_swap_buffers();

  return 1;
}

int main()
{
  vita2d_init();

  while (1) {
    mainloop();
  }

  vita2d_fini();

  sceKernelExitProcess(0);

  return 0;
}