#include "game/palette.h"

#include <string.h>

#include "game/cycle.h"
#include "game/gsound.h"
#include "plib/color/color.h"
#include "plib/gnw/debug.h"
#include "plib/gnw/input.h"
#include "xboxkrnl/xboxkrnl.h"
namespace fallout {

// 0x661F20
static unsigned char current_palette[256 * 3];

// 0x662220
unsigned char white_palette[256 * 3];

// 0x662520
unsigned char black_palette[256 * 3];

// 0x662820
static int fade_steps;

// 0x485090
void palette_init()
{
    // DbgPrint("palette_init(): starting\n");

    memset(black_palette, 0, 256 * 3);
    memset(white_palette, 63, 256 * 3);
    memcpy(current_palette, cmap, 256 * 3);

    unsigned int tick = SDL_GetTicks(); // Replaces get_time()

    // Stubbed: disable audio fade callback logic
    // Original:
    // if (gsound_background_is_enabled() || gsound_speech_is_enabled()) {
    //     colorSetFadeBkFunc(soundUpdate);
    // }
    // DbgPrint("palette_init(): skipping sound fade background callback\n");

    // Stubbed: skip actual palette fade logic
    // Original:
    // fadeSystemPalette(current_palette, current_palette, 60);
    // DbgPrint("palette_init(): skipping fadeSystemPalette()\n");

    // Stubbed: remove callback after fade
    // Original:
    // colorSetFadeBkFunc(NULL);
    // DbgPrint("palette_init(): skipping colorSetFadeBkFunc(NULL)\n");

    // Simulate fade duration
    unsigned int actualFadeDuration = SDL_GetTicks() - tick;
    if (actualFadeDuration == 0) actualFadeDuration = 1; // prevent divide by zero

    // Calculate fade steps (kept intact)
    fade_steps = 60 * 700 / actualFadeDuration;

    debug_printf("\nFade time is %u\nFade steps are %d\n", actualFadeDuration, fade_steps);
    // DbgPrint("palette_init(): complete\n");
}


// 0x485160
void palette_reset()
{
}

// 0x485160
void palette_exit()
{
}

// 0x485164
void palette_fade_to(unsigned char* palette)
{
    bool colorCycleWasEnabled = cycle_is_enabled();
    cycle_disable();

    if (gsound_background_is_enabled() || gsound_speech_is_enabled()) {
        colorSetFadeBkFunc(soundUpdate);
    }

    fadeSystemPalette(current_palette, palette, fade_steps);
    colorSetFadeBkFunc(NULL);

    memcpy(current_palette, palette, 768);

    if (colorCycleWasEnabled) {
        cycle_enable();
    }
}

// 0x4851D8
void palette_set_to(unsigned char* palette)
{
    memcpy(current_palette, palette, sizeof(current_palette));
    setSystemPalette(palette);
}

// 0x485208
void palette_set_entries(unsigned char* palette, int start, int end)
{
    memcpy(current_palette + 3 * start, palette, 3 * (end - start + 1));
    setSystemPaletteEntries(palette, start, end);
}

} // namespace fallout
