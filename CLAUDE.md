# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

MP3 player application for the M5Stack Cardputer (ESP32-S3). Supports MP3, FLAC, AAC, and WAV playback from SD card, with Wi-Fi streaming, audio visualizers, and a full settings system.

## Build & Flash

**IDE:** Arduino IDE (no PlatformIO)

1. Select board: **M5Stack Cardputer** (Tools → Board)
2. Install libraries via Library Manager:
   - M5Cardputer (by M5Stack)
   - M5Unified (by M5Stack)
   - ESP8266Audio v1.9.7+ (by Earle F. Philhower III)
3. Open `MP3PlayerM5Cardputer/MP3PlayerM5Cardputer.ino`
4. Connect Cardputer via USB-C, click Upload

**SD Card:** FAT32 formatted, MP3/FLAC/AAC/WAV files in root or subfolders.

## Architecture

**Single-file application** — all code lives in `MP3PlayerM5Cardputer/MP3PlayerM5Cardputer.ino` (~2,350 lines). No separate .h/.cpp files.

### Core Classes (all defined in the .ino)

- **AudioEngine** — Playback pipeline, playlist management, seeking. Pipeline: `AudioFileSourceSD → AudioFileSourceBuffer (16KB) → AudioFileSourceID3 → AudioGenerator* → AudioOutputM5Speaker`
- **UIManager** — All rendering, visual state, sprite-based visualizations
- **ConfigManager** — Settings persistence via ESP32 NVS (`Preferences` API, namespace `sam_music`) and SD card export/import (`/config.txt`)
- **WebServerManager** — HTTP server for remote streaming (endpoints: `/`, `/api/songs`, `/stream?id=`, `/download?id=`)
- **AudioOutputM5Speaker** — Custom output with triple-buffering (3 × 2048 bytes)
- **fft_t** — 256-point FFT for audio visualizer

### UI State Machine

States: `UI_PLAYER`, `UI_SETTINGS`, `UI_HELP`, `UI_WIFI_SCAN`, `UI_TEXT_INPUT`, `UI_SEARCH`, `UI_FOLDER_SELECT`

### Playlist Caching

- `playlist.txt` — flat index of all songs with byte offsets for O(1) random access
- `pl_FolderName.txt` — per-folder cached playlists
- First boot scans SD; subsequent boots load from cache

## Hardware Pin Map

| Function | Pin(s) |
|----------|--------|
| SD SPI: SCK/MISO/MOSI/CS | 40/39/14/12 |
| Audio | M5 Speaker via I2S |
| Button A (G0) | Multi-click: 1=play/pause, 2=next, 3=prev |
| Display | 240×135 LCD (landscape) |

## Display Layout Constants

Playlist sidebar: 120px wide. Header: 20px. Bottom bar: 18px. Row height: 15px. Max visible rows: 6.

## Settings (20 values persisted in NVS)

Brightness, theme (4 themes), visualizer mode (4 modes), screen timeout, resume-play, sample rate (44.1k–128k), seek interval (5–60s), Wi-Fi on/off, Wi-Fi mode (STA/AP), power saver (OFF/160MHz/80MHz), current folder.

## Key Patterns

- **Triple-buffering** for glitch-free audio
- **Byte-offset caching** to handle large playlists without loading all paths into RAM
- **CPU frequency scaling** for power management (240/160/80 MHz)
- **Sprite-based visualization** to avoid full-screen redraws
- **4 color themes** using RGB565 color variables (`C_BG_DARK`, `C_ACCENT`, `C_PLAYING`, etc.)
