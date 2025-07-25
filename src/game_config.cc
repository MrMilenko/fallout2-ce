#include "game_config.h"
#include "sfall_config.h"
#ifdef NXDK
#include "xboxkrnl/xboxkrnl.h"
#endif
#include <stdio.h>
#include <string.h>

#include "db.h"
#include "main.h"
#include "platform_compat.h"

namespace fallout {

static void gameConfigResolvePath(const char* section, const char* key);

// A flag indicating if [gGameConfig] was initialized.
//
// 0x5186D0
bool gGameConfigInitialized = false;

// fallout2.cfg
//
// 0x58E950
Config gGameConfig;

// NOTE: There are additional 4 bytes following this array at 0x58EA7C, which
// probably means it's size is 264 bytes.
//
// 0x58E978
char gGameConfigFilePath[COMPAT_MAX_PATH];

// Inits main game config.
//
// [isMapper] is a flag indicating whether we're initing config for a main
// game, or a mapper. This value is `false` for the game itself.
//
// [argc] and [argv] are command line arguments. The engine assumes there is
// at least 1 element which is executable path at index 0. There is no
// additional check for [argc], so it will crash if you pass NULL, or an empty
// array into [argv].
//
// The executable path from [argv] is used resolve path to `fallout2.cfg`,
// which should be in the same folder. This function provide defaults if
// `fallout2.cfg` is not present, or cannot be read for any reason.
//
// Finally, this function merges key-value pairs from [argv] if any, see
// [configParseCommandLineArguments] for expected format.
//
// 0x444570
bool gameConfigInit(bool isMapper, int argc, char** argv)
{
    if (gGameConfigInitialized) {
        return false;
    }

    if (!configInit(&gGameConfig)) {
        return false;
    }

    // Initialize defaults.
    configSetString(&gGameConfig, GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_EXECUTABLE_KEY, "game");
    configSetString(&gGameConfig, GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_MASTER_DAT_KEY, "master.dat");
    configSetString(&gGameConfig, GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_MASTER_PATCHES_KEY, "data");
    configSetString(&gGameConfig, GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_CRITTER_DAT_KEY, "critter.dat");
    configSetString(&gGameConfig, GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_CRITTER_PATCHES_KEY, "data");
    configSetString(&gGameConfig, GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_LANGUAGE_KEY, ENGLISH);
    configSetInt(&gGameConfig, GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_SCROLL_LOCK_KEY, 0);
    configSetInt(&gGameConfig, GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_INTERRUPT_WALK_KEY, 1);
    configSetInt(&gGameConfig, GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_ART_CACHE_SIZE_KEY, 8);
    configSetInt(&gGameConfig, GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_COLOR_CYCLING_KEY, 1);
    configSetInt(&gGameConfig, GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_HASHING_KEY, 1);
    configSetInt(&gGameConfig, GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_SPLASH_KEY, 0);
    configSetInt(&gGameConfig, GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_FREE_SPACE_KEY, 20480);
    configSetInt(&gGameConfig, GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_GAME_DIFFICULTY_KEY, 1);
    configSetInt(&gGameConfig, GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_COMBAT_DIFFICULTY_KEY, 1);
    configSetInt(&gGameConfig, GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_VIOLENCE_LEVEL_KEY, 3);
    configSetInt(&gGameConfig, GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_TARGET_HIGHLIGHT_KEY, 2);
    configSetInt(&gGameConfig, GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_ITEM_HIGHLIGHT_KEY, 1);
    configSetInt(&gGameConfig, GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_COMBAT_LOOKS_KEY, 0);
    configSetInt(&gGameConfig, GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_COMBAT_MESSAGES_KEY, 1);
    configSetInt(&gGameConfig, GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_COMBAT_TAUNTS_KEY, 1);
    configSetInt(&gGameConfig, GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_LANGUAGE_FILTER_KEY, 0);
    configSetInt(&gGameConfig, GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_RUNNING_KEY, 0);
    configSetInt(&gGameConfig, GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_SUBTITLES_KEY, 0);
    configSetInt(&gGameConfig, GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_COMBAT_SPEED_KEY, 0);
    configSetInt(&gGameConfig, GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_PLAYER_SPEED_KEY, 0);
    configSetDouble(&gGameConfig, GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_TEXT_BASE_DELAY_KEY, 3.5);
    configSetDouble(&gGameConfig, GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_TEXT_LINE_DELAY_KEY, 1.399994);
    configSetDouble(&gGameConfig, GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_BRIGHTNESS_KEY, 1.0);
    configSetDouble(&gGameConfig, GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_MOUSE_SENSITIVITY_KEY, 1.0);
    configSetInt(&gGameConfig, GAME_CONFIG_SOUND_KEY, GAME_CONFIG_INITIALIZE_KEY, 1);
    configSetInt(&gGameConfig, GAME_CONFIG_SOUND_KEY, GAME_CONFIG_DEVICE_KEY, -1);
    configSetInt(&gGameConfig, GAME_CONFIG_SOUND_KEY, GAME_CONFIG_PORT_KEY, -1);
    configSetInt(&gGameConfig, GAME_CONFIG_SOUND_KEY, GAME_CONFIG_IRQ_KEY, -1);
    configSetInt(&gGameConfig, GAME_CONFIG_SOUND_KEY, GAME_CONFIG_DMA_KEY, -1);
    configSetInt(&gGameConfig, GAME_CONFIG_SOUND_KEY, GAME_CONFIG_SOUNDS_KEY, 1);
    configSetInt(&gGameConfig, GAME_CONFIG_SOUND_KEY, GAME_CONFIG_MUSIC_KEY, 1);
    configSetInt(&gGameConfig, GAME_CONFIG_SOUND_KEY, GAME_CONFIG_SPEECH_KEY, 1);
    configSetInt(&gGameConfig, GAME_CONFIG_SOUND_KEY, GAME_CONFIG_MASTER_VOLUME_KEY, 22281);
    configSetInt(&gGameConfig, GAME_CONFIG_SOUND_KEY, GAME_CONFIG_MUSIC_VOLUME_KEY, 22281);
    configSetInt(&gGameConfig, GAME_CONFIG_SOUND_KEY, GAME_CONFIG_SNDFX_VOLUME_KEY, 22281);
    configSetInt(&gGameConfig, GAME_CONFIG_SOUND_KEY, GAME_CONFIG_SPEECH_VOLUME_KEY, 22281);
    configSetInt(&gGameConfig, GAME_CONFIG_SOUND_KEY, GAME_CONFIG_CACHE_SIZE_KEY, 448);
    configSetString(&gGameConfig, GAME_CONFIG_SOUND_KEY, GAME_CONFIG_MUSIC_PATH1_KEY, "sound\\music\\");
    configSetString(&gGameConfig, GAME_CONFIG_SOUND_KEY, GAME_CONFIG_MUSIC_PATH2_KEY, "sound\\music\\");
    configSetString(&gGameConfig, GAME_CONFIG_DEBUG_KEY, GAME_CONFIG_MODE_KEY, "environment");
    configSetInt(&gGameConfig, GAME_CONFIG_DEBUG_KEY, GAME_CONFIG_SHOW_TILE_NUM_KEY, 0);
    configSetInt(&gGameConfig, GAME_CONFIG_DEBUG_KEY, GAME_CONFIG_SHOW_SCRIPT_MESSAGES_KEY, 0);
    configSetInt(&gGameConfig, GAME_CONFIG_DEBUG_KEY, GAME_CONFIG_SHOW_LOAD_INFO_KEY, 0);
    configSetInt(&gGameConfig, GAME_CONFIG_DEBUG_KEY, GAME_CONFIG_OUTPUT_MAP_DATA_INFO_KEY, 0);

    if (isMapper) {
        configSetString(&gGameConfig, GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_EXECUTABLE_KEY, "mapper");
        configSetInt(&gGameConfig, GAME_CONFIG_MAPPER_KEY, GAME_CONFIG_OVERRIDE_LIBRARIAN_KEY, 0);
        configSetInt(&gGameConfig, GAME_CONFIG_MAPPER_KEY, GAME_CONFIG_LIBRARIAN_KEY, 0);
        configSetInt(&gGameConfig, GAME_CONFIG_MAPPER_KEY, GAME_CONFIG_USE_ART_NOT_PROTOS_KEY, 0);
        configSetInt(&gGameConfig, GAME_CONFIG_MAPPER_KEY, GAME_CONFIG_REBUILD_PROTOS_KEY, 0);
        configSetInt(&gGameConfig, GAME_CONFIG_MAPPER_KEY, GAME_CONFIG_FIX_MAP_OBJECTS_KEY, 0);
        configSetInt(&gGameConfig, GAME_CONFIG_MAPPER_KEY, GAME_CONFIG_FIX_MAP_INVENTORY_KEY, 0);
        configSetInt(&gGameConfig, GAME_CONFIG_MAPPER_KEY, GAME_CONFIG_IGNORE_REBUILD_ERRORS_KEY, 0);
        configSetInt(&gGameConfig, GAME_CONFIG_MAPPER_KEY, GAME_CONFIG_SHOW_PID_NUMBERS_KEY, 0);
        configSetInt(&gGameConfig, GAME_CONFIG_MAPPER_KEY, GAME_CONFIG_SAVE_TEXT_MAPS_KEY, 0);
        configSetInt(&gGameConfig, GAME_CONFIG_MAPPER_KEY, GAME_CONFIG_RUN_MAPPER_AS_GAME_KEY, 0);
        configSetInt(&gGameConfig, GAME_CONFIG_MAPPER_KEY, GAME_CONFIG_DEFAULT_F8_AS_GAME_KEY, 1);
        configSetInt(&gGameConfig, GAME_CONFIG_MAPPER_KEY, GAME_CONFIG_SORT_SCRIPT_LIST_KEY, 0);
    }

#ifndef NXDK
    // CE: Detect alternative default music directory.
    DbgPrint("[gameConfigInit] Detecting alternative music directory...\n");
    char alternativeMusicPath[COMPAT_MAX_PATH];
    strcpy(alternativeMusicPath, "data\\sound\\music\\*.acm");
    compat_windows_path_to_native(alternativeMusicPath);
    compat_resolve_path(alternativeMusicPath);
    DbgPrint("[gameConfigInit] Alternative music path: %s\n", alternativeMusicPath);
    char** acms;
    int acmsLength = fileNameListInit(alternativeMusicPath, &acms, 0, 0);
    if (acmsLength != -1) {
        if (acmsLength > 0) {
            configSetString(&gGameConfig, GAME_CONFIG_SOUND_KEY, GAME_CONFIG_MUSIC_PATH1_KEY, "data\\sound\\music\\");
            configSetString(&gGameConfig, GAME_CONFIG_SOUND_KEY, GAME_CONFIG_MUSIC_PATH2_KEY, "data\\sound\\music\\");
        }
        fileNameListFree(&acms, 0);
    }
    DbgPrint("[gameConfigInit] Alternative music directory detection done\n");
#else
    DbgPrint("[gameConfigInit] Skipping wildcard music detection under NXDK\n");
#endif

    DbgPrint("[gameConfigInit] Using config file: %s\n", gGameConfigFilePath);
    // SFALL: Custom config file name.
    char* customConfigFileName = nullptr;
    configGetString(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_CONFIG_FILE, &customConfigFileName);

    const char* configFileName = (customConfigFileName != nullptr && *customConfigFileName != '\0')
        ? customConfigFileName
        : DEFAULT_GAME_CONFIG_FILE_NAME;
    DbgPrint("[gameConfigInit] Using config file name: %s\n", configFileName);
#ifdef NXDK
    // On Xbox, use a fixed config path
    snprintf(gGameConfigFilePath, sizeof(gGameConfigFilePath), "E:\\UDATA\\FALLOUT2\\fallout2.cfg");
    DbgPrint("[gameConfigInit] Using fixed config path: %s\n", gGameConfigFilePath);
#else
    // Desktop-style dynamic path construction
    char* executable = argv[0];
    char* ch = strrchr(executable, '\\');
    if (ch != nullptr) {
        *ch = '\0';
        if (isMapper) {
            snprintf(gGameConfigFilePath,
                sizeof(gGameConfigFilePath),
                "%s\\%s",
                executable,
                MAPPER_CONFIG_FILE_NAME);
        } else {
            snprintf(gGameConfigFilePath,
                sizeof(gGameConfigFilePath),
                "%s\\%s",
                executable,
                configFileName);
        }
        *ch = '\\';
    } else {
        if (isMapper) {
            strcpy(gGameConfigFilePath, MAPPER_CONFIG_FILE_NAME);
        } else {
            strcpy(gGameConfigFilePath, configFileName);
        }
    }
#endif
    DbgPrint("[gameConfigInit] Config file path: %s\n", gGameConfigFilePath);

    // Read contents of `fallout2.cfg` into config. The values from the file
    // will override the defaults above.
    DbgPrint("[gameConfigInit] Reading config file: %s\n", gGameConfigFilePath);
    configRead(&gGameConfig, gGameConfigFilePath, false);

    // Add key-values from command line, which overrides both defaults and
    // whatever was loaded from `fallout2.cfg`.
    DbgPrint("[gameConfigInit] Parsing command line arguments\n");
    configParseCommandLineArguments(&gGameConfig, argc, argv);

    // CE: Normalize and resolve asset bundle paths.
    DbgPrint("[gameConfigInit] Resolving paths for game config keys\n");
    gameConfigResolvePath(GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_MASTER_DAT_KEY);
    gameConfigResolvePath(GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_MASTER_PATCHES_KEY);
    gameConfigResolvePath(GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_CRITTER_DAT_KEY);
    gameConfigResolvePath(GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_CRITTER_PATCHES_KEY);
    gameConfigResolvePath(GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_CRITTER_PATCHES_KEY);
    gameConfigResolvePath(GAME_CONFIG_SOUND_KEY, GAME_CONFIG_MUSIC_PATH1_KEY);
    gameConfigResolvePath(GAME_CONFIG_SOUND_KEY, GAME_CONFIG_MUSIC_PATH2_KEY);

    DbgPrint("[gameConfigInit] Game config paths resolved\n");
    gGameConfigInitialized = true;

    DbgPrint("[gameConfigInit] Game config initialized successfully\n");
    return true;
}

// Saves game config into `fallout2.cfg`.
//
// 0x444C14
bool gameConfigSave()
{
    if (!gGameConfigInitialized) {
        return false;
    }

    if (!configWrite(&gGameConfig, gGameConfigFilePath, false)) {
        return false;
    }

    return true;
}

// Frees game config, optionally saving it.
//
// 0x444C3C
bool gameConfigExit(bool shouldSave)
{
    if (!gGameConfigInitialized) {
        return false;
    }

    bool result = true;

    if (shouldSave) {
        if (!configWrite(&gGameConfig, gGameConfigFilePath, false)) {
            result = false;
        }
    }

    configFree(&gGameConfig);

    gGameConfigInitialized = false;

    return result;
}

static void gameConfigResolvePath(const char* section, const char* key)
{
    char* path;
    configGetString(&gGameConfig, section, key, &path);
    compat_windows_path_to_native(path);
    compat_resolve_path(path);
}

} // namespace fallout
