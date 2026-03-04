#include "platform_compat.h"

#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef _WIN32
#include <direct.h>
#ifndef NXDK
#include <io.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#ifndef NXDK
#ifndef _O_RDONLY
#define _O_RDONLY 0x0000
#endif
#ifndef O_WRONLY
#define O_WRONLY 0x01
#endif
#endif

#if defined(_WIN32) && !defined(NXDK)
#include <timeapi.h>
#else
#include <chrono>
#endif

#include <SDL.h>

namespace fallout {

int compat_stricmp(const char* string1, const char* string2)
{
    return SDL_strcasecmp(string1, string2);
}

int compat_strnicmp(const char* string1, const char* string2, size_t size)
{
    return SDL_strncasecmp(string1, string2, size);
}

char* compat_strupr(char* string)
{
    return SDL_strupr(string);
}

char* compat_strlwr(char* string)
{
    return SDL_strlwr(string);
}

char* compat_itoa(int value, char* buffer, int radix)
{
    return SDL_itoa(value, buffer, radix);
}

void compat_splitpath(const char* path, char* drive, char* dir, char* fname, char* ext)
{
#if defined(_WIN32) && !defined(NXDK)
    _splitpath(path, drive, dir, fname, ext);
#else
    const char* driveStart = path;
    if (path[0] == '/' && path[1] == '/') {
        path += 2;
        while (*path != '\0' && *path != '/' && *path != '.') {
            path++;
        }
    }

    if (drive != nullptr) {
        size_t driveSize = path - driveStart;
        if (driveSize > COMPAT_MAX_DRIVE - 1) {
            driveSize = COMPAT_MAX_DRIVE - 1;
        }
        strncpy(drive, path, driveSize);
        drive[driveSize] = '\0';
    }

    const char* dirStart = path;
    const char* fnameStart = path;
    const char* extStart = nullptr;

    const char* end = path;
    while (*end != '\0') {
        if (*end == '/') {
            fnameStart = end + 1;
        } else if (*end == '.') {
            extStart = end;
        }
        end++;
    }

    if (extStart == nullptr) {
        extStart = end;
    }

    if (dir != nullptr) {
        size_t dirSize = fnameStart - dirStart;
        if (dirSize > COMPAT_MAX_DIR - 1) {
            dirSize = COMPAT_MAX_DIR - 1;
        }
        strncpy(dir, path, dirSize);
        dir[dirSize] = '\0';
    }

    if (fname != nullptr) {
        size_t fileNameSize = extStart - fnameStart;
        if (fileNameSize > COMPAT_MAX_FNAME - 1) {
            fileNameSize = COMPAT_MAX_FNAME - 1;
        }
        strncpy(fname, fnameStart, fileNameSize);
        fname[fileNameSize] = '\0';
    }

    if (ext != nullptr) {
        size_t extSize = end - extStart;
        if (extSize > COMPAT_MAX_EXT - 1) {
            extSize = COMPAT_MAX_EXT - 1;
        }
        strncpy(ext, extStart, extSize);
        ext[extSize] = '\0';
    }
#endif
}

void compat_makepath(char* path, const char* drive, const char* dir, const char* fname, const char* ext)
{
#if defined(_WIN32) && !defined(NXDK)
    _makepath(path, drive, dir, fname, ext);
#else
    path[0] = '\0';

    if (drive != nullptr) {
        if (*drive != '\0') {
            strcpy(path, drive);
            path = strchr(path, '\0');

            if (path[-1] == '/') {
                path--;
            } else {
                *path = '/';
            }
        }
    }

    if (dir != nullptr) {
        if (*dir != '\0') {
            if (*dir != '/' && *path == '/') {
                path++;
            }

            strcpy(path, dir);
            path = strchr(path, '\0');

            if (path[-1] == '/') {
                path--;
            } else {
                *path = '/';
            }
        }
    }

    if (fname != nullptr && *fname != '\0') {
        if (*fname != '/' && *path == '/') {
            path++;
        }

        strcpy(path, fname);
        path = strchr(path, '\0');
    } else {
        if (*path == '/') {
            path++;
        }
    }

    if (ext != nullptr) {
        if (*ext != '\0') {
            if (*ext != '.') {
                *path++ = '.';
            }

            strcpy(path, ext);
            path = strchr(path, '\0');
        }
    }

    *path = '\0';
#endif
}

int compat_open(const char* path, int flags)
{
#ifdef NXDK
    char nativePath[COMPAT_MAX_PATH];
    strcpy(nativePath, path);
    compat_windows_path_to_native(nativePath);
    const char* mode = (flags & 0x01) ? "wb" : "rb";
    FILE* f = fopen(nativePath, mode);
    if (f == nullptr) return -1;
    return static_cast<int>(reinterpret_cast<intptr_t>(f));
#else
    char nativePath[COMPAT_MAX_PATH];
    strcpy(nativePath, path);
    compat_windows_path_to_native(nativePath);
    const char* mode = (flags & 0x01) ? "wb" : "rb";
    FILE* f = fopen(nativePath, mode);
    if (f == nullptr) return -1;
    return static_cast<int>(reinterpret_cast<intptr_t>(f));
#endif
}

int compat_close(int fileHandle)
{
    FILE* f = reinterpret_cast<FILE*>(static_cast<intptr_t>(fileHandle));
    return fclose(f);
}

int compat_read(int fileHandle, void* buf, unsigned int size)
{
    FILE* f = reinterpret_cast<FILE*>(static_cast<intptr_t>(fileHandle));
    return static_cast<int>(fread(buf, 1, size, f));
}

int compat_write(int fileHandle, const void* buf, unsigned int size)
{
    FILE* f = reinterpret_cast<FILE*>(static_cast<intptr_t>(fileHandle));
    return static_cast<int>(fwrite(buf, 1, size, f));
}

long compat_lseek(int fileHandle, long offset, int origin)
{
    FILE* f = reinterpret_cast<FILE*>(static_cast<intptr_t>(fileHandle));
    fseek(f, offset, origin);
    return ftell(f);
}

long compat_tell(int fd)
{
    return compat_lseek(fd, 0, SEEK_CUR);
}

long compat_filelength(int fd)
{
    long originalOffset = compat_lseek(fd, 0, SEEK_CUR);
    compat_lseek(fd, 0, SEEK_SET);
    long filesize = compat_lseek(fd, 0, SEEK_END);
    compat_lseek(fd, originalOffset, SEEK_SET);
    return filesize;
}

int compat_mkdir(const char* path)
{
    char nativePath[COMPAT_MAX_PATH];
    strcpy(nativePath, path);
    compat_windows_path_to_native(nativePath);

#ifdef NXDK
    BOOL result = CreateDirectoryA(nativePath, NULL);
    return result ? 0 : -1;
#else
    compat_resolve_path(nativePath);
#ifdef _WIN32
    return mkdir(nativePath);
#else
    return mkdir(nativePath, 0755);
#endif
#endif
}

unsigned int compat_timeGetTime()
{
#ifdef NXDK
    return GetTickCount();
#elif defined(_WIN32)
    return timeGetTime();
#else
    static auto start = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    return static_cast<unsigned int>(std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count());
#endif
}

FILE* compat_fopen(const char* path, const char* mode)
{
    char nativePath[COMPAT_MAX_PATH];
    strcpy(nativePath, path);
    compat_windows_path_to_native(nativePath);

#ifdef NXDK
    // NXDK doesn't support 't' mode flag; strip it
    char cleanMode[8];
    int j = 0;
    for (int i = 0; mode[i] && j < 7; i++) {
        if (mode[i] != 't') {
            cleanMode[j++] = mode[i];
        }
    }
    cleanMode[j] = '\0';
    return fopen(nativePath, cleanMode);
#else
    compat_resolve_path(nativePath);
    return fopen(nativePath, mode);
#endif
}

#ifndef Z_SOLO
gzFile compat_gzopen(const char* path, const char* mode)
{
    char nativePath[COMPAT_MAX_PATH];
    strcpy(nativePath, path);
    compat_windows_path_to_native(nativePath);
    compat_resolve_path(nativePath);
    return gzopen(nativePath, mode);
}
#endif

char* compat_fgets(char* buffer, int maxCount, FILE* stream)
{
    buffer = fgets(buffer, maxCount, stream);

    if (buffer != nullptr) {
        size_t len = strlen(buffer);
        if (len >= 2 && buffer[len - 1] == '\n' && buffer[len - 2] == '\r') {
            buffer[len - 2] = '\n';
            buffer[len - 1] = '\0';
        }
    }

    return buffer;
}

#ifndef Z_SOLO
char* compat_gzgets(gzFile stream, char* buffer, int maxCount)
{
    buffer = gzgets(stream, buffer, maxCount);

    if (buffer != nullptr) {
        size_t len = strlen(buffer);
        if (len >= 2 && buffer[len - 1] == '\n' && buffer[len - 2] == '\r') {
            buffer[len - 2] = '\n';
            buffer[len - 1] = '\0';
        }
    }

    return buffer;
}
#endif

int compat_remove(const char* path)
{
    char nativePath[COMPAT_MAX_PATH];
    strcpy(nativePath, path);
    compat_windows_path_to_native(nativePath);
    compat_resolve_path(nativePath);
    return remove(nativePath);
}

int compat_rename(const char* oldFileName, const char* newFileName)
{
    char nativeOldFileName[COMPAT_MAX_PATH];
    strcpy(nativeOldFileName, oldFileName);
    compat_windows_path_to_native(nativeOldFileName);
    compat_resolve_path(nativeOldFileName);

    char nativeNewFileName[COMPAT_MAX_PATH];
    strcpy(nativeNewFileName, newFileName);
    compat_windows_path_to_native(nativeNewFileName);
    compat_resolve_path(nativeNewFileName);

    return rename(nativeOldFileName, nativeNewFileName);
}

void compat_windows_path_to_native(char* path)
{
#ifndef _WIN32
    char* pch = path;
    while (*pch != '\0') {
        if (*pch == '\\') {
            *pch = '/';
        }
        pch++;
    }
#endif
}

void compat_resolve_path(char* path)
{
#ifndef _WIN32
    char* pch = path;

    DIR* dir;
    if (pch[0] == '/') {
        dir = opendir("/");
        pch++;
    } else {
        dir = opendir(".");
    }

    while (dir != nullptr) {
        char* sep = strchr(pch, '/');
        size_t length;
        if (sep != nullptr) {
            length = sep - pch;
        } else {
            length = strlen(pch);
        }

        bool found = false;

        struct dirent* entry = readdir(dir);
        while (entry != nullptr) {
            if (strlen(entry->d_name) == length && compat_strnicmp(pch, entry->d_name, length) == 0) {
                strncpy(pch, entry->d_name, length);
                found = true;
                break;
            }
            entry = readdir(dir);
        }

        closedir(dir);
        dir = nullptr;

        if (!found) {
            break;
        }

        if (sep == nullptr) {
            break;
        }

        *sep = '\0';
        dir = opendir(path);
        *sep = '/';

        pch = sep + 1;
    }
#endif
}

int compat_access(const char* path, int mode)
{
    char nativePath[COMPAT_MAX_PATH];
    strcpy(nativePath, path);
    compat_windows_path_to_native(nativePath);

#ifdef NXDK
    DWORD attrs = GetFileAttributesA(nativePath);
    return (attrs != INVALID_FILE_ATTRIBUTES) ? 0 : -1;
#else
    compat_resolve_path(nativePath);
    return access(nativePath, mode);
#endif
}

char* compat_strdup(const char* string)
{
    return SDL_strdup(string);
}

// It's a replacement for compat_filelength(fileno(stream)) on platforms without
// fileno defined.
long getFileSize(FILE* stream)
{
    long originalOffset = ftell(stream);
    fseek(stream, 0, SEEK_END);
    long filesize = ftell(stream);
    fseek(stream, originalOffset, SEEK_SET);
    return filesize;
}

} // namespace fallout
