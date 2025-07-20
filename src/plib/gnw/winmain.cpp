#include "plib/gnw/winmain.h"

#include <stdlib.h>
#include <SDL.h>

#ifndef _WIN32
#include <unistd.h>
#endif

#include "game/main.h"
#include "plib/gnw/gnw.h"
#include "plib/gnw/svga.h"

#if __APPLE__ && TARGET_OS_IOS
#include "platform/ios/paths.h"
#endif

#if defined(__NXDK__)
#include <xboxkrnl/xboxkrnl.h>
#include <nxdk/path.h>
#endif

namespace fallout {

// 0x53A290
bool GNW95_isActive = false;

#if _WIN32
// 0x53A294
HANDLE GNW95_mutex = NULL;
#endif

// 0x6B0760
char GNW95_title[256];

#if defined(__NXDK__)
void debug_simulated_cwd()
{
    char ntPath[260] = {0};
    nxGetCurrentXbeNtPath(ntPath);

    if (ntPath[0]) {
        // DbgPrint("MAIN: Loaded XBE NT path: %s\n", ntPath);

        if (strncmp(ntPath, "\\Device\\Harddisk0\\Partition1", 28) == 0) {
            const char* relativePath = ntPath + 28;
            char dosPath[260];
            strncpy(dosPath, "E:", sizeof(dosPath));
            strncat(dosPath, relativePath, sizeof(dosPath) - strlen(dosPath) - 1);

            // Trim off the filename to simulate a CWD
            char* lastSlash = strrchr(dosPath, '\\');
            if (lastSlash) {
                *lastSlash = '\0';
            }

            // DbgPrint("MAIN: simulated working directory: %s\n", dosPath);
        } else {
            // DbgPrint("MAIN: Unrecognized NT path prefix\n");
        }
    } else {
        // DbgPrint("MAIN: nxGetCurrentXbeNtPath returned empty path\n");
    }
}
#endif

int main(int argc, char* argv[])
{
    // DbgPrint("Fallout1-CE: Entering main()\n");

    int rc;

#if _WIN32
    GNW95_mutex = CreateMutexA(0, TRUE, "GNW95MUTEX");
    if (GetLastError() != ERROR_SUCCESS) {
        // DbgPrint("Fallout1-CE: Failed to acquire GNW95 mutex\n");
        return 0;
    }
#endif

#if __APPLE__ && TARGET_OS_IOS
    SDL_SetHint(SDL_HINT_MOUSE_TOUCH_EVENTS, "0");
    SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "0");
    chdir(iOSGetDocumentsPath());
#endif

#if __APPLE__ && TARGET_OS_OSX
    char* basePath = SDL_GetBasePath();
    chdir(basePath);
    SDL_free(basePath);
#endif

#if __ANDROID__
    SDL_SetHint(SDL_HINT_MOUSE_TOUCH_EVENTS, "0");
    SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "0");
    chdir(SDL_AndroidGetExternalStoragePath());
#endif

#if defined(__NXDK__)
    debug_simulated_cwd();
#endif

    SDL_ShowCursor(SDL_DISABLE);
    GNW95_isActive = true;

    // DbgPrint("Fallout1-CE: Calling gnw_main()\n");
    rc = gnw_main(argc, argv);
    // DbgPrint("Fallout1-CE: gnw_main() returned %d\n", rc);

#if _WIN32
    CloseHandle(GNW95_mutex);
#endif

    // DbgPrint("Fallout1-CE: Exiting main()\n");
    return rc;
}

} // namespace fallout

int main(int argc, char* argv[])
{
    return fallout::main(argc, argv);
}

