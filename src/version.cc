#include "version.h"
#include "sfall_config.h"

#include <stdio.h>

namespace fallout {

// 0x4B4580
void versionGetVersion(char* dest, size_t size)
{
    // SFALL: custom version string.
    char* versionString = nullptr;
    if (configGetString(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_VERSION_STRING, &versionString)) {
        if (*versionString == '\0') {
            versionString = nullptr;
        }
    }
#ifdef NXDK
    snprintf(dest, size, (versionString != nullptr ? versionString : "FALLOUT II %d.%02d-Milenko-Xbox"), VERSION_MAJOR, VERSION_MINOR);
#elif __APPLE__
    snprintf(dest, size, (versionString != nullptr ? versionString : "FALLOUT II %d.%02d-Milenko-macOS"), VERSION_MAJOR, VERSION_MINOR);
#elif __linux__
    snprintf(dest, size, (versionString != nullptr ? versionString : "FALLOUT II %d.%02d-Milenko-Linux"), VERSION_MAJOR, VERSION_MINOR);
#elif _WIN32
    snprintf(dest, size, (versionString != nullptr ? versionString : "FALLOUT II %d.%02d-Milenko-Windows"), VERSION_MAJOR, VERSION_MINOR);
#else
    snprintf(dest, size, (versionString != nullptr ? versionString : "FALLOUT II %d.%02d-Milenko"), VERSION_MAJOR, VERSION_MINOR);
#endif
}

} // namespace fallout
