# Fallout 2: Community Edition — Xbox Port

[![Build](https://github.com/MrMilenko/fallout2-ce/actions/workflows/ci-build.yml/badge.svg)](https://github.com/MrMilenko/fallout2-ce/actions/workflows/ci-build.yml)
[![Latest Release](https://img.shields.io/github/v/release/MrMilenko/fallout2-ce)](https://github.com/MrMilenko/fallout2-ce/releases/latest)

A fork of [Fallout 2 Community Edition](https://github.com/alexbatalov/fallout2-ce) adding:

- **Original Xbox support** — built with [NXDK](https://github.com/XboxDev/nxdk) and SDL
- **Steam Deck support** — packaged as an AppImage at 1280x800
- **Basic controller support** — left stick moves the cursor, face buttons for common actions. Fallout 2 is a mouse-driven game so this is limited.

Some Xbox build infrastructure was adapted from [Justy's fallout1-ce Xbox port](https://github.com/jroc-hb/fallout1-ce-xbox).

## Status

Working:
- FMVs (on real hardware)
- Audio
- Save system
- Basic joystick/controller input

TODO:
- Quite a bit! See [Known Issues](#known-issues).

---

## A Note About This Project

This Xbox port was originally created by [@MrMilenko](https://github.com/MrMilenko) — the platform compat layer, controller mapping, NXDK integration, audio/video init, and drive mounting are all hand-written work from building and testing on real Xbox hardware.

During the process of cleaning up the git history and organizing the codebase for public release, [Claude Code](https://claude.com/claude-code) (Anthropic's AI coding assistant) was used to help with:

- Reorganizing commits into a clean, reviewable merge from upstream
- Building the CI/CD pipeline (GitHub Actions for all 5 platforms)
- Fixing cross-platform build issues (Z_SOLO guards, CMake compat, etc.)
- Writing setup scripts, packaging configs, and this README

The game code itself — the Xbox port, controller input, platform abstractions — is human-written. The AI helped ship it cleanly.

---

## Getting the Game Files

You need `master.dat`, `critter.dat`, and the `data/` folder from a legitimate copy of Fallout 2 ([GOG](https://www.gog.com/game/fallout_2) · [Steam](https://store.steampowered.com/app/38410) · [Epic](https://store.epicgames.com/p/fallout-2)).

Each release includes a setup script that copies the retail files for you:

```sh
# Linux / macOS / Steam Deck
bash setup-game-data.sh <source> [output-dir]

# Windows
setup-game-data.bat <fallout2-install-directory> [output-dir]
```

Supported sources (shell script):
- **Directory** — an existing Fallout 2 install folder
- **`.exe`** — GOG/Steam offline installer (requires [`innoextract`](https://constexpr.org/innoextract/))
- **`.dmg`** — GOG macOS installer (macOS only)
- **`.sh`** — GOG Linux installer

The script will never overwrite your existing config files (`ddraw.ini`, `f2_res.ini`, `fallout2.cfg`).

---

## Installation

### Windows

1. Download `fallout2-ce-windows.zip` from the [latest release](https://github.com/MrMilenko/fallout2-ce/releases/latest) and unzip it
2. Run `setup-game-data.bat "C:\GOG Games\Fallout 2"` (or wherever your copy is installed), or manually copy `master.dat`, `critter.dat`, and `data/` into the same folder as `fallout2-ce.exe`
3. Run `fallout2-ce.exe`

### macOS

1. Download `fallout2-ce-macos.zip` from the [latest release](https://github.com/MrMilenko/fallout2-ce/releases/latest) and unzip it
2. Move the `.app` to Applications (or anywhere you like)
3. Install game data to `~/Games/Fallout 2 CE/`:
   ```sh
   bash setup-game-data.sh ~/Downloads/fallout_2_2.0.0.4.dmg ~/Games/"Fallout 2 CE"
   # or point it at a directory, .exe, or .sh installer
   ```
4. Launch the app — it automatically looks in `~/Games/Fallout 2 CE/` for game files

### Linux

1. Download `fallout2-ce-linux-x64.tar.gz` from the [latest release](https://github.com/MrMilenko/fallout2-ce/releases/latest) and extract it
2. Run `bash setup-game-data.sh <source>` to copy game files next to the binary, or do it manually
3. Run `./fallout2-ce`

### Steam Deck

1. Download `fallout2-ce-steam-deck.zip` from the [latest release](https://github.com/MrMilenko/fallout2-ce/releases/latest) and unzip it
2. Switch to Desktop Mode. Run `bash setup-game-data.sh <source>` to copy game files into the same folder as the AppImage
3. `chmod +x Fallout2-CE-SteamDeck-x86_64.AppImage` and run it — or add to Steam as a Non-Steam game
4. The game defaults to 1280x800 for the Deck screen

### Xbox

Copy `f2_res.ini`, `fallout2.cfg`, `ddraw.ini`, `master.dat`, `critter.dat`, and `default.xbe` along with the `data` and `sound` folders to your Xbox.

#### Running from disc / ISO

The `fallout2-ce/` directory in this repo is packed into the ISO automatically by the build. `master.dat` and `critter.dat` go on the DVD (`D:\`), while the `data\` folder goes on the hard drive at `E:\UDATA\FALLOUT2\data`.

#### Running from HDD (softmod/chip)

Copy everything to a folder on `E:\` (e.g. `E:\Games\Fallout2\`). `D:\` automatically maps to the directory containing `default.xbe` — no config changes needed.

#### Edge Cases

You may need to extract a few files from `master.dat` using [DAT Explorer](https://fallout.fandom.com/wiki/Resources:DAT_Explorer_Guide):

- `pro_crit.msg`
- `pro_item.msg`
- `pro_scen.msg`
- `pro_tile.msg`
- `pro_wall.msg`

Place them in `E:\UDATA\FALLOUT2\data\text\english\game\`. This bug has appeared in some builds intermittently.

---

## Building

### Desktop (Windows / Linux / macOS)

Standard CMake build — same as upstream:

```sh
cmake -B build -S .
cmake --build build
```

Place `master.dat`, `critter.dat`, and the `data/` folder from your Fallout 2 installation next to the binary (Linux/Windows) or in `~/Games/Fallout 2 CE/` (macOS).

### Xbox (NXDK)

Set up the NXDK toolchain, then:

```sh
export NXDK_DIR=/path/to/nxdk
export PATH=$NXDK_DIR/bin:$PATH
nxdk-cmake -B build-nxdk -S . -DNXDK_DIR=$NXDK_DIR
cmake --build build-nxdk
```

This produces `fallout2-ce.iso`, which can be burned to a disc or loaded in [xemu](https://xemu.app).

> **Note:** Several NXDK libraries must be built once before the first build (`libc++.lib`, `libSDL2.lib`). See the NXDK documentation for details.

---

## Configuration

Each release ships with default config files. The setup script will never overwrite them if they already exist.

| File | Purpose |
|---|---|
| `fallout2.cfg` | Game settings (data paths, sound, language) |
| `f2_res.ini` | Resolution and display settings |
| `ddraw.ini` | Sfall configuration (engine tweaks) |

### Xbox-specific notes

The Xbox build uses different data paths (`D:\` for DVD, `E:\UDATA\FALLOUT2\` for HDD). The CI build handles this automatically.

The original Xbox outputs 480p (640x480) by default. Higher resolutions may work depending on your TV and cable, but 640x480 is the safe default.

---

## Controller Support

Basic joystick support is available on all platforms. Fallout 2 was designed for mouse and keyboard, so controller support is limited — many actions (naming characters, free text input, precise cursor work) still require a keyboard.

| Button | Action |
|---|---|
| **Left stick** | Move mouse cursor |
| **A** | Left mouse click |
| **B** | Right mouse click |
| **X** | Inventory |
| **Y** | Pip-Boy |
| **Back** | Character sheet |
| **Start** | ESC |
| **D-Pad Up / Down** | Scroll |
| **D-Pad Left** | Skilldex |
| **D-Pad Right** | Quick Save |
| **White (LB)** | End Turn |
| **Black (RB)** | End Combat |

Sensitivity and deadzone are hardcoded in `src/dinput.cc`.

---

## Known Issues

### Xbox
- Various graphical glitches, some paths may need correcting
- Third character preset in character creation is blank (no portrait) — pre-existing, cause unknown
- Some scripts may terminate early (e.g. Christa is selectable but shows no info)

### Xbox / xemu
- **FMVs crash xemu** — they work on real hardware. Set `disable_fmv=1` under `[debug]` in `fallout2.cfg` when testing in xemu.
- **Black screen after character selection** — works on real hardware, emulator limitation.

### All Platforms
- Many actions still require a keyboard — controller support is basic
- Right stick is read but not mapped to any action

### macOS
- FMVs play but the first frame may flash briefly (cosmetic)

---

## License

Source code in this repository is available under the [Sustainable Use License](LICENSE.md).
