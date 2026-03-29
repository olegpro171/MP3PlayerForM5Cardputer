# M5Cardplayer ADV

A feature-rich portable music player for the **M5Stack Cardputer** (ESP32-S3). Supports MP3, FLAC, AAC, and WAV playback from SD card, with album-based browsing, audio visualizers, Wi-Fi streaming, and a full settings system.

Based on [SanchitMinda's MP3PlayerForM5Cardputer](https://github.com/sanchitminda/MP3PlayerForM5Cardputer) with significant UX improvements.

## Note from author

I wanted a Cardputer audio player that could do two things: automatically group tracks by metadata tags, and look good doing it. 
I couldn't find one that did both — so I built one instead, forking the MP3PlayerForM5Cardputer by Sanchit Minda as the foundation.

This project started just as a UX pass, but grew into a fairly deep rebuild — album-based navigation, accurate time tracking and a handful of quality-of-life fixes that bothered me enough to actually fix. This readme documents what changed and why.

A huge thank you to [Sanchit Minda](https://github.com/sanchitminda) for the original project. The audio pipeline he figured out made all of this possible.

## Who is this project for

This player is built for people who listen to albums as albums — start to finish, in order, the way the artist intended. If you queue up a record and let it run, this firmware is made for you.
The core navigation revolves around artists and albums rather than flat song lists. Tracks play in album order by default, shuffle operates within album or artist scope, and the library is organized around metadata tags rather than folder structure. Everything is optimized for the "put on an album and listen" workflow.
You'll probably enjoy this if you:

- Have a music library organized by artist and album with proper ID3/FLAC tags
- Like to browse by artist/album/track rather than scroll through one giant list
- Prefer shuffle modes that respect album or artist context over pure random playback

This project is probably not for you if you:

- Primarily listen to hand-curated playlists mixing tracks from many different artists
- Have a large collection of untagged or loosely tagged files
- Want folder-based or playlist-file navigation (that system was intentionally removed in this fork)

If the playlist-first workflow is what you need, at the current time the original project this fork is based on may be a better fit.


## What's New in This Fork

This fork restructures the player around **album-based navigation** and improves the overall experience for listening to full albums on the Cardputer's small screen.

### Cyrillic support

A built-in Cyrillic keyboard layout is available in search, so you can type queries in Russian (or other Cyrillic-script languages) directly on the Cardputer's keyboard without any workarounds.
If your library includes tracks with Cyrillic metadata — artist names, album titles, and track names in Russian and other Cyrillic languages are fully supported.

### Album Browsing with Artist Groups

The original player showed a flat list of all songs. This fork replaces that with an **album browser grouped by artist**:

> Because of the automatic grouping by artists/albums, it is highly recommended to use this firmware with music files that have metadata tags written. 
> All files without tags will be located in single 'Unknown artist'->'No Album' group, which might be not optimal for navigation.

- The sidebar shows artists as collapsible headers with their albums indented below
- Press Enter on an artist to expand/collapse, Enter on an album to see its tracks
- Tracks are sorted by track number from ID3 tags, not by filename
- Album-scoped playback: next/prev stays within the album instead of jumping to random files
- When an album finishes (no loop), it pauses on the first track instead of stopping silently

All metadata (artist, album, track number, duration) is read from ID3v2/v1, FLAC Vorbis comments, and M4A/AAC atoms during a one-time library scan and cached to SD card.

### Accurate Time Display

The original player used a hardcoded bytes-per-second constant for time calculations, which showed incorrect durations for most files. This fork **pre-calculates track duration** during the library scan by reading:

- MP3: Xing/Info VBR header frame count, with CBR bitrate fallback
- FLAC: STREAMINFO total samples / sample rate
- M4A/AAC: mvhd atom duration / timescale
- WAV: fmt chunk byte rate

Elapsed time is computed as `duration * (bytesPlayed / totalAudioBytes)`, accounting for metadata header offsets, which eliminates all possible drift and float accumulation, and perfectly survives seek and pause.

### Smarter Search

Search now supports **multi-token fuzzy matching against ID3 tags**. The search index includes artist, title, album, and file path, so you can also search by folder name or any combination.

### Multi-Scope Shuffle

The original shuffle picked a random song each time, often repeating tracks back-to-back. This fork replaces it with a **no-repeat shuffle queue** — like shuffling a deck of cards. Every song plays once before reshuffling.

Four shuffle modes, cycled with the **F** key:

| Display | Mode | Scope |
|---------|------|-------|
|   | Off | Sequential within album |
| `SA` | Shuffle Album | All tracks in current album |
| `SR` | Shuffle Artist | All songs by the current artist, across all their albums |
| `SG` | Shuffle Global | All songs in the entire library |

Prev goes backward through the shuffled order. When the queue is exhausted, it reshuffles (if looping) or pauses.

### Better State Persistence

- **Volume** is now saved and restored on boot (it wasn't before)
- **Song position** is saved reliably on every song change, not just on pause
- **Album context** is saved and restored on boot — next/prev continues within the same album after reboot instead of jumping to the global playlist
- The original had a bug where the save guard `if(currentPos > 0)` silently skipped saving when a song started from position 0 (every normal play). Fixed.

### Additional Improvements

- **Track info popup (T key)**: Shows full title, artist, and album in a popup overlay for text that doesn't fit in the small now-playing area
- **Play/Pause key (P)**: Dedicated keyboard shortcut alongside the existing Button A
- **Expand/Collapse all (E/Q)**: Quickly expand or collapse all artist groups in the sidebar
- **Splash screen toggle**: New setting to skip the 3-second boot animation for instant startup
- **Factory reset (hold Esc on boot)**: Erases all NVS settings to recover from corrupted config
- **Settings validation**: All settings are bounds-checked on load to prevent crashes from corrupted NVS values
- **Now Playing visualizer**: New text-only visualizer mode showing artist, album, and time without the animated art, giving more space for text
- **Visualizer renders when paused**: Info visualizer mode (Now Playing tags with no extra graphics), stays visible during pause instead of going blank
- **No SD card prompt**: Friendly message instead of a black screen hang when SD card is missing, with retry on key press

### Removed

- **Folder/playlist system**: The original let you filter songs by folder. Since albums are now the primary navigation, folder filtering was removed entirely.

## Hardware Requirements

- **M5Stack Cardputer** (ESP32-S3 based)
- **MicroSD Card** (FAT32 formatted)
- Audio files: MP3, FLAC, M4A/AAC, WAV

## Installation

1. **Prepare the SD Card:**
   - Format to FAT32
   - Copy music files to the card (root or subfolders, up to 3 levels deep)
   - Insert into the Cardputer

2. **Setup Arduino IDE:**
   - Select board: **M5Stack Cardputer** (Tools > Board)
   - Install libraries via Library Manager:
     - M5Cardputer (by M5Stack)
     - M5Unified (by M5Stack)
     - ESP8266Audio v1.9.7+ (by Earle F. Philhower III)
   - Set Tools->Partition scheme to 'Huge APP (3MB No OTA/1MB SPIFFS)' 

3. **Flash:**
   - Open `MP3PlayerM5Cardputer.ino`
   - Connect via USB-C, click Upload

4. **First boot:**
   - The player will scan the SD card and build the album index (takes a moment)
   - Subsequent boots load from cache instantly

## Controls

### Main Player

| Key | Function |
|-----|----------|
| **; / .** | Scroll through artists and albums |
| **Enter** | Expand/collapse artist, or open album |
| **P** | Play / Pause |
| **N / B** | Next / Previous track |
| **/ / ,** | Seek forward / backward |
| **[ / ]** | Volume down / up |
| **S** | Search (multi-token, searches tags) |
| **F** | Shuffle mode: off / SA (album) / SR (artist) / SG (global) |
| **L** | Loop mode: 1x (once) / LP (loop) / 1T (one track) |
| **V** | Cycle visualizer mode |
| **T** | Track info popup (full title/artist/album) |
| **E / Q** | Expand all / Collapse all artists |
| **I** | Controls & Help |
| **Esc / `** | Settings |

### Album Songs View

| Key | Function |
|-----|----------|
| **; / .** | Scroll through tracks |
| **Enter** | Play selected track |
| **`** | Back to album list |

### Pocket Mode (Button A / G0)

- **1 click**: Play / Pause
- **2 clicks**: Next track
- **3 clicks**: Previous track

Press any key to wake the screen after timeout.

### Factory Reset

Hold **Esc** while the device boots to erase all settings and restore defaults.

## Settings

Accessible via Esc key. Navigate with ; / . and adjust with / / , keys.

- Brightness, Screen timeout, Resume on boot
- DAC sample rate, Seek interval
- Wi-Fi power, Wi-Fi mode (STA/AP), network setup
- Power saver (OFF / 160MHz / 80MHz)
- Theme (4 color themes), Visualizer mode (6 modes)
- Splash screen toggle
- Rescan library, Export/Import config to SD

## Visualizer Modes

1. **Classic Bars** - FFT frequency bar graph
2. **Waveform Line** - FFT waveform display
3. **Circular Spikes** - FFT circular visualization
4. **Art + Info** - Animated pixel art with artist, album, and time
5. **Now Playing** - Text-only artist, album, and time (full width)
6. **OFF** - No visualizer


## File Structure

```
/ (SD Card Root)
+-- Music/
|   +-- Artist - Song.mp3
|   +-- Album Folder/
|       +-- 01 Track.flac
+-- playlist.txt      (auto-generated song index)
+-- albums.idx        (auto-generated album index with artist groups)
+-- search.idx        (auto-generated search index with tags + durations)
```

All index files are rebuilt when you select "Rescan Library" in settings.

## Troubleshooting

- **No songs found**: Ensure SD card is FAT32. Supported formats: .mp3, .flac, .m4a, .aac, .wav
- **Wrong time display**: Rescan the library (Settings > Rescan Library) to rebuild duration index
- **Device won't boot**: Hold Esc during boot to factory reset settings
- **Audio stuttering on Ultra power saver (80MHz)**: Switch to Basic (160MHz) or OFF
- **Compilation errors**: Ensure ESP8266Audio v1.9.7+ is installed

## Credits

- Original project by [Sanchit Minda](https://github.com/sanchitminda)
- Audio processing by [ESP8266Audio Library](https://github.com/earlephilhower/ESP8266Audio)
- UI and hardware integration via M5Stack libraries
