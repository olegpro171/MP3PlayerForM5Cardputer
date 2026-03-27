# Lessons Learned

## SD Card I/O in Draw Loops Starves Audio Playback

**Date:** 2026-03-27
**Context:** Adding album songs view (`drawAlbumSongs`)

### What happened

Each scroll action in the album songs view caused a brief but audible playback freeze. Other views (search, folder select) with similar UI patterns did not have this issue.

### Root cause

The draw function performed **3 separate SD file open/seek/read/close cycles per visible row**:

1. `getAlbumSongPath(ri)` — open, seek, read, close
2. `getAlbumSongTrackStr(ri)` — open, seek, read, close (reading the same line twice)
3. `audioApp.getSongPath(currentIndex)` — open, seek, read, close (same value every iteration)

With 6 visible rows, that's **18 SD operations per redraw**. SD card I/O on ESP32 at 25MHz SPI blocks the CPU, which starves `AudioEngine::loopTasks()` — the audio decoder can't refill its buffer in time, causing glitches.

### Why other views didn't have this problem

- `drawPlaylist()` opens the file **once** and reads rows sequentially (no per-row open/close).
- `drawSearch()` calls `getSongPath()` once per row but doesn't double-read the same data.
- `drawFolderSelect()` reads from a RAM vector (`folderList`), no SD I/O in the loop at all.

### Fix

1. **Open the index file once** before the loop, seek within it per row, close after the loop.
2. **Cache repeated values outside the loop** — the current playing path is the same for every row, so read it once before iterating.
3. **Parse all needed fields from a single line read** instead of calling two separate accessor functions that each open/read/close the file independently.

Result: 18 SD operations reduced to 2 file opens + 7 seeks.

### Rule for this codebase

**Never open/close an SD file inside a per-row drawing loop.** Open the file once before the loop, seek within it, and close after. Cache any value that doesn't change between rows (like the currently playing song path) outside the loop. If you need multiple fields from the same line, parse them from one read — don't call separate accessor functions that each do their own file I/O.

---

## SD I/O Between fillScreen and Row Drawing Causes Visible Flicker

**Date:** 2026-03-27
**Context:** Album list sidebar (`drawPlaylist`) and album songs view (`drawAlbumSongs`)

### What happened

Scrolling through the album list caused the sidebar to visibly flash — the background color appeared for a split second before the rows were drawn on top. Other views (folder select, search) that use the same fill-then-draw pattern did not flicker.

### Root cause

The draw sequence was:

1. `fillRect` / `fillScreen` — clears the area to background color (instantly visible)
2. SD file open + per-row seek/read — **slow I/O gap where the blank background is visible**
3. Draw row text on top of the background

The time between step 1 and step 3 is the "flicker window." In views that don't flicker:
- `drawFolderSelect()` reads from a **RAM vector** — no I/O between fill and draw, so the gap is microseconds (invisible).
- The original `drawPlaylist()` opened the file once and read sequentially — minimal latency per row.

In the album list, each row called `getAlbumName()` which did a full SD open/seek/read/close cycle, making the gap long enough to see.

### Fix

**Do all SD reads BEFORE any display writes.** Pre-read visible row data into a small stack array, then fill the background and draw rows from RAM with zero I/O gaps:

```cpp
// 1. Read all data into RAM first (SD I/O happens here)
String albumNames[MAX_VISIBLE_ROWS];
File f = SD.open(indexFile);
for (int i = 0; i < rowCount; i++) {
    f.seek(offsets[startIdx + i]);
    albumNames[i] = f.readStringUntil('\n');
}
f.close();

// 2. Now do all display writes — no SD I/O between fill and text
Display.fillRect(..., bgColor);
for (int i = 0; i < rowCount; i++) {
    Display.print(albumNames[i]);
}
```

### Rule for this codebase

**Never interleave SD reads with display writes in a draw function.** Read all needed data into RAM first (a small stack array is fine for 6 rows), then do all display operations as a batch. The `fillRect` → draw text sequence must be uninterrupted by any blocking I/O.

---

## Full-Screen Redraw on Scroll Causes Flicker — Only Redraw What Changed

**Date:** 2026-03-27
**Context:** Album songs view (`drawAlbumSongs`) scroll still flickering after SD I/O was fixed

### What happened

After fixing SD I/O timing, scrolling in the album songs view still caused a one-frame flash. The header (album name) and footer (key hints) visibly blanked and repainted, even though their content never changes during a scroll. No other view in the project had this problem.

### Root cause

`drawAlbumSongs()` called `fillScreen(C_BG_DARK)` on every scroll, blanking the **entire 240×135 display** — header, content, and footer — then repainted all three. The header and footer were being destroyed and redrawn for no reason.

Other views that scroll without flicker follow a different pattern:
- `drawPlaylist()` (sidebar) only fills its own 120×97px rectangle — never touches the header, now-playing area, or footer.
- `drawFolderSelect()` does `fillScreen` too, but its data is entirely in RAM (a `std::vector<String>`) so the blank-to-repaint gap is sub-millisecond and invisible.

The album songs view combined the worst of both: a full-screen clear **and** SD I/O (even pre-read, the function still cleared everything).

### Fix

Split the monolithic `drawAlbumSongs()` into three functions:
- `drawAlbumSongsHeader()` — draws the album name header
- `drawAlbumSongsList()` — clears and redraws **only the content area** between header and footer
- `drawAlbumSongsFooter()` — draws the key hints footer

`drawAlbumSongs()` calls all three (used once on state entry). Scroll handlers call only `drawAlbumSongsList()`, which clears just the content rectangle — the header and footer stay completely untouched.

### Rule for this codebase

**On scroll actions, only redraw the region that changed.** Split full-screen draw functions into header / content / footer parts. The initial state entry calls all three; scroll handlers call only the content part. Static elements (headers, footers) must never be touched by scroll redraws.
