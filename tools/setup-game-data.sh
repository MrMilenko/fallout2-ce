#!/usr/bin/env bash
set -e

USAGE="Usage: $0 <installer.exe|installer.sh|install-dir> [output-dir]"
INPUT="$1"
OUTPUT="${2:-.}"
mkdir -p "$OUTPUT"

check_innoextract() {
    command -v innoextract &>/dev/null || {
        echo "innoextract is required for .exe installers. Install it:"
        echo "  macOS:  brew install innoextract"
        echo "  Ubuntu: sudo apt install innoextract"
        exit 1
    }
}

copy_game_files() {
    local src="$1"
    local base
    # -L follows symlinks (needed for GOG macOS Wineskin bundles)
    base=$(find -L "$src" -maxdepth 10 -iname "master.dat" -printf "%h\n" 2>/dev/null | head -1)
    # macOS find doesn't support -printf; fall back to a POSIX-compatible form
    if [ -z "$base" ]; then
        base=$(find -L "$src" -maxdepth 10 -iname "master.dat" 2>/dev/null | head -1 | xargs -I{} dirname {})
    fi
    [ -z "$base" ] && { echo "ERROR: Could not find master.dat in $src"; exit 1; }
    echo "Found game files at: $base"

    echo "Copying master.dat..."
    cp -v "$base/master.dat"  "$OUTPUT/"
    echo "Copying critter.dat..."
    cp -v "$base/critter.dat" "$OUTPUT/"
    echo "Copying data/ folder..."
    # Skip broken symlinks (e.g. GOG Wineskin SAVEGAME link)
    if command -v rsync &>/dev/null; then
        rsync -rv --safe-links "$base/data" "$OUTPUT/" || true
    else
        cp -rv "$base/data" "$OUTPUT/" 2>/dev/null || true
    fi

    # Copy patch files if present (official patches, mods)
    for f in "$base"/patch*.dat; do
        [ -e "$f" ] && cp -v "$f" "$OUTPUT/"
    done

    # Preserve our custom configs — never overwrite
    for cfg in ddraw.ini f2_res.ini fallout2.cfg; do
        if [ -f "$OUTPUT/$cfg" ]; then
            echo "Keeping existing $cfg (not overwritten)"
        elif [ -f "$base/$cfg" ]; then
            cp -v "$base/$cfg" "$OUTPUT/"
        fi
    done

    echo ""
    echo "=== Game data installed to: $OUTPUT ==="
    echo "You can now run the game!"
}

print_usage() {
    echo ""
    echo "Fallout 2: Community Edition - Game Data Installer"
    echo ""
    echo "This script copies the required retail Fallout 2 game files"
    echo "into the game directory. You need a legitimate copy of Fallout 2"
    echo "(GOG, Steam, or original CD)."
    echo ""
    echo "Supported sources:"
    echo "  Directory    Point to an existing Fallout 2 install folder"
    echo "  .exe         GOG/Steam offline installer (requires innoextract)"
    echo "  .dmg         GOG macOS installer (macOS only)"
    echo "  .sh          GOG Linux installer"
    echo ""
}

if [ -z "$INPUT" ]; then
    print_usage
    echo "$USAGE"
    exit 1
elif [ -d "$INPUT" ];          then copy_game_files "$INPUT"
elif [[ "$INPUT" == *.exe ]];  then check_innoextract
                                    TMP=$(mktemp -d); trap "rm -rf $TMP" EXIT
                                    innoextract --output-dir "$TMP" "$INPUT"
                                    copy_game_files "$TMP"
elif [[ "$INPUT" == *.dmg ]];  then command -v hdiutil &>/dev/null || {
                                        echo "ERROR: .dmg support requires macOS (hdiutil)"
                                        exit 1
                                    }
                                    MNT=$(mktemp -d); trap "hdiutil detach '$MNT' -quiet 2>/dev/null; rm -rf '$MNT'" EXIT
                                    echo "Mounting $INPUT..."
                                    hdiutil attach "$INPUT" -mountpoint "$MNT" -nobrowse -quiet
                                    copy_game_files "$MNT"
elif [[ "$INPUT" == *.sh ]];   then TMP=$(mktemp -d); trap "rm -rf $TMP" EXIT
                                    sh "$INPUT" --tar-extract "$TMP" || true
                                    copy_game_files "$TMP"
else print_usage
     echo "$USAGE"
     exit 1
fi
