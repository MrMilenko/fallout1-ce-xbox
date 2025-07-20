#include "game/gmovie.h"
#include "xboxkrnl/xboxkrnl.h"
#include <stdio.h>
#include <string.h>

#include "game/cycle.h"
#include "game/game.h"
#include "game/gconfig.h"
#include "game/gmouse.h"
#include "game/gsound.h"
#include "game/moviefx.h"
#include "game/palette.h"
#include "int/movie.h"
#include "int/window.h"
#include "platform_compat.h"
#include "plib/color/color.h"
#include "plib/gnw/debug.h"
#include "plib/gnw/gnw.h"
#include "plib/gnw/input.h"
#include "plib/gnw/svga.h"
#include "plib/gnw/text.h"
#include "plib/gnw/touch.h"

namespace fallout {

#define GAME_MOVIE_WINDOW_WIDTH 640
#define GAME_MOVIE_WINDOW_HEIGHT 480

static char* gmovie_subtitle_func(char* movieFilePath);

// 0x5053FC
static const char* movie_list[MOVIE_COUNT] = {
    "iplogo.mve",
    "mplogo.mve",
    "intro.mve",
    "vexpld.mve",
    "cathexp.mve",
    "ovrintro.mve",
    "boil3.mve",
    "ovrrun.mve",
    "walkm.mve",
    "walkw.mve",
    "dipedv.mve",
    "boil1.mve",
    "boil2.mve",
    "raekills.mve",
};

// 0x596C78
static unsigned char gmovie_played_list[MOVIE_COUNT];

// gmovie_init
// 0x44E5C0
int gmovie_init()
{
    int volume = 0;
    if (gsound_background_is_enabled()) {
        volume = gsound_background_volume_get();
    }

    movieSetVolume(volume);

    movieSetSubtitleFunc(gmovie_subtitle_func);

    memset(gmovie_played_list, 0, sizeof(gmovie_played_list));

    return 0;
}

// 0x44E60C
void gmovie_reset()
{
    memset(gmovie_played_list, 0, sizeof(gmovie_played_list));
}

// 0x446064
void gmovie_exit()
{
}

// 0x44E638
int gmovie_load(DB_FILE* stream)
{
    if (db_fread(gmovie_played_list, sizeof(*gmovie_played_list), MOVIE_COUNT, stream) != MOVIE_COUNT) {
        return -1;
    }

    return 0;
}

// 0x44E664
int gmovie_save(DB_FILE* stream)
{
    if (db_fwrite(gmovie_played_list, sizeof(*gmovie_played_list), MOVIE_COUNT, stream) != MOVIE_COUNT) {
        return -1;
    }

    return 0;
}

int gmovie_play(int game_movie, int game_movie_flags)
{
    dir_entry de;
    char movieFilePath[COMPAT_MAX_PATH];

    // DbgPrint("gmovie_play: Playing movie index %d (%s)\n", game_movie, movie_list[game_movie]);

    snprintf(movieFilePath, sizeof(movieFilePath), "art\\cuts\\%s", movie_list[game_movie]);
    // DbgPrint("gmovie_play: Resolved path: %s\n", movieFilePath);

    if (db_dir_entry(movieFilePath, &de) != 0) {
        // DbgPrint("gmovie_play: Failed to open %s\n", movieFilePath);
        return -1;
    }

    if ((game_movie_flags & GAME_MOVIE_FADE_IN) != 0) {
        // DbgPrint("gmovie_play: Fading in to black\n");
        //palette_fade_to(black_palette);
    }

    int gameMovieWindowX = (screenGetWidth() - GAME_MOVIE_WINDOW_WIDTH) / 2;
    int gameMovieWindowY = (screenGetHeight() - GAME_MOVIE_WINDOW_HEIGHT) / 2;
    int win = win_add(gameMovieWindowX, gameMovieWindowY, GAME_MOVIE_WINDOW_WIDTH, GAME_MOVIE_WINDOW_HEIGHT, 0, WINDOW_MODAL);
    if (win == -1) {
        // DbgPrint("gmovie_play: Failed to create movie window\n");
        return -1;
    }

    // DbgPrint("gmovie_play: Created movie window at (%d,%d)\n", gameMovieWindowX, gameMovieWindowY);

    if ((game_movie_flags & GAME_MOVIE_STOP_MUSIC) != 0) {
        // DbgPrint("gmovie_play: Stopping background music\n");
        gsound_background_stop();
    } else if ((game_movie_flags & GAME_MOVIE_PAUSE_MUSIC) != 0) {
        // DbgPrint("gmovie_play: Pausing background music\n");
        gsound_background_pause();
    }

    win_draw(win);
    // DbgPrint("gmovie_play: Window drawn\n");

    bool subtitlesEnabled = false;
    if (game_movie == MOVIE_BOIL3 || game_movie == MOVIE_BOIL1 || game_movie == MOVIE_BOIL2) {
        subtitlesEnabled = true;
    } else {
        configGetBool(&game_config, GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_SUBTITLES_KEY, &subtitlesEnabled);
    }

    int movie_flags = 4;
    if (subtitlesEnabled) {
        char* subtitlesFilePath = gmovie_subtitle_func(movieFilePath);
        // DbgPrint("gmovie_play: Checking for subtitles at: %s\n", subtitlesFilePath);

        if (db_dir_entry(subtitlesFilePath, &de) == 0) {
            movie_flags |= 0x8;
            // DbgPrint("gmovie_play: Subtitles enabled\n");
        } else {
            subtitlesEnabled = false;
            // DbgPrint("gmovie_play: Subtitles not found, disabling\n");
        }
    }

    // DbgPrint("gmovie_play: Setting movie flags: 0x%X\n", movie_flags);
    movieSetFlags(movie_flags);

    int oldTextColor;
    int oldFont;
    if (subtitlesEnabled) {
        loadColorTable("art\\cuts\\subtitle.pal");
        oldTextColor = windowGetTextColor();
        windowSetTextColor(1.0, 1.0, 1.0);
        oldFont = text_curr();
        windowSetFont(101);
        // DbgPrint("gmovie_play: Subtitle font and color applied\n");
    }

    bool cursorWasHidden = mouse_hidden();
    if (cursorWasHidden) {
        // DbgPrint("gmovie_play: Cursor was hidden, setting to NONE and showing\n");
        gmouse_set_cursor(MOUSE_CURSOR_NONE);
        mouse_show();
    }

    while (mouse_get_buttons() != 0) {
        mouse_info();
    }

    // DbgPrint("gmovie_play: Hiding mouse and disabling cycle\n");
    mouse_hide();
    cycle_disable();

    // DbgPrint("gmovie_play: Starting movie effects\n");
    moviefx_start(movieFilePath);

    // DbgPrint("gmovie_play: Calling movieRun()\n");
    movieRun(win, movieFilePath);
    // DbgPrint("gmovie_play: movieRun() returned\n");

    int v11 = 0;
    int buttons;
    do {
        if (!moviePlaying() || game_user_wants_to_quit || get_input() != -1) {
            // DbgPrint("gmovie_play: Movie ended or user input detected\n");
            break;
        }

        Gesture gesture;
        if (touch_get_gesture(&gesture) && gesture.state == kEnded) {
            // DbgPrint("gmovie_play: Touch gesture detected\n");
            break;
        }

        int x, y;
        mouse_get_raw_state(&x, &y, &buttons);
        v11 |= buttons;
    } while (((v11 & 1) == 0 && (v11 & 2) == 0) || (buttons & 1) != 0 || (buttons & 2) != 0);

    // DbgPrint("gmovie_play: Stopping movie\n");
    movieStop();
    moviefx_stop();
    movieUpdate();
    palette_set_to(black_palette);

    gmovie_played_list[game_movie] = 1;

    // DbgPrint("gmovie_play: Re-enabling cycle\n");
    cycle_enable();

    gmouse_set_cursor(MOUSE_CURSOR_ARROW);
    if (!cursorWasHidden) {
        mouse_show();
    }

    if (subtitlesEnabled) {
        loadColorTable("color.pal");
        windowSetFont(oldFont);

        float r = (float)((Color2RGB(oldTextColor) & 0x7C00) >> 10) / 31.0f;
        float g = (float)((Color2RGB(oldTextColor) & 0x3E0) >> 5) / 31.0f;
        float b = (float)(Color2RGB(oldTextColor) & 0x1F) / 31.0f;
        windowSetTextColor(r, g, b);

        // DbgPrint("gmovie_play: Subtitle font/color restored\n");
    }

    win_delete(win);
    // DbgPrint("gmovie_play: Deleted movie window\n");

    win_refresh_all(&scr_size);

    if ((game_movie_flags & GAME_MOVIE_PAUSE_MUSIC) != 0) {
        gsound_background_unpause();
        // DbgPrint("gmovie_play: Resumed paused music\n");
    }

    if ((game_movie_flags & GAME_MOVIE_FADE_OUT) != 0) {
        if (!subtitlesEnabled) {
            loadColorTable("color.pal");
        }
        palette_fade_to(cmap);
        // DbgPrint("gmovie_play: Fading out\n");
    }

    // DbgPrint("gmovie_play: Done\n");
    return 0;
}


// 0x44EB04
bool gmovie_has_been_played(int movie)
{
    return gmovie_played_list[movie] == 1;
}

// 0x44EB1C
static char* gmovie_subtitle_func(char* movie_file_path)
{
    // 0x595226
    static char full_path[COMPAT_MAX_PATH];

    char* language;
    char* separator;

    config_get_string(&game_config, GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_LANGUAGE_KEY, &language);

    separator = strrchr(movie_file_path, '\\');
    if (separator != NULL) {
        movie_file_path = separator + 1;
    }

    snprintf(full_path, sizeof(full_path), "TEXT\\%s\\CUTS\\%s", language, movie_file_path);

    separator = strrchr(full_path, '.');
    if (*separator != '\0') {
        *separator = '\0';
    }

    strcpy(full_path + strlen(full_path), ".SVE");

    return full_path;
}

} // namespace fallout
