#include "game_movie.h"

#include <stdio.h>
#include <string.h>

#include "color.h"
#include "cycle.h"
#include "debug.h"
#include "game.h"
#include "game_mouse.h"
#include "game_sound.h"
#include "input.h"
#include "mouse.h"
#include "movie.h"
#include "movie_effect.h"
#include "palette.h"
#include "platform_compat.h"
#include "settings.h"
#include "svga.h"
#include "text_font.h"
#include "touch.h"
#include "window_manager.h"

namespace fallout {

#define GAME_MOVIE_WINDOW_WIDTH 640
#define GAME_MOVIE_WINDOW_HEIGHT 480

static char* gameMovieBuildSubtitlesFilePath(char* movieFilePath);

// 0x50352A
static const float flt_50352A = 0.032258064f;

// 0x518DA0
static const char* gMovieFileNames[MOVIE_COUNT] = {
    "iplogo.mve",
    "intro.mve",
    "elder.mve",
    "vsuit.mve",
    "afailed.mve",
    "adestroy.mve",
    "car.mve",
    "cartucci.mve",
    "timeout.mve",
    "tanker.mve",
    "enclave.mve",
    "derrick.mve",
    "artimer1.mve",
    "artimer2.mve",
    "artimer3.mve",
    "artimer4.mve",
    "credits.mve",
};

// 0x518DE4
static const char* gMoviePaletteFilePaths[MOVIE_COUNT] = {
    nullptr,
    "art\\cuts\\introsub.pal",
    "art\\cuts\\eldersub.pal",
    nullptr,
    "art\\cuts\\artmrsub.pal",
    nullptr,
    nullptr,
    nullptr,
    "art\\cuts\\artmrsub.pal",
    nullptr,
    nullptr,
    nullptr,
    "art\\cuts\\artmrsub.pal",
    "art\\cuts\\artmrsub.pal",
    "art\\cuts\\artmrsub.pal",
    "art\\cuts\\artmrsub.pal",
    "art\\cuts\\crdtssub.pal",
};

// 0x518E28
static bool gGameMovieIsPlaying = false;

// 0x518E2C
static bool gGameMovieFaded = false;

// 0x596C78
static unsigned char gGameMoviesSeen[MOVIE_COUNT];

// 0x596C89
static char gGameMovieSubtitlesFilePath[COMPAT_MAX_PATH];

// gmovie_init
// 0x44E5C0
int gameMoviesInit()
{
    int v1 = 0;
    if (backgroundSoundIsEnabled()) {
        v1 = backgroundSoundGetVolume();
    }

    movieSetVolume(v1);

    movieSetBuildSubtitleFilePathProc(gameMovieBuildSubtitlesFilePath);

    memset(gGameMoviesSeen, 0, sizeof(gGameMoviesSeen));

    gGameMovieIsPlaying = false;
    gGameMovieFaded = false;

    return 0;
}

// 0x44E60C
void gameMoviesReset()
{
    memset(gGameMoviesSeen, 0, sizeof(gGameMoviesSeen));

    gGameMovieIsPlaying = false;
    gGameMovieFaded = false;
}

// 0x44E638
int gameMoviesLoad(File* stream)
{
    if (fileRead(gGameMoviesSeen, sizeof(*gGameMoviesSeen), MOVIE_COUNT, stream) != MOVIE_COUNT) {
        return -1;
    }

    return 0;
}

// 0x44E664
int gameMoviesSave(File* stream)
{
    if (fileWrite(gGameMoviesSeen, sizeof(*gGameMoviesSeen), MOVIE_COUNT, stream) != MOVIE_COUNT) {
        return -1;
    }

    return 0;
}

// gmovie_play
// 0x44E690
int gameMoviePlay(int movie, int flags)
{
    gGameMovieIsPlaying = true;

    const char* movieFileName = gMovieFileNames[movie];
    DbgPrint("\nPlaying movie: %s\n", movieFileName);

    const char* language = settings.system.language.c_str();
    char movieFilePath[COMPAT_MAX_PATH];
    int movieFileSize;
    bool movieFound = false;

    if (compat_stricmp(language, ENGLISH) != 0) {
        snprintf(movieFilePath, sizeof(movieFilePath), "art\\%s\\cuts\\%s", language, gMovieFileNames[movie]);
        movieFound = dbGetFileSize(movieFilePath, &movieFileSize) == 0;
    }

    if (!movieFound) {
        snprintf(movieFilePath, sizeof(movieFilePath), "art\\cuts\\%s", gMovieFileNames[movie]);
        movieFound = dbGetFileSize(movieFilePath, &movieFileSize) == 0;
    }

    if (!movieFound) {
        debugPrint("\ngmovie_play() - Error: Unable to open %s\n", gMovieFileNames[movie]);
        gGameMovieIsPlaying = false;
        return -1;
    }

    if ((flags & GAME_MOVIE_FADE_IN) != 0) {
        DbgPrint("\ngmovie_play() - Fading in\n");
        paletteFadeTo(gPaletteBlack);
        gGameMovieFaded = true;
    }
    int gameMovieWindowX = (screenGetWidth() - GAME_MOVIE_WINDOW_WIDTH) / 2;
    int gameMovieWindowY = (screenGetHeight() - GAME_MOVIE_WINDOW_HEIGHT) / 2;
    DbgPrint("\ngmovie_play() - windowCreate at (%d, %d)\n", gameMovieWindowX, gameMovieWindowY);
    int win = windowCreate(gameMovieWindowX,
        gameMovieWindowY,
        GAME_MOVIE_WINDOW_WIDTH,
        GAME_MOVIE_WINDOW_HEIGHT,
        0,
        WINDOW_MODAL);
    if (win == -1) {
        gGameMovieIsPlaying = false;
        DbgPrint("\ngmovie_play() - Error: Unable to create movie window\n");
        return -1;
    }

    if ((flags & GAME_MOVIE_STOP_MUSIC) != 0) {
        backgroundSoundDelete();
    } else if ((flags & GAME_MOVIE_PAUSE_MUSIC) != 0) {
        backgroundSoundPause();
    }
    DbgPrint("\ngmovie_play() - windowRefresh\n");
    windowRefresh(win);

    bool subtitlesEnabled = settings.preferences.subtitles;
    int v1 = 4;
    if (subtitlesEnabled) {
        char* subtitlesFilePath = gameMovieBuildSubtitlesFilePath(movieFilePath);

        int subtitlesFileSize;
        if (dbGetFileSize(subtitlesFilePath, &subtitlesFileSize) == 0) {
            v1 = 12;
        } else {
            subtitlesEnabled = false;
        }
    }
    DbgPrint("\ngmovie_play() - movieSetFlags\n");
    movieSetFlags(v1);

    int oldTextColor;
    int oldFont;
    if (subtitlesEnabled) {
        const char* subtitlesPaletteFilePath;
        if (gMoviePaletteFilePaths[movie] != nullptr) {
            subtitlesPaletteFilePath = gMoviePaletteFilePaths[movie];
        } else {
            subtitlesPaletteFilePath = "art\\cuts\\subtitle.pal";
        }
        DbgPrint("\ngmovie_play() - colorPaletteLoad\n");
        colorPaletteLoad(subtitlesPaletteFilePath);
        DbgPrint("\ngmovie_play() - windowSetTextColor\n");
        oldTextColor = windowGetTextColor();
        windowSetTextColor(1.0, 1.0, 1.0);
        DbgPrint("\ngmovie_play() - windowSetFont\n");
        oldFont = fontGetCurrent();
        windowSetFont(101);
    }
    DbgPrint("\ngmovie_play() - mouse shit\n");
    bool cursorWasHidden = cursorIsHidden();
    if (cursorWasHidden) {
        gameMouseSetCursor(MOUSE_CURSOR_NONE);
        mouseShowCursor();
    }

    while (mouseGetEvent() != 0) {
        _mouse_info();
    }

    mouseHideCursor();
    DbgPrint("\ngmovie_play() - mouse shit done\n");
    DbgPrint("\ngmovie_play() - colorCycleDisable()\n");
    colorCycleDisable();
    DbgPrint("\ngmovie_play() - colorCycleDisable() finished\n");
    DbgPrint("\ngmovie_play() - movieEffectsLoad\n");
    movieEffectsLoad(movieFilePath);
    DbgPrint("\ngmovie_play() - movieEffectsLoad finished\n");

    DbgPrint("\ngmovie_play() - _zero_vid_mem\n");
    _zero_vid_mem();
    DbgPrint("\ngmovie_play() - _movieRun\n");
    _movieRun(win, movieFilePath);
    DbgPrint("\ngmovie_play() - _movieRun finished\n");

    int v11 = 0;
    int buttons;
    do {
        if (!_moviePlaying() || _game_user_wants_to_quit || inputGetInput() != -1) {
            break;
        }

        Gesture gesture;
        if (touch_get_gesture(&gesture) && gesture.state == kEnded) {
            break;
        }

        int x;
        int y;
        _mouse_get_raw_state(&x, &y, &buttons);

        v11 |= buttons;
    } while (((v11 & 1) == 0 && (v11 & 2) == 0) || (buttons & 1) != 0 || (buttons & 2) != 0);

    DbgPrint("gmovie_play() - calling _movieStop()\n");
    _movieStop();
    DbgPrint("gmovie_play() - _movieStop() done\n");

    DbgPrint("gmovie_play() - calling _moviefx_stop()\n");
    _moviefx_stop();
    DbgPrint("gmovie_play() - _moviefx_stop() done\n");

    DbgPrint("gmovie_play() - calling _movieUpdate()\n");
    _movieUpdate();
    DbgPrint("gmovie_play() - _movieUpdate() done\n");

    DbgPrint("gmovie_play() - calling paletteSetEntries(gPaletteBlack)\n");
    DbgPrint("gPaletteBlack = %p\n", gPaletteBlack);
    paletteSetEntries(gPaletteBlack);
    DbgPrint("gmovie_play() - paletteSetEntries() done\n");

    gGameMoviesSeen[movie] = 1;
    DbgPrint("gmovie_play() - marked movie %d as seen\n", movie);

    colorCycleEnable();
    DbgPrint("gmovie_play() - colorCycleEnable()\n");

    gameMouseSetCursor(MOUSE_CURSOR_ARROW);
    DbgPrint("gmovie_play() - gameMouseSetCursor(ARROW)\n");

    if (!cursorWasHidden) {
        DbgPrint("gmovie_play() - cursor was not hidden, showing cursor\n");
        mouseShowCursor();
    }

    if (subtitlesEnabled) {
        DbgPrint("gmovie_play() - subtitles were enabled, restoring text settings\n");

        colorPaletteLoad("color.pal");
        DbgPrint("gmovie_play() - colorPaletteLoad(\"color.pal\") done\n");

        windowSetFont(oldFont);
        DbgPrint("gmovie_play() - windowSetFont(%d) done\n", oldFont);

        float r = (float)((Color2RGB(oldTextColor) & 0x7C00) >> 10) * flt_50352A;
        float g = (float)((Color2RGB(oldTextColor) & 0x3E0) >> 5) * flt_50352A;
        float b = (float)(Color2RGB(oldTextColor) & 0x1F) * flt_50352A;
        DbgPrint("gmovie_play() - restoring text color: RGB = %.2f %.2f %.2f\n", r, g, b);
        windowSetTextColor(r, g, b);
    }

    DbgPrint("gmovie_play() - destroying movie window\n");
    windowDestroy(win);
    DbgPrint("gmovie_play() - windowDestroy() done\n");

    DbgPrint("gmovie_play() - refreshing all windows\n");
    windowRefreshAll(&_scr_size);
    DbgPrint("gmovie_play() - windowRefreshAll() done\n");

    if ((flags & GAME_MOVIE_PAUSE_MUSIC) != 0) {
        DbgPrint("gmovie_play() - resuming background music\n");
        backgroundSoundResume();
    }

    if ((flags & GAME_MOVIE_FADE_OUT) != 0) {
        if (!subtitlesEnabled) {
            DbgPrint("gmovie_play() - colorPaletteLoad(\"color.pal\") before fade out\n");
            colorPaletteLoad("color.pal");
        }

        DbgPrint("gmovie_play() - paletteFadeTo(_cmap)\n");
        paletteFadeTo(_cmap);
        gGameMovieFaded = false;
    }

    gGameMovieIsPlaying = false;
    DbgPrint("gmovie_play() - movie finished and cleaned up\n");

    return 0;
}

// 0x44EAE4
void gameMovieFadeOut()
{
    if (gGameMovieFaded) {
        paletteFadeTo(_cmap);
        gGameMovieFaded = false;
    }
}

// 0x44EB04
bool gameMovieIsSeen(int movie)
{
    return gGameMoviesSeen[movie] == 1;
}

// 0x44EB14
bool gameMovieIsPlaying()
{
    return gGameMovieIsPlaying;
}

// 0x44EB1C
static char* gameMovieBuildSubtitlesFilePath(char* movieFilePath)
{
    char* path = movieFilePath;

    char* separator = strrchr(path, '\\');
    if (separator != nullptr) {
        path = separator + 1;
    }

    snprintf(gGameMovieSubtitlesFilePath, sizeof(gGameMovieSubtitlesFilePath), "text\\%s\\cuts\\%s", settings.system.language.c_str(), path);

    char* pch = strrchr(gGameMovieSubtitlesFilePath, '.');
    if (*pch != '\0') {
        *pch = '\0';
    }

    strcpy(gGameMovieSubtitlesFilePath + strlen(gGameMovieSubtitlesFilePath), ".SVE");

    return gGameMovieSubtitlesFilePath;
}

} // namespace fallout
