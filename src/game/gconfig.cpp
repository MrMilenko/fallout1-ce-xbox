#include "game/gconfig.h"
#include "xboxkrnl/xboxkrnl.h"
#include <stdio.h>
#include <string.h>

#include "platform_compat.h"

namespace fallout {

// A flag indicating if `game_config` was initialized.
//
// 0x504FD8
static bool gconfig_initialized = false;

// fallout.cfg
//
// 0x58CC20
Config game_config;

// NOTE: There are additional 4 bytes following this array at 0x58EA7C, which
// probably means it's size is 264 bytes.
//
// 0x58CC48
static char gconfig_file_name[COMPAT_MAX_PATH];

// Inits main game config.
//
// `isMapper` is a flag indicating whether we're initing config for a main
// game, or a mapper. This value is `false` for the game itself.
//
// `argc` and `argv` are command line arguments. The engine assumes there is
// at least 1 element which is executable path at index 0. There is no
// additional check for `argc`, so it will crash if you pass NULL, or an empty
// array into `argv`.
//
// The executable path from `argv` is used resolve path to `fallout.cfg`,
// which should be in the same folder. This function provide defaults if
// `fallout.cfg` is not present, or cannot be read for any reason.
//
// Finally, this function merges key-value pairs from `argv` if any, see
// `config_cmd_line_parse` for expected format.
//
// 0x43D690
bool gconfig_init(bool isMapper, int argc, char** argv)
{
    // DbgPrint("gconfig_init: entered, isMapper=%d, argc=%d\n", isMapper, argc);

    if (gconfig_initialized) {
        // DbgPrint("gconfig_init: already initialized\n");
        return false;
    }

    // DbgPrint("gconfig_init: calling config_init\n");
    if (!config_init(&game_config)) {
        // DbgPrint("gconfig_init: config_init FAILED\n");
        return false;
    }

    // Build fallout.cfg file path
    char basePath[256] = "";
    if (argv != NULL && argc > 0 && argv[0] != NULL) {
        // DbgPrint("gconfig_init: argv[0] = %s\n", argv[0]);
        char* sep = strrchr(argv[0], '\\');
        if (sep != NULL) {
            *sep = '\0';
            strncpy(basePath, argv[0], sizeof(basePath));
            *sep = '\\';
            snprintf(gconfig_file_name, sizeof(gconfig_file_name), "%s\\%s", basePath, GAME_CONFIG_FILE_NAME);
        } else {
            strncpy(basePath, ".", sizeof(basePath));
            strcpy(gconfig_file_name, GAME_CONFIG_FILE_NAME);
        }
    } else {
        // DbgPrint("gconfig_init: argv[0] was null, falling back to default\n");
        strncpy(basePath, ".", sizeof(basePath));
        strcpy(gconfig_file_name, GAME_CONFIG_FILE_NAME);
    }

    // DbgPrint("gconfig_init: basePath = %s\n", basePath);
    // DbgPrint("gconfig_init: gconfig_file_name = %s\n", gconfig_file_name);

    // Construct full paths
    char fullMasterDat[256];
    char fullCritterDat[256];
    char fullDataPath[256];

    snprintf(fullMasterDat, sizeof(fullMasterDat), "%s\\master.dat", basePath);
    snprintf(fullCritterDat, sizeof(fullCritterDat), "%s\\critter.dat", basePath);
    snprintf(fullDataPath, sizeof(fullDataPath), "%s\\data", basePath);

    // DbgPrint("gconfig_init: fullMasterDat = %s\n", fullMasterDat);
    // DbgPrint("gconfig_init: fullCritterDat = %s\n", fullCritterDat);
    // DbgPrint("gconfig_init: fullDataPath  = %s\n", fullDataPath);

    // Set default config values (with resolved paths)
    // DbgPrint("gconfig_init: setting default config values\n");

    config_set_string(&game_config, GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_EXECUTABLE_KEY, "game");
    config_set_string(&game_config, GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_MASTER_DAT_KEY, fullMasterDat);
    config_set_string(&game_config, GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_MASTER_PATCHES_KEY, fullDataPath);
    config_set_string(&game_config, GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_CRITTER_DAT_KEY, fullCritterDat);
    config_set_string(&game_config, GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_CRITTER_PATCHES_KEY, fullDataPath);
    config_set_string(&game_config, GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_LANGUAGE_KEY, ENGLISH);
    config_set_value(&game_config, GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_SCROLL_LOCK_KEY, 0);
    config_set_value(&game_config, GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_INTERRUPT_WALK_KEY, 1);
    config_set_value(&game_config, GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_ART_CACHE_SIZE_KEY, 8);
    config_set_value(&game_config, GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_COLOR_CYCLING_KEY, 1);
    config_set_value(&game_config, GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_HASHING_KEY, 1);
    config_set_value(&game_config, GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_SPLASH_KEY, 0);
    config_set_value(&game_config, GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_FREE_SPACE_KEY, 20480);
    config_set_value(&game_config, GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_GAME_DIFFICULTY_KEY, 1);
    config_set_value(&game_config, GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_COMBAT_DIFFICULTY_KEY, 1);
    config_set_value(&game_config, GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_VIOLENCE_LEVEL_KEY, 3);
    config_set_value(&game_config, GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_TARGET_HIGHLIGHT_KEY, 2);
    config_set_value(&game_config, GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_ITEM_HIGHLIGHT_KEY, 1);
    config_set_value(&game_config, GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_RUNNING_BURNING_GUY_KEY, 1);
    config_set_value(&game_config, GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_COMBAT_MESSAGES_KEY, 1);
    config_set_value(&game_config, GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_COMBAT_TAUNTS_KEY, 1);
    config_set_value(&game_config, GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_LANGUAGE_FILTER_KEY, 0);
    config_set_value(&game_config, GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_RUNNING_KEY, 0);
    config_set_value(&game_config, GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_SUBTITLES_KEY, 0);
    config_set_value(&game_config, GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_COMBAT_SPEED_KEY, 0);
    config_set_value(&game_config, GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_PLAYER_SPEED_KEY, 0);
    config_set_double(&game_config, GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_TEXT_BASE_DELAY_KEY, 3.5);
    config_set_double(&game_config, GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_TEXT_LINE_DELAY_KEY, 1.399994);
    config_set_double(&game_config, GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_BRIGHTNESS_KEY, 1.0);
    config_set_double(&game_config, GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_MOUSE_SENSITIVITY_KEY, 1.0);
    config_set_value(&game_config, GAME_CONFIG_SOUND_KEY, GAME_CONFIG_INITIALIZE_KEY, 1);
    config_set_value(&game_config, GAME_CONFIG_SOUND_KEY, GAME_CONFIG_DEVICE_KEY, -1);
    config_set_value(&game_config, GAME_CONFIG_SOUND_KEY, GAME_CONFIG_PORT_KEY, -1);
    config_set_value(&game_config, GAME_CONFIG_SOUND_KEY, GAME_CONFIG_IRQ_KEY, -1);
    config_set_value(&game_config, GAME_CONFIG_SOUND_KEY, GAME_CONFIG_DMA_KEY, -1);
    config_set_value(&game_config, GAME_CONFIG_SOUND_KEY, GAME_CONFIG_SOUNDS_KEY, 1);
    config_set_value(&game_config, GAME_CONFIG_SOUND_KEY, GAME_CONFIG_MUSIC_KEY, 1);
    config_set_value(&game_config, GAME_CONFIG_SOUND_KEY, GAME_CONFIG_SPEECH_KEY, 1);
    config_set_value(&game_config, GAME_CONFIG_SOUND_KEY, GAME_CONFIG_MASTER_VOLUME_KEY, 22281);
    config_set_value(&game_config, GAME_CONFIG_SOUND_KEY, GAME_CONFIG_MUSIC_VOLUME_KEY, 22281);
    config_set_value(&game_config, GAME_CONFIG_SOUND_KEY, GAME_CONFIG_SNDFX_VOLUME_KEY, 22281);
    config_set_value(&game_config, GAME_CONFIG_SOUND_KEY, GAME_CONFIG_SPEECH_VOLUME_KEY, 22281);
    config_set_value(&game_config, GAME_CONFIG_SOUND_KEY, GAME_CONFIG_CACHE_SIZE_KEY, 448);
    config_set_string(&game_config, GAME_CONFIG_SOUND_KEY, GAME_CONFIG_MUSIC_PATH1_KEY, "sound\\music\\");
    config_set_string(&game_config, GAME_CONFIG_SOUND_KEY, GAME_CONFIG_MUSIC_PATH2_KEY, "sound\\music\\");
    config_set_string(&game_config, GAME_CONFIG_DEBUG_KEY, GAME_CONFIG_MODE_KEY, "environment");
    config_set_value(&game_config, GAME_CONFIG_DEBUG_KEY, GAME_CONFIG_SHOW_TILE_NUM_KEY, 0);
    config_set_value(&game_config, GAME_CONFIG_DEBUG_KEY, GAME_CONFIG_SHOW_SCRIPT_MESSAGES_KEY, 0);
    config_set_value(&game_config, GAME_CONFIG_DEBUG_KEY, GAME_CONFIG_SHOW_LOAD_INFO_KEY, 0);
    config_set_value(&game_config, GAME_CONFIG_DEBUG_KEY, GAME_CONFIG_OUTPUT_MAP_DATA_INFO_KEY, 0);

    if (isMapper) {
        // DbgPrint("gconfig_init: applying mapper overrides\n");
        config_set_string(&game_config, GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_EXECUTABLE_KEY, "mapper");
        config_set_value(&game_config, GAME_CONFIG_MAPPER_KEY, GAME_CONFIG_OVERRIDE_LIBRARIAN_KEY, 0);
        config_set_value(&game_config, GAME_CONFIG_MAPPER_KEY, GAME_CONFIG_USE_ART_NOT_PROTOS_KEY, 0);
        config_set_value(&game_config, GAME_CONFIG_MAPPER_KEY, GAME_CONFIG_REBUILD_PROTOS_KEY, 0);
        config_set_value(&game_config, GAME_CONFIG_MAPPER_KEY, GAME_CONFIG_FIX_MAP_OBJECTS_KEY, 0);
        config_set_value(&game_config, GAME_CONFIG_MAPPER_KEY, GAME_CONFIG_FIX_MAP_INVENTORY_KEY, 0);
        config_set_value(&game_config, GAME_CONFIG_MAPPER_KEY, GAME_CONFIG_IGNORE_REBUILD_ERRORS_KEY, 0);
        config_set_value(&game_config, GAME_CONFIG_MAPPER_KEY, GAME_CONFIG_SHOW_PID_NUMBERS_KEY, 0);
        config_set_value(&game_config, GAME_CONFIG_MAPPER_KEY, GAME_CONFIG_SAVE_TEXT_MAPS_KEY, 0);
        config_set_value(&game_config, GAME_CONFIG_MAPPER_KEY, GAME_CONFIG_RUN_MAPPER_AS_GAME_KEY, 0);
        config_set_value(&game_config, GAME_CONFIG_MAPPER_KEY, GAME_CONFIG_DEFAULT_F8_AS_GAME_KEY, 1);
    }

    if (!config_load(&game_config, gconfig_file_name, false)) {
        // DbgPrint("gconfig_init: config_load failed or file missing\n");
    } else {
        // DbgPrint("gconfig_init: config_load succeeded\n");
    }

    // DbgPrint("gconfig_init: parsing command-line overrides\n");
    config_cmd_line_parse(&game_config, argc, argv);

    gconfig_initialized = true;
    // DbgPrint("gconfig_init: completed successfully\n");

    return true;
}






// Saves game config into `fallout.cfg`.
//
// 0x43DD08
bool gconfig_save()
{
    if (!gconfig_initialized) {
        return false;
    }

    if (!config_save(&game_config, gconfig_file_name, false)) {
        return false;
    }

    return true;
}

// Frees game config, optionally saving it.
//
// 0x43DD30
bool gconfig_exit(bool shouldSave)
{
    if (!gconfig_initialized) {
        return false;
    }

    bool result = true;

    if (shouldSave) {
        if (!config_save(&game_config, gconfig_file_name, false)) {
            result = false;
        }
    }

    config_exit(&game_config);

    gconfig_initialized = false;

    return result;
}

} // namespace fallout
