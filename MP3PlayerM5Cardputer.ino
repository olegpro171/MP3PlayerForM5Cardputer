///////////////////////////////////////////////
/// Made by: Sanchit Minda() | Github/sanchitminda
/// Fully Functional OOP & State Machine Architecture
//////////////////////////////////////////////
#include <vector>
#include <M5Unified.h>
#include <M5Cardputer.h>
#include <SPI.h>
#include <Preferences.h>
#include <nvs_flash.h>
#include <WiFi.h>
#include <WebServer.h>
#include "USB.h"
#include "USBMSC.h"

// Audio Libraries
#include <AudioOutput.h>
#include <AudioFileSourceSD.h>
#include <AudioFileSourceID3.h>
#include <AudioGeneratorMP3.h>
#include <AudioGeneratorFLAC.h>
#include <AudioGeneratorAAC.h>
#include <AudioGeneratorWAV.h>
#include <AudioFileSourceBuffer.h>
// ==========================================
// CONSTANTS & CONFIG
// ==========================================
#define SD_SPI_SCK_PIN  40
#define SD_SPI_MISO_PIN 39
#define SD_SPI_MOSI_PIN 14
#define SD_SPI_CS_PIN   12
#define PLAYLIST_FILE   "/playlist.txt"   // "All Music" flat cache
#define CONFIG_FILE     "/config.txt"
#define ALBUM_INDEX_FILE "/albums.idx"
#define ALBUM_RAW_FILE   "/albums_raw.tmp"

struct TagInfo {
    String album;
    uint8_t trackNo;
};

#define PLAYLIST_WIDTH 120
#define ROW_HEIGHT 15
#define HEADER_HEIGHT 20
#define BOTTOM_BAR_HEIGHT 18
#define MAX_VISIBLE_ROWS 6  

// --- DYNAMIC COLORS & THEMES ---
uint16_t C_BG_DARK, C_BG_LIGHT, C_HEADER, C_ACCENT, C_PLAYING, C_HIGHLIGHT, C_TEXT_MAIN, C_TEXT_DIM;

const int NUM_THEMES = 4;
const char* themeLabels[] = { "Gunmetal Blue", "Cyberpunk", "Retro Amber", "Hacker Green" };
const int NUM_VIS_MODES = 5;
const char* visModeLabels[] = { "Classic Bars", "Waveform Line", "Circular Spikes", "Now Playing", "OFF" };

void applyTheme(int index) {
    switch(index) {
        case 0: // Gunmetal Blue (Original)
            C_BG_DARK = 0x1002; C_BG_LIGHT = 0x2124; C_HEADER = 0x18E3;
            C_ACCENT = 0x05BF; C_PLAYING = 0x07E0; C_HIGHLIGHT = 0xF81F;
            C_TEXT_MAIN = 0xFFFF; C_TEXT_DIM = 0x9492;
            break;
        case 1: // Cyberpunk
            C_BG_DARK = 0x0803; C_BG_LIGHT = 0x1866; C_HEADER = 0xA013;
            C_ACCENT = 0x07FF; C_PLAYING = 0xFFE0; C_HIGHLIGHT = 0xF800;
            C_TEXT_MAIN = 0xFFFF; C_TEXT_DIM = 0x7BEF;
            break;
        case 2: // Retro Amber
            C_BG_DARK = TFT_BLACK; C_BG_LIGHT = 0x2104; C_HEADER = 0x6A00; 
            C_ACCENT = 0xFDA0; C_PLAYING = TFT_ORANGE; C_HIGHLIGHT = TFT_RED;
            C_TEXT_MAIN = 0xFEA0; C_TEXT_DIM = 0xA340;
            break;
        case 3: // Hacker Green
            C_BG_DARK = 0x0000; C_BG_LIGHT = 0x0180; C_HEADER = 0x0320; 
            C_ACCENT = 0x07E0; C_PLAYING = 0x07FF; C_HIGHLIGHT = TFT_WHITE;
            C_TEXT_MAIN = 0x07E0; C_TEXT_DIM = 0x03E0;
            break;
    }
}

enum LoopState { NO_LOOP, LOOP_ALL, LOOP_ONE };
enum UIState { UI_PLAYER, UI_SETTINGS, UI_HELP, UI_WIFI_SCAN, UI_TEXT_INPUT, UI_SEARCH, UI_ALBUM_SONGS };

const uint32_t sampleRateValues[] = { 44100, 48000, 88200, 96000, 128000 };
const char* sampleRateLabels[] = { "44.1k", "48k", "88.2k", "96k", "128k" };
const long timeoutValues[] = { 0, 30000, 60000, 120000, 300000 };
const char* timeoutLabels[] = { "Always On", "30 Sec", "1 Min", "2 Min", "5 Min" };
const char* powerModeLabels[] = { "OFF", "BASIC (160)", "ULTRA (80)" };
const char* helpLines[] = {
  "--- MUSIC PLAYER ---",
  "Enter: Open Album / Play",
  "; / . : Scroll Albums/Songs",
  "[ / ] : Volume - / +",
  "N / B : Next / Prev Song",
  "/ / , : Seek +Xs / -Xs",
  "S: Search  F: Shuffle",
  "Esc / ` : Settings",
  "V: Visualizer",
  "I:print Close Help",
  "--- SMART FEATURES ---",
  "Web UI: Enable Wi-Fi in",
  "settings to access.",
  "Power Saver: Underclock",
  "CPU to save battery life.",
  "",
  "--- POCKET MODE ---",
  "Btn A (1 Click): Play/Pause",
  "Btn A (2 Clicks): Next",
  "Btn A (3 Clicks): Prev",
  "Press any key to wake screen",
  "--- ABOUT ---",
  "Made with <3 by SaM",
  "Sit back, relax, and",
  "enjoy the music!",
  "---",
  "GH: github.com/sanchitminda",
  "Share your suggestions!"
};
const int numHelpLines = 28;
const int numSettings = 16;

// ==========================================
// GLOBALS
// ==========================================
struct Settings {
    int brightness = 100;      
    int themeIndex = 0;
    int visMode = 0;
    int timeoutIndex = 0;      
    bool resumePlay = true;    
    int spkRateIndex = 4;      
    int lastIndex = 0;         
    uint32_t lastPos = 0;      
    bool wifiEnabled = false;  
    String wifiSSID = "";      
    String wifiPass = "";      
    bool isAPMode = false;
    String apSSID = "Cardputer";   
    String apPass = "12345678";    
    int powerSaverMode = 0;        
    int seek = 0;
    int volume = 128;
    bool showSplash = true;
};

Settings userSettings;
Preferences preferences;
WebServer server(80);
LGFX_Sprite visSprite(&M5Cardputer.Display);

UIState currentState = UI_PLAYER;
unsigned long lastInputTime = 0;
bool isScreenOff = false;

// Text Input Globals
int textInputTarget = 0; // 0=STA Pass, 1=AP SSID, 2=AP Pass
String enteredText = "";


// Search globals
String g_searchQuery = "";
std::vector<int> g_searchResults;  // indices into audioApp.songOffsets
int g_searchCursor = 0;
int g_searchScrollOffset = 0;

// Album browsing globals
std::vector<uint32_t> g_albumOffsets;       // byte offsets of '@' lines in albums.idx
int g_albumCursor = 0;
int g_albumScrollOffset = 0;
std::vector<uint32_t> g_albumSongOffsets;   // byte offsets of song lines in current album
int g_albumSongCursor = 0;
int g_albumSongScrollOffset = 0;
String g_currentAlbumName = "";
bool g_albumPlaybackActive = false;         // true when playing within an album
int g_albumPlaybackIndex = 0;               // index into g_albumSongOffsets for current track

// ==========================================
// HARDWARE CLASSES (FFT & SPEAKER)
// ==========================================
#define FFT_SIZE 256
class fft_t {
  float _wr[FFT_SIZE + 1], _wi[FFT_SIZE + 1], _fr[FFT_SIZE + 1], _fi[FFT_SIZE + 1];
  uint16_t _br[FFT_SIZE + 1]; size_t _ie;
public:
  fft_t(void) {
#ifndef M_PI
#define M_PI 3.141592653
#endif
    _ie = logf( (float)FFT_SIZE ) / log(2.0) + 0.5;
    static constexpr float omega = 2.0f * M_PI / FFT_SIZE;
    static constexpr int s4 = FFT_SIZE / 4; static constexpr int s2 = FFT_SIZE / 2;
    for ( int i = 1 ; i < s4 ; ++i) { float f = cosf(omega * i); _wi[s4 + i] = f; _wi[s4 - i] = f; _wr[i] = f; _wr[s2 - i] = -f; }
    _wi[s4] = _wr[0] = 1; size_t je = 1; _br[0] = 0; _br[1] = FFT_SIZE / 2;
    for ( size_t i = 0 ; i < _ie - 1 ; ++i ) { _br[ je << 1 ] = _br[ je ] >> 1; je = je << 1; for ( size_t j = 1 ; j < je ; ++j ) _br[je + j] = _br[je] + _br[j]; }
  }
  void exec(const int16_t* in) {
    memset(_fi, 0, sizeof(_fi));
    for ( size_t j = 0 ; j < FFT_SIZE / 2 ; ++j ) {
      float basej = 0.25 * (1.0-_wr[j]); size_t r = FFT_SIZE - j - 1;
      _fr[_br[j]] = basej * (in[j * 2] + in[j * 2 + 1]); _fr[_br[r]] = basej * (in[r * 2] + in[r * 2 + 1]);
    }
    size_t s = 1; size_t i = 0;
    do { size_t ke = s; s <<= 1; size_t je = FFT_SIZE / s; size_t j = 0;
      do { size_t k = 0;
        do { size_t l = s * j + k; size_t m = ke * (2 * j + 1) + k; size_t p = je * k;
          float Wxmr = _fr[m] * _wr[p] + _fi[m] * _wi[p], Wxmi = _fi[m] * _wr[p] - _fr[m] * _wi[p];
          _fr[m] = _fr[l] - Wxmr; _fi[m] = _fi[l] - Wxmi; _fr[l] += Wxmr; _fi[l] += Wxmi;
        } while ( ++k < ke);
      } while ( ++j < je );
    } while ( ++i < _ie );
  }
  uint32_t get(size_t index) { return (index < FFT_SIZE / 2) ? (uint32_t)sqrtf(_fr[ index ] * _fr[ index ] + _fi[ index ] * _fi[ index ]) : 0u; }
};
class AudioOutputM5Speaker : public AudioOutput {
  public:
    AudioOutputM5Speaker(m5::Speaker_Class* m5sound, uint8_t virtual_sound_channel = 0) { _m5sound = m5sound; _virtual_ch = virtual_sound_channel; }
    virtual ~AudioOutputM5Speaker(void) {};
    virtual bool begin(void) override { return true; }
    virtual bool ConsumeSample(int16_t sample[2]) override {
      if (_tri_buffer_index < tri_buf_size) {
        _tri_buffer[_tri_index][_tri_buffer_index] = sample[0]; _tri_buffer[_tri_index][_tri_buffer_index+1] = sample[1]; _tri_buffer_index += 2; return true;
      }
      flush(); return false;
    }
    virtual void flush(void) override {
      if (_tri_buffer_index) { _m5sound->playRaw(_tri_buffer[_tri_index], _tri_buffer_index, hertz, true, 1, _virtual_ch); _tri_index = _tri_index < 2 ? _tri_index + 1 : 0; _tri_buffer_index = 0; }
    }
    virtual bool stop(void) override { flush(); _m5sound->stop(_virtual_ch); return true; }
    const int16_t* getBuffer(void) const { return _tri_buffer[(_tri_index + 2) % 3]; }
  protected:
    m5::Speaker_Class* _m5sound; uint8_t _virtual_ch; static constexpr size_t tri_buf_size = 2048;
    int16_t _tri_buffer[3][tri_buf_size]; size_t _tri_buffer_index = 0; size_t _tri_index = 0;
};

AudioOutputM5Speaker *out = nullptr;
static constexpr size_t WAVE_SIZE = 320;
static fft_t fft;
static int16_t raw_data[WAVE_SIZE * 2];

// ==========================================
// CONFIG MANAGER
// ==========================================
class ConfigManager {
public:
    static void load() {
        preferences.begin("sam_music", true);
        userSettings.brightness = preferences.getInt("brightness", 100);
        userSettings.timeoutIndex = preferences.getInt("timeoutIndex", 0);
        userSettings.resumePlay = preferences.getBool("resumePlay", true);
        userSettings.spkRateIndex = preferences.getInt("spkRate", 4);
        userSettings.lastIndex = preferences.getInt("lastIndex", 0);
        userSettings.lastPos = preferences.getUInt("lastPos", 0);
        userSettings.wifiEnabled = preferences.getBool("wifiEnabled", false);
        userSettings.wifiSSID = preferences.getString("wifiSSID", "");
        userSettings.wifiPass = preferences.getString("wifiPass", "");
        userSettings.isAPMode = preferences.getBool("isAPMode", false);
        userSettings.apSSID = preferences.getString("apSSID", "Cardputer");
        userSettings.apPass = preferences.getString("apPass", "12345678");
        userSettings.powerSaverMode = preferences.getInt("powerMode", 0);
        userSettings.themeIndex = preferences.getInt("themeIndex", 0);
        userSettings.visMode = preferences.getInt("visMode", 0);
        userSettings.seek = preferences.getInt("seek", 5);
        userSettings.volume = preferences.getInt("volume", 128);
        userSettings.showSplash = preferences.getBool("showSplash", true);
        preferences.end();

        if(userSettings.apSSID.length() == 0) userSettings.apSSID = "Cardputer";
        if(userSettings.apPass.length() < 8) userSettings.apPass = "12345678";
    }

    static void save(uint32_t currentPos = 0, int currentIndex = -1) {
        if(currentIndex >= 0) { userSettings.lastPos = currentPos; userSettings.lastIndex = currentIndex; }
        preferences.begin("sam_music", false);
        preferences.putInt("brightness", userSettings.brightness);
        preferences.putInt("timeoutIndex", userSettings.timeoutIndex);
        preferences.putBool("resumePlay", userSettings.resumePlay);
        preferences.putInt("spkRate", userSettings.spkRateIndex);
        preferences.putInt("lastIndex", userSettings.lastIndex);
        preferences.putUInt("lastPos", userSettings.lastPos);
        preferences.putBool("wifiEnabled", userSettings.wifiEnabled);
        preferences.putString("wifiSSID", userSettings.wifiSSID);
        preferences.putString("wifiPass", userSettings.wifiPass);
        preferences.putBool("isAPMode", userSettings.isAPMode);
        preferences.putString("apSSID", userSettings.apSSID);
        preferences.putString("apPass", userSettings.apPass);
        preferences.putInt("powerMode", userSettings.powerSaverMode);
        preferences.putInt("themeIndex", userSettings.themeIndex);
        preferences.putInt("visMode", userSettings.visMode);
        preferences.putInt("seek", userSettings.seek);
        preferences.putInt("volume", userSettings.volume);
        preferences.putBool("showSplash", userSettings.showSplash);
        preferences.end();
    }

    static void exportToSD() {
        if (SD.exists(CONFIG_FILE)) SD.remove(CONFIG_FILE);
        File file = SD.open(CONFIG_FILE, FILE_WRITE);
        if (file) {
            file.println(userSettings.brightness); file.println(userSettings.timeoutIndex);
            file.println(userSettings.resumePlay ? 1 : 0); file.println(userSettings.spkRateIndex);
            file.println(userSettings.lastIndex); file.println(userSettings.lastPos);
            file.println(userSettings.wifiEnabled ? 1 : 0); file.println(userSettings.wifiSSID);
            file.println(userSettings.wifiPass); file.println(userSettings.isAPMode ? 1 : 0);
            file.println(userSettings.apSSID); file.println(userSettings.apPass);
            file.println(userSettings.powerSaverMode); file.println(userSettings.seek);
            file.println(userSettings.volume);
            file.println(userSettings.showSplash ? 1 : 0); file.close();
            
            M5Cardputer.Display.fillScreen(C_BG_DARK); M5Cardputer.Display.setCursor(10, 40);
            M5Cardputer.Display.setTextColor(C_PLAYING); M5Cardputer.Display.print("Exported to SD!"); delay(1000);
        }
    }

    static void importFromSD() {
        if (!SD.exists(CONFIG_FILE)) {
            M5Cardputer.Display.fillScreen(C_BG_DARK); M5Cardputer.Display.setCursor(10, 40);
            M5Cardputer.Display.setTextColor(TFT_RED); M5Cardputer.Display.print("No config.txt found!"); delay(1000); return;
        }
        File file = SD.open(CONFIG_FILE);
        if (file) {
            if(file.available()) userSettings.brightness = file.readStringUntil('\n').toInt();
            if(file.available()) userSettings.timeoutIndex = file.readStringUntil('\n').toInt();
            if(file.available()) userSettings.resumePlay = (file.readStringUntil('\n').toInt() == 1);
            if(file.available()) userSettings.spkRateIndex = file.readStringUntil('\n').toInt();
            if(file.available()) userSettings.lastIndex = file.readStringUntil('\n').toInt();
            if(file.available()) userSettings.lastPos = file.readStringUntil('\n').toInt();
            if(file.available()) userSettings.wifiEnabled = (file.readStringUntil('\n').toInt() == 1);
            if(file.available()) { userSettings.wifiSSID = file.readStringUntil('\n'); userSettings.wifiSSID.trim(); }
            if(file.available()) { userSettings.wifiPass = file.readStringUntil('\n'); userSettings.wifiPass.trim(); }
            if(file.available()) userSettings.isAPMode = (file.readStringUntil('\n').toInt() == 1);
            if(file.available()) { userSettings.apSSID = file.readStringUntil('\n'); userSettings.apSSID.trim(); }
            if(file.available()) { userSettings.apPass = file.readStringUntil('\n'); userSettings.apPass.trim(); }
            if(file.available()) userSettings.powerSaverMode = file.readStringUntil('\n').toInt();
            if(file.available()) userSettings.seek = file.readStringUntil('\n').toInt();
            if(file.available()) userSettings.volume = file.readStringUntil('\n').toInt();
            if(file.available()) userSettings.showSplash = (file.readStringUntil('\n').toInt() == 1);
            file.close();
            save(); M5Cardputer.Display.setBrightness(userSettings.brightness);
            M5Cardputer.Display.fillScreen(C_BG_DARK); M5Cardputer.Display.setCursor(10, 40);
            M5Cardputer.Display.setTextColor(C_PLAYING); M5Cardputer.Display.print("Imported from SD!"); delay(1000);
        }
    }
};

void applyCpuFrequency() {
    if (userSettings.wifiEnabled || userSettings.powerSaverMode == 0) setCpuFrequencyMhz(240); 
    else if (userSettings.powerSaverMode == 1) setCpuFrequencyMhz(160); 
    else setCpuFrequencyMhz(80); 
}

// ==========================================
// ALBUM INDEX HELPERS
// ==========================================
bool loadAlbumIndex() {
    g_albumOffsets.clear();
    g_albumCursor = 0;
    g_albumScrollOffset = 0;
    if (!SD.exists(ALBUM_INDEX_FILE)) return false;
    File f = SD.open(ALBUM_INDEX_FILE);
    if (!f) return false;
    String header = f.readStringUntil('\n'); header.trim();
    if (header != "ALBUM_INDEX_V1") { f.close(); return false; }
    while (f.available()) {
        uint32_t pos = f.position();
        String line = f.readStringUntil('\n'); line.trim();
        if (line.startsWith("@")) {
            g_albumOffsets.push_back(pos);
        }
    }
    f.close();
    return (g_albumOffsets.size() > 0);
}

String getAlbumName(int albumIdx) {
    if (albumIdx < 0 || (size_t)albumIdx >= g_albumOffsets.size()) return "";
    File f = SD.open(ALBUM_INDEX_FILE);
    if (!f) return "";
    f.seek(g_albumOffsets[albumIdx]);
    String line = f.readStringUntil('\n'); line.trim();
    f.close();
    if (line.startsWith("@")) line = line.substring(1);
    return line;
}

void loadAlbumSongs(int albumIdx) {
    g_albumSongOffsets.clear();
    g_albumSongCursor = 0;
    g_albumSongScrollOffset = 0;
    if (albumIdx < 0 || (size_t)albumIdx >= g_albumOffsets.size()) return;
    File f = SD.open(ALBUM_INDEX_FILE);
    if (!f) return;
    // Seek past the '@AlbumName' line
    f.seek(g_albumOffsets[albumIdx]);
    f.readStringUntil('\n'); // skip album header
    // Read song lines until next album or EOF
    while (f.available()) {
        uint32_t pos = f.position();
        String line = f.readStringUntil('\n'); line.trim();
        if (line.startsWith("@")) break; // next album
        if (line.length() > 3 && line.charAt(2) == '|') {
            g_albumSongOffsets.push_back(pos);
        }
    }
    f.close();
    g_currentAlbumName = getAlbumName(albumIdx);
}

String getAlbumSongPath(int songIdx) {
    if (songIdx < 0 || (size_t)songIdx >= g_albumSongOffsets.size()) return "";
    File f = SD.open(ALBUM_INDEX_FILE);
    if (!f) return "";
    f.seek(g_albumSongOffsets[songIdx]);
    String line = f.readStringUntil('\n'); line.trim();
    f.close();
    // Format: TT|/path/to/file.mp3
    int pipePos = line.indexOf('|');
    if (pipePos < 0) return "";
    String path = line.substring(pipePos + 1);
    path.trim();
    if (path.length() > 0 && !path.startsWith("/")) path = "/" + path;
    return path;
}

String getAlbumSongTrackStr(int songIdx) {
    if (songIdx < 0 || (size_t)songIdx >= g_albumSongOffsets.size()) return "";
    File f = SD.open(ALBUM_INDEX_FILE);
    if (!f) return "";
    f.seek(g_albumSongOffsets[songIdx]);
    String line = f.readStringUntil('\n'); line.trim();
    f.close();
    int pipePos = line.indexOf('|');
    if (pipePos < 0) return "";
    return line.substring(0, pipePos);
}

// ==========================================
// TAG READER (Album + Track Number)
// ==========================================

bool readTagInfo(const char* filepath, TagInfo& out) {
    out.album = "";
    out.trackNo = 0;
    File f = SD.open(filepath);
    if (!f) return false;
    size_t fsize = f.size();
    if (fsize < 10) { f.close(); return false; }

    String path = String(filepath); path.toLowerCase();

    // --- MP3: ID3v2 then ID3v1 fallback ---
    if (path.endsWith(".mp3")) {
        uint8_t hdr[10];
        f.read(hdr, 10);
        if (hdr[0] == 'I' && hdr[1] == 'D' && hdr[2] == '3') {
            uint32_t tagSize = ((uint32_t)(hdr[6] & 0x7F) << 21) | ((uint32_t)(hdr[7] & 0x7F) << 14)
                             | ((uint32_t)(hdr[8] & 0x7F) << 7) | (hdr[9] & 0x7F);
            uint32_t pos = 10;
            uint32_t limit = min(tagSize + 10, (uint32_t)fsize);
            while (pos + 10 < limit) {
                uint8_t fhdr[10];
                f.seek(pos); f.read(fhdr, 10);
                if (fhdr[0] == 0) break;
                char frameId[5] = { (char)fhdr[0], (char)fhdr[1], (char)fhdr[2], (char)fhdr[3], 0 };
                uint32_t frameSize = ((uint32_t)fhdr[4] << 24) | ((uint32_t)fhdr[5] << 16) | ((uint32_t)fhdr[6] << 8) | fhdr[7];
                if (frameSize == 0 || frameSize > 10000) break;
                if (strcmp(frameId, "TALB") == 0 || strcmp(frameId, "TRCK") == 0) {
                    uint8_t enc; f.read(&enc, 1);
                    char buf[128]; memset(buf, 0, sizeof(buf));
                    int toRead = min((uint32_t)(frameSize - 1), (uint32_t)127);
                    f.read((uint8_t*)buf, toRead);
                    // Handle UTF-16 BOM: convert to ASCII
                    String val;
                    if (enc == 1 || enc == 2) {
                        // UTF-16 with BOM
                        int start = 0;
                        if (toRead >= 2 && ((uint8_t)buf[0] == 0xFF || (uint8_t)buf[0] == 0xFE)) start = 2;
                        for (int i = start; i < toRead - 1; i += 2) {
                            char c = (enc == 2 || (uint8_t)buf[0] == 0xFE) ? buf[i+1] : buf[i];
                            if (c >= 32 && c < 127) val += c;
                        }
                    } else {
                        val = String(buf);
                    }
                    val.trim();
                    if (strcmp(frameId, "TALB") == 0) out.album = val;
                    else {
                        int slashPos = val.indexOf('/');
                        if (slashPos > 0) val = val.substring(0, slashPos);
                        out.trackNo = (uint8_t)val.toInt();
                    }
                }
                pos += 10 + frameSize;
                if (out.album.length() > 0 && out.trackNo > 0) break;
            }
        }
        // ID3v1 fallback if album still empty
        if (out.album.length() == 0 && fsize > 128) {
            f.seek(fsize - 128);
            uint8_t tag[128]; f.read(tag, 128);
            if (tag[0] == 'T' && tag[1] == 'A' && tag[2] == 'G') {
                char albBuf[31]; memset(albBuf, 0, 31); memcpy(albBuf, tag + 63, 30);
                String alb = String(albBuf); alb.trim();
                if (alb.length() > 0) out.album = alb;
                // ID3v1.1 track number
                if (tag[125] == 0 && tag[126] != 0 && out.trackNo == 0)
                    out.trackNo = tag[126];
            }
        }
        f.close(); return true;
    }

    // --- FLAC: Vorbis Comment ---
    if (path.endsWith(".flac")) {
        uint8_t magic[4]; f.read(magic, 4);
        if (magic[0] != 'f' || magic[1] != 'L' || magic[2] != 'a' || magic[3] != 'C') { f.close(); return false; }
        bool lastBlock = false;
        while (!lastBlock && f.position() < fsize) {
            uint8_t bhdr[4]; f.read(bhdr, 4);
            lastBlock = (bhdr[0] & 0x80) != 0;
            uint8_t blockType = bhdr[0] & 0x7F;
            uint32_t blockLen = ((uint32_t)bhdr[1] << 16) | ((uint32_t)bhdr[2] << 8) | bhdr[3];
            if (blockType == 4) { // VORBIS_COMMENT
                uint32_t blockStart = f.position();
                uint32_t blockEnd = blockStart + blockLen;
                // Skip vendor string
                uint8_t lb[4]; f.read(lb, 4);
                uint32_t vendorLen = lb[0] | (lb[1]<<8) | (lb[2]<<16) | (lb[3]<<24);
                f.seek(f.position() + vendorLen);
                f.read(lb, 4);
                uint32_t numComments = lb[0] | (lb[1]<<8) | (lb[2]<<16) | (lb[3]<<24);
                for (uint32_t ci = 0; ci < numComments && f.position() < blockEnd; ci++) {
                    f.read(lb, 4);
                    uint32_t cLen = lb[0] | (lb[1]<<8) | (lb[2]<<16) | (lb[3]<<24);
                    if (cLen > 512) { f.seek(f.position() + cLen); continue; }
                    char cbuf[513]; memset(cbuf, 0, 513);
                    f.read((uint8_t*)cbuf, cLen);
                    String comment = String(cbuf);
                    String commentUpper = comment; commentUpper.toUpperCase();
                    if (commentUpper.startsWith("ALBUM=")) {
                        out.album = comment.substring(6); out.album.trim();
                    } else if (commentUpper.startsWith("TRACKNUMBER=")) {
                        out.trackNo = (uint8_t)comment.substring(12).toInt();
                    }
                    if (out.album.length() > 0 && out.trackNo > 0) break;
                }
                break; // done with vorbis comment block
            } else {
                f.seek(f.position() + blockLen);
            }
        }
        f.close(); return true;
    }

    // --- M4A/AAC: MP4 atoms ---
    if (path.endsWith(".m4a") || path.endsWith(".aac")) {
        // Simplified MP4 parser: scan for ilst atom containing album and track
        // Read first 256KB max to find metadata
        uint32_t searchLimit = min((uint32_t)fsize, (uint32_t)262144);
        uint8_t buf[8];
        uint32_t pos = 0;
        while (pos + 8 < searchLimit) {
            f.seek(pos); f.read(buf, 8);
            uint32_t atomSize = ((uint32_t)buf[0]<<24) | ((uint32_t)buf[1]<<16) | ((uint32_t)buf[2]<<8) | buf[3];
            char atomType[5] = { (char)buf[4], (char)buf[5], (char)buf[6], (char)buf[7], 0 };
            if (atomSize < 8) break;
            // Container atoms we descend into
            if (strcmp(atomType, "moov") == 0 || strcmp(atomType, "udta") == 0 ||
                strcmp(atomType, "meta") == 0 || strcmp(atomType, "ilst") == 0) {
                uint32_t skip = 8;
                if (strcmp(atomType, "meta") == 0) skip = 12; // meta has 4-byte version/flags
                pos += skip;
                continue;
            }
            // Check for album atom: \xA9alb
            if (buf[4] == 0xA9 && buf[5] == 'a' && buf[6] == 'l' && buf[7] == 'b') {
                // Inside: data atom at pos+8
                if (pos + atomSize <= searchLimit && atomSize > 24) {
                    f.seek(pos + 24); // skip atom header + data atom header (8+8+8)
                    int dataLen = min((uint32_t)(atomSize - 24), (uint32_t)127);
                    char dbuf[128]; memset(dbuf, 0, 128);
                    f.read((uint8_t*)dbuf, dataLen);
                    out.album = String(dbuf); out.album.trim();
                }
                pos += atomSize; continue;
            }
            // Check for track number atom: trkn
            if (strcmp(atomType, "trkn") == 0 && atomSize > 24) {
                f.seek(pos + 24);
                uint8_t trkData[4]; f.read(trkData, 4);
                // trkn data is big-endian: [0][0][track_hi][track_lo]...
                out.trackNo = (uint8_t)((trkData[2] << 8) | trkData[3]);
                pos += atomSize; continue;
            }
            pos += atomSize;
            if (out.album.length() > 0 && out.trackNo > 0) break;
        }
        f.close(); return true;
    }

    // WAV: no standard album tag, skip
    f.close();
    return true;
}

// Forward declarations for album helpers used in AudioEngine
String getAlbumSongPath(int songIdx);
int findSongInPlaylist(const String& targetPath);

// ==========================================
// AUDIO ENGINE
// ==========================================
class AudioEngine {
public:
    AudioFileSourceSD *file = nullptr;
    AudioFileSourceID3 *id3 = nullptr;
    AudioFileSourceBuffer *buff = nullptr; 
    AudioGenerator *decoder = nullptr;

    std::vector<uint32_t> songOffsets;
    int currentIndex = 0;
    int browserIndex = 0;
    bool isPaused = false;
    bool isShuffle = false;
    LoopState loopMode = NO_LOOP;
    uint32_t paused_at = 0;
    uint32_t pausedSize = 0;
    String currentTitle = "";
    String currentArtist = "";
    String currentAlbum = "";

    void listDir(fs::FS &fs, const char *dirname, uint8_t levels, File &playlistFile, File *rawAlbumFile = nullptr) {
        File root = fs.open(dirname); if (!root || !root.isDirectory()) return;
        File f = root.openNextFile();
        while (f) {
            String fname = f.name();
            if (fname.startsWith(".")){
                f = root.openNextFile();
                continue;
            }
            if (f.isDirectory()) { if (levels) listDir(fs, f.path(), levels - 1, playlistFile, rawAlbumFile); }
            else {
                String filename = f.name(); String filepath = f.path();
                String filenameLower = filename; filenameLower.toLowerCase();
                if (filenameLower.endsWith(".mp3") || filenameLower.endsWith(".flac") || filenameLower.endsWith(".m4a") || filenameLower.endsWith(".aac") || filenameLower.endsWith(".wav")) {
                    playlistFile.println(filepath);
                    if (rawAlbumFile) {
                        TagInfo tag;
                        readTagInfo(filepath.c_str(), tag);
                        String albumName = tag.album.length() > 0 ? tag.album : "[No Album]";
                        albumName.replace("\t", " ");
                        rawAlbumFile->print(albumName); rawAlbumFile->print("\t");
                        rawAlbumFile->print(tag.trackNo); rawAlbumFile->print("\t");
                        rawAlbumFile->println(filepath);
                    }
                    M5Cardputer.Display.fillScreen(C_BG_DARK); M5Cardputer.Display.setCursor(10, 40);
                    M5Cardputer.Display.println("Scanning..."); M5Cardputer.Display.setTextColor(C_ACCENT);
                    M5Cardputer.Display.println(filename); M5Cardputer.Display.setTextColor(C_TEXT_MAIN);
                }
            }
            f = root.openNextFile();
        }
    }

    void buildAlbumIndex() {
        M5Cardputer.Display.fillScreen(C_BG_DARK); M5Cardputer.Display.setCursor(10, 40);
        M5Cardputer.Display.setTextColor(C_ACCENT); M5Cardputer.Display.println("Building album index...");
        M5Cardputer.Display.setTextColor(C_TEXT_MAIN);

        if (!SD.exists(ALBUM_RAW_FILE)) return;

        // Pass 1: collect unique album names
        std::vector<String> albums;
        File raw = SD.open(ALBUM_RAW_FILE);
        if (!raw) return;
        while (raw.available()) {
            String line = raw.readStringUntil('\n'); line.trim();
            int tab1 = line.indexOf('\t');
            if (tab1 < 0) continue;
            String albumName = line.substring(0, tab1);
            bool found = false;
            for (size_t i = 0; i < albums.size(); i++) {
                if (albums[i] == albumName) { found = true; break; }
            }
            if (!found) albums.push_back(albumName);
        }
        raw.close();

        // Sort albums alphabetically, [No Album] last
        for (size_t i = 0; i < albums.size(); i++) {
            for (size_t j = i + 1; j < albums.size(); j++) {
                bool swapNeeded = false;
                if (albums[i] == "[No Album]") swapNeeded = true;
                else if (albums[j] != "[No Album]") {
                    String a = albums[i]; a.toLowerCase();
                    String b = albums[j]; b.toLowerCase();
                    if (a > b) swapNeeded = true;
                }
                if (swapNeeded) { String tmp = albums[i]; albums[i] = albums[j]; albums[j] = tmp; }
            }
        }

        // Pass 2: for each album, collect its songs, sort by track, write to index
        if (SD.exists(ALBUM_INDEX_FILE)) SD.remove(ALBUM_INDEX_FILE);
        File idx = SD.open(ALBUM_INDEX_FILE, FILE_WRITE);
        if (!idx) return;
        idx.println("ALBUM_INDEX_V1");

        for (size_t ai = 0; ai < albums.size(); ai++) {
            // Collect songs for this album
            struct SongEntry { uint8_t trackNo; String path; };
            std::vector<SongEntry> songs;
            raw = SD.open(ALBUM_RAW_FILE);
            while (raw.available()) {
                String line = raw.readStringUntil('\n'); line.trim();
                int tab1 = line.indexOf('\t');
                int tab2 = line.indexOf('\t', tab1 + 1);
                if (tab1 < 0 || tab2 < 0) continue;
                String albName = line.substring(0, tab1);
                if (albName != albums[ai]) continue;
                uint8_t trk = (uint8_t)line.substring(tab1 + 1, tab2).toInt();
                String fpath = line.substring(tab2 + 1);
                songs.push_back({trk, fpath});
            }
            raw.close();

            // Sort by track number
            for (size_t i = 0; i < songs.size(); i++) {
                for (size_t j = i + 1; j < songs.size(); j++) {
                    if (songs[j].trackNo < songs[i].trackNo) {
                        SongEntry tmp = songs[i]; songs[i] = songs[j]; songs[j] = tmp;
                    }
                }
            }

            // Write album header and songs
            idx.print("@"); idx.println(albums[ai]);
            for (size_t si = 0; si < songs.size(); si++) {
                char trackStr[4]; sprintf(trackStr, "%02d", songs[si].trackNo);
                idx.print(trackStr); idx.print("|"); idx.println(songs[si].path);
            }
        }
        idx.close();
        if (SD.exists(ALBUM_RAW_FILE)) SD.remove(ALBUM_RAW_FILE);
    }

    // Full scan of the entire SD card → saves to PLAYLIST_FILE ("All Music")
    void performFullScan() {
        stop(); songOffsets.clear();
        M5Cardputer.Display.fillScreen(C_BG_DARK); M5Cardputer.Display.setCursor(10, 40); M5Cardputer.Display.println("Scanning SD Card...");
        if (SD.exists(PLAYLIST_FILE)) SD.remove(PLAYLIST_FILE);
        if (SD.exists(ALBUM_RAW_FILE)) SD.remove(ALBUM_RAW_FILE);
        File playlistFile = SD.open(PLAYLIST_FILE, FILE_WRITE);
        File rawAlbumFile = SD.open(ALBUM_RAW_FILE, FILE_WRITE);
        if (playlistFile) {
            listDir(SD, "/", 3, playlistFile, rawAlbumFile ? &rawAlbumFile : nullptr);
            playlistFile.close();
        }
        if (rawAlbumFile) rawAlbumFile.close();
        buildAlbumIndex();
        loadPlaylist();
    }

    bool loadPlaylist() {
        songOffsets.clear();
        if (!SD.exists(PLAYLIST_FILE)) return false;
        File f = SD.open(PLAYLIST_FILE);
        if (!f) return false;
        while (f.available()) {
            uint32_t pos = f.position(); 
            String line = f.readStringUntil('\n'); line.trim(); line.toLowerCase();
            if (line.endsWith(".mp3") || line.endsWith(".flac") || line.endsWith(".m4a") || line.endsWith(".aac") || line.endsWith(".wav")) { songOffsets.push_back(pos); }
        }
        f.close(); return (songOffsets.size() > 0);
    }

    String getSongPath(int index) {
        if (index < 0 || (size_t)index >= songOffsets.size()) return "";
        File f = SD.open(PLAYLIST_FILE); f.seek(songOffsets[index]); String path = f.readStringUntil('\n'); f.close();
        path.trim(); if (path.length() > 0 && !path.startsWith("/")) path = "/" + path;
        return path;
    }

    void stop() {
        if (decoder) { decoder->stop(); delete decoder; decoder = nullptr; }
        if (id3) { id3->close(); delete id3; id3 = nullptr; }
        if (buff) { buff->close(); delete buff; buff = nullptr; }
        if (file) { file->close(); delete file; file = nullptr; }
    }

    static void MDCallback(void *cbData, const char *type, bool isUnicode, const char *string);

    bool play(int index, uint32_t startPos = 0) {
        stop(); if (songOffsets.empty()) return false;
        currentIndex = index; browserIndex = index; currentTitle = ""; currentArtist = ""; currentAlbum = "";
        String fname = getSongPath(currentIndex);

        file = new AudioFileSourceSD(fname.c_str());
        buff = new AudioFileSourceBuffer(file, 16384); 
        id3 = new AudioFileSourceID3(buff); 
        id3->RegisterMetadataCB(MDCallback, (void*)"ID3TAG");
        
        if (startPos > 0) id3->seek(startPos, 1);
        String fnameLower = fname; fnameLower.toLowerCase();
        if (fnameLower.endsWith(".flac")) decoder = new AudioGeneratorFLAC();
        else if (fnameLower.endsWith(".m4a") || fnameLower.endsWith(".aac")) decoder = new AudioGeneratorAAC();
        else if (fnameLower.endsWith(".wav")) decoder = new AudioGeneratorWAV();
        else decoder = new AudioGeneratorMP3();
        
        isPaused = false;
        bool ok = decoder->begin(id3, out);
        if (ok) ConfigManager::save(startPos, currentIndex);
        return ok;
    }

    void togglePause() {
        if (!decoder) return;
        if (decoder->isRunning()) { paused_at = id3->getPos(); pausedSize = id3->getSize(); decoder->stop(); isPaused = true; }
        else if (isPaused) {
            String savedTitle = currentTitle, savedArtist = currentArtist, savedAlbum = currentAlbum;
            play(currentIndex, paused_at);
            currentTitle = savedTitle; currentArtist = savedArtist; currentAlbum = savedAlbum;
        }
    }

    void seek(int seconds) {
        if (!decoder || !decoder->isRunning() || !id3) return;
        int32_t newPos = id3->getPos() + (seconds * 16000);
        if (newPos < 0) newPos = 0; if (newPos > id3->getSize()) newPos = id3->getSize() - 1000;
        // Preserve metadata across seek — play() clears it, and seeking
        // past the ID3 header means the callback won't repopulate it.
        String savedTitle = currentTitle, savedArtist = currentArtist, savedAlbum = currentAlbum;
        play(currentIndex, newPos);
        currentTitle = savedTitle; currentArtist = savedArtist; currentAlbum = savedAlbum;
    }

    void next(bool autoPlay = false) {
        if (songOffsets.empty()) return;
        if (autoPlay && loopMode == LOOP_ONE) { play(currentIndex); return; }

        // Album-scoped playback
        if (g_albumPlaybackActive && g_albumSongOffsets.size() > 0) {
            if (isShuffle) {
                g_albumPlaybackIndex = random(0, g_albumSongOffsets.size());
            } else {
                g_albumPlaybackIndex++;
                if (g_albumPlaybackIndex >= (int)g_albumSongOffsets.size()) {
                    if (loopMode == LOOP_ALL) g_albumPlaybackIndex = 0;
                    else { stop(); g_albumPlaybackIndex = 0; g_albumPlaybackActive = false; return; }
                }
            }
            String songPath = getAlbumSongPath(g_albumPlaybackIndex);
            int globalIdx = findSongInPlaylist(songPath);
            if (globalIdx >= 0) play(globalIdx); else { g_albumPlaybackActive = false; }
            return;
        }

        if (isShuffle) currentIndex = random(0, songOffsets.size());
        else {
            currentIndex++;
            if ((size_t)currentIndex >= songOffsets.size()) { if (loopMode == LOOP_ALL) currentIndex = 0; else { stop(); currentIndex = 0; return; } }
        }
        play(currentIndex);
    }

    void prev() {
        if (songOffsets.empty()) return;

        // Album-scoped playback
        if (g_albumPlaybackActive && g_albumSongOffsets.size() > 0) {
            if (isShuffle) {
                g_albumPlaybackIndex = random(0, g_albumSongOffsets.size());
            } else {
                g_albumPlaybackIndex--;
                if (g_albumPlaybackIndex < 0) g_albumPlaybackIndex = (int)g_albumSongOffsets.size() - 1;
            }
            String songPath = getAlbumSongPath(g_albumPlaybackIndex);
            int globalIdx = findSongInPlaylist(songPath);
            if (globalIdx >= 0) play(globalIdx); else { g_albumPlaybackActive = false; }
            return;
        }

        if (isShuffle) currentIndex = random(0, songOffsets.size());
        else { currentIndex--; if (currentIndex < 0) currentIndex = songOffsets.size() - 1; }
        play(currentIndex);
    }

    void loopTasks() {
        if (decoder && decoder->isRunning()) {
            if (!decoder->loop()) { decoder->stop(); next(true); if(userSettings.resumePlay) ConfigManager::save(id3 ? id3->getPos() : 0, currentIndex); }
        }
    }

};

AudioEngine audioApp;

// Find a file path in the global songOffsets playlist. Returns index or -1.
int findSongInPlaylist(const String& targetPath) {
    File f = SD.open(PLAYLIST_FILE);
    if (!f) return -1;
    for (size_t i = 0; i < audioApp.songOffsets.size(); i++) {
        f.seek(audioApp.songOffsets[i]);
        String path = f.readStringUntil('\n'); path.trim();
        if (path.length() > 0 && !path.startsWith("/")) path = "/" + path;
        if (path == targetPath) { f.close(); return (int)i; }
    }
    f.close();
    return -1;
}

// ==========================================
// UI MANAGER
// ==========================================
class UIManager {
public:
    static int settingsCursor;
    static int menuScrollOffset;
    static bool showVisualizer;
    static int wifiCursor;
    static int wifiScrollOffset;
    static int wifiNetworkCount;
    static int helpScrollOffset;


    // -----------------------------------------------
    // SEARCH UI
    // -----------------------------------------------
    static void rebuildSearchResults() {
        g_searchResults.clear();
        g_searchCursor = 0;
        g_searchScrollOffset = 0;
        if (g_searchQuery.length() == 0) return;
        String queryLower = g_searchQuery; queryLower.toLowerCase();
        File f = SD.open(PLAYLIST_FILE);
        if (!f) return;
        int idx = 0;
        while (f.available()) {
            String line = f.readStringUntil('\n'); line.trim();
            String lineLower = line; lineLower.toLowerCase();
            // Extract filename portion for matching
            int slash = lineLower.lastIndexOf('/');
            String fname = (slash >= 0) ? lineLower.substring(slash + 1) : lineLower;
            if (fname.indexOf(queryLower) >= 0) {
                g_searchResults.push_back(idx);
            }
            idx++;
        }
        f.close();
    }

    static void drawSearch() {
        M5Cardputer.Display.fillScreen(C_BG_DARK);
        // Header
        M5Cardputer.Display.fillRect(0, 0, M5Cardputer.Display.width(), HEADER_HEIGHT, C_HEADER);
        M5Cardputer.Display.setFont(&fonts::Font0);
        M5Cardputer.Display.setTextColor(C_TEXT_MAIN, C_HEADER);
        M5Cardputer.Display.setCursor(5, 5);
        M5Cardputer.Display.print("Search Songs");

        // Search input box
        int boxY = HEADER_HEIGHT + 4;
        M5Cardputer.Display.fillRect(2, boxY, M5Cardputer.Display.width() - 4, 16, C_BG_LIGHT);
        M5Cardputer.Display.drawRect(2, boxY, M5Cardputer.Display.width() - 4, 16, C_ACCENT);
        M5Cardputer.Display.setTextColor(C_TEXT_MAIN);
        M5Cardputer.Display.setCursor(6, boxY + 4);
        String displayQuery = g_searchQuery.length() > 0 ? g_searchQuery : "";
        M5Cardputer.Display.print(displayQuery + "_");

        // Results
        int yPos = boxY + 20;
        int totalResults = g_searchResults.size();

        if (g_searchQuery.length() == 0) {
            M5Cardputer.Display.setTextColor(C_TEXT_DIM);
            M5Cardputer.Display.setCursor(6, yPos);
            M5Cardputer.Display.print("Type to search...");
        } else if (totalResults == 0) {
            M5Cardputer.Display.setTextColor(TFT_RED);
            M5Cardputer.Display.setCursor(6, yPos);
            M5Cardputer.Display.print("No results.");
        } else {
            for (int i = 0; i < MAX_VISIBLE_ROWS; i++) {
                int ri = g_searchScrollOffset + i;
                if (ri >= totalResults) break;
                int songIdx = g_searchResults[ri];
                bool isSelected = (ri == g_searchCursor);
                bool isPlaying  = (songIdx == audioApp.currentIndex);

                if (isSelected) {
                    M5Cardputer.Display.fillRect(2, yPos - 1, M5Cardputer.Display.width() - 4, ROW_HEIGHT, C_ACCENT);
                    M5Cardputer.Display.setTextColor(C_BG_DARK);
                } else if (isPlaying) {
                    M5Cardputer.Display.setTextColor(C_PLAYING);
                } else {
                    M5Cardputer.Display.setTextColor(C_TEXT_MAIN);
                }

                // Get filename from playlist
                String songPath = audioApp.getSongPath(songIdx);
                int slash = songPath.lastIndexOf('/');
                String fname = (slash >= 0) ? songPath.substring(slash + 1) : songPath;
                if (fname.length() > 28) fname = fname.substring(0, 27) + "~";

                M5Cardputer.Display.setCursor(6, yPos + 2);
                if (isPlaying && !isSelected) M5Cardputer.Display.print("> ");
                M5Cardputer.Display.print(fname);
                yPos += ROW_HEIGHT;
            }

            // Scroll arrows
            M5Cardputer.Display.setTextColor(C_ACCENT);
            if (g_searchScrollOffset > 0)
                M5Cardputer.Display.drawString("^", M5Cardputer.Display.width() - 10, boxY + 22);
            if (g_searchScrollOffset + MAX_VISIBLE_ROWS < totalResults)
                M5Cardputer.Display.drawString("v", M5Cardputer.Display.width() - 10, boxY + 20 + (MAX_VISIBLE_ROWS * ROW_HEIGHT) - 6);
        }

        // Footer
        int footerY = M5Cardputer.Display.height() - BOTTOM_BAR_HEIGHT;
        M5Cardputer.Display.fillRect(0, footerY, M5Cardputer.Display.width(), BOTTOM_BAR_HEIGHT, C_HEADER);
        M5Cardputer.Display.setTextColor(C_TEXT_DIM, C_HEADER);
        M5Cardputer.Display.setCursor(5, footerY + 4);
        M5Cardputer.Display.print("Enter:Play ;/.:Scroll  `:Close");
    }

    static void drawHelp() {
        drawPopup("CONTROLS & HELP", "Press 'I' to Exit");
        int px = 15, py = 15;
        int contentY = py + 25;
        int lineHeight = 12;
        int visibleLines = 7;

        M5Cardputer.Display.setFont(&fonts::Font0);
        M5Cardputer.Display.setTextColor(C_TEXT_MAIN);

        for (int i = 0; i < visibleLines; i++) {
            int idx = helpScrollOffset + i;
            if (idx >= numHelpLines) break;
            M5Cardputer.Display.setCursor(px + 10, contentY + (i * lineHeight));
            M5Cardputer.Display.print(helpLines[idx]);
        }
        
        M5Cardputer.Display.setTextColor(C_ACCENT);
        if (helpScrollOffset > 0) M5Cardputer.Display.drawString("^", px + 190, contentY);
        if (helpScrollOffset < numHelpLines - visibleLines) M5Cardputer.Display.drawString("v", px + 190, contentY + (visibleLines * lineHeight) - 10);
    }

    static void drawPopup(const char* title, const char* footer) {
        int px = 15, py = 15, pw = 210, ph = 120;
        M5Cardputer.Display.fillRoundRect(px, py, pw, ph, 4, C_BG_LIGHT); M5Cardputer.Display.drawRoundRect(px, py, pw, ph, 4, C_ACCENT);
        M5Cardputer.Display.fillRoundRect(px+2, py+2, pw-4, 18, 2, C_HEADER); M5Cardputer.Display.setFont(&fonts::Font0);
        M5Cardputer.Display.setTextColor(C_TEXT_MAIN); M5Cardputer.Display.setCursor(px + 8, py + 5); M5Cardputer.Display.print(title);
        if (footer) { M5Cardputer.Display.setCursor(px + 10, py + ph - 15); M5Cardputer.Display.setTextColor(C_TEXT_DIM); M5Cardputer.Display.print(footer); }
    }

    static void drawHeader() {
        M5Cardputer.Display.fillRect(0, 0, M5Cardputer.Display.width(), HEADER_HEIGHT, C_HEADER);
        M5Cardputer.Display.setFont(&fonts::Font0); 
        String prefix = "Music Player";
        if (userSettings.wifiEnabled) {
            if (userSettings.isAPMode) { prefix = WiFi.softAPIP().toString(); M5Cardputer.Display.setTextColor(TFT_ORANGE, C_HEADER); } 
            else if (WiFi.status() == WL_CONNECTED) { prefix = WiFi.localIP().toString(); M5Cardputer.Display.setTextColor(C_ACCENT, C_HEADER); } 
            else M5Cardputer.Display.setTextColor(C_TEXT_MAIN, C_HEADER);
        } else M5Cardputer.Display.setTextColor(C_TEXT_MAIN, C_HEADER);
        
        String headerText = prefix + " [" + String(g_albumOffsets.size()) + " albums]";
        M5Cardputer.Display.drawString(headerText.c_str(), 5, 5); 
        drawBattery();
    }

    static void drawBattery() {
        int batLevel = M5Cardputer.Power.getBatteryLevel();
        int w = 24, h = 10, x = M5Cardputer.Display.width() - w - 5, y = 5;
        M5Cardputer.Display.fillRect(x - 30, 0, w + 35, HEADER_HEIGHT, C_HEADER); 
        M5Cardputer.Display.drawRect(x, y, w, h, C_TEXT_MAIN); 
        M5Cardputer.Display.fillRect(x + w, y + 2, 2, 6, C_TEXT_MAIN);
        uint16_t color = batLevel < 20 ? TFT_RED : (batLevel < 50 ? TFT_YELLOW : C_PLAYING);
        int fillW = max(0, (int)map(batLevel, 0, 100, 0, w - 2));
        M5Cardputer.Display.fillRect(x + 1, y + 1, fillW, h - 2, color);
        M5Cardputer.Display.setFont(&fonts::Font0); 
        M5Cardputer.Display.setTextColor(C_TEXT_MAIN, C_HEADER);
        M5Cardputer.Display.setCursor(batLevel == 100 ? x - 29 : x - 25, y + 1); 
        M5Cardputer.Display.print(batLevel); 
        M5Cardputer.Display.print("%");
    }

    static void drawBottomBar() {
        int yPos = M5Cardputer.Display.height() - BOTTOM_BAR_HEIGHT;
        M5Cardputer.Display.fillRect(0, yPos, M5Cardputer.Display.width(), BOTTOM_BAR_HEIGHT, C_HEADER);
        M5Cardputer.Display.setTextColor(C_TEXT_MAIN); M5Cardputer.Display.setFont(&fonts::Font0);
        M5Cardputer.Display.setCursor(5, yPos + 4); M5Cardputer.Display.print("Enter:Open ;/.:Scroll  I:Info");

        if (!audioApp.songOffsets.empty()) {
            String fname = audioApp.getSongPath(audioApp.currentIndex); fname.toLowerCase();
            String codecText = "MP3"; uint16_t codecColor = C_ACCENT;
            if (fname.endsWith(".flac")) { codecText = "FLAC"; codecColor = C_PLAYING; }
            else if (fname.endsWith(".m4a") || fname.endsWith(".aac")) { codecText = "AAC"; codecColor = C_HIGHLIGHT; }
            else if (fname.endsWith(".wav")) { codecText = "WAV"; codecColor = TFT_ORANGE; }

            int boxW = 36, boxH = 14, boxX = M5Cardputer.Display.width() - boxW - 2, boxY = yPos + 2; 
            M5Cardputer.Display.fillRoundRect(boxX, boxY, boxW, boxH, 3, codecColor); M5Cardputer.Display.setTextColor(C_BG_DARK); 
            M5Cardputer.Display.setCursor(boxX + ((boxW - M5Cardputer.Display.textWidth(codecText.c_str())) / 2), boxY + 3);
            M5Cardputer.Display.print(codecText);
        }
    }

    static void drawPlaylist() {
        M5Cardputer.Display.setFont(&fonts::Font0);
        int totalAlbums = g_albumOffsets.size();
        int yPos = HEADER_HEIGHT + 2, xPos = 0;

        if (totalAlbums == 0) {
            M5Cardputer.Display.fillRect(0, HEADER_HEIGHT, PLAYLIST_WIDTH, M5Cardputer.Display.height() - HEADER_HEIGHT - BOTTOM_BAR_HEIGHT, C_BG_LIGHT);
            M5Cardputer.Display.drawFastVLine(PLAYLIST_WIDTH, HEADER_HEIGHT, M5Cardputer.Display.height() - HEADER_HEIGHT - BOTTOM_BAR_HEIGHT, C_BG_DARK);
            M5Cardputer.Display.setTextColor(C_TEXT_DIM);
            M5Cardputer.Display.setCursor(xPos + 5, yPos + 3);
            M5Cardputer.Display.print("No albums");
            return;
        }

        int startIdx = max(0, min(g_albumCursor - (MAX_VISIBLE_ROWS / 2), totalAlbums - MAX_VISIBLE_ROWS));

        // Pre-read all visible album names into RAM before touching the display.
        // This eliminates the delay between clearing the sidebar and drawing rows,
        // which was causing visible flicker.
        String albumNames[MAX_VISIBLE_ROWS];
        int rowCount = 0;
        File f = SD.open(ALBUM_INDEX_FILE);
        for (int i = 0; i < MAX_VISIBLE_ROWS; i++) {
            int actualIdx = startIdx + i; if (actualIdx >= totalAlbums) break;
            if (f) {
                f.seek(g_albumOffsets[actualIdx]);
                String line = f.readStringUntil('\n'); line.trim();
                if (line.startsWith("@")) line = line.substring(1);
                albumNames[i] = line;
            }
            rowCount++;
        }
        if (f) f.close();

        // Now do all display writes with data already in RAM — no SD I/O gaps
        M5Cardputer.Display.fillRect(0, HEADER_HEIGHT, PLAYLIST_WIDTH, M5Cardputer.Display.height() - HEADER_HEIGHT - BOTTOM_BAR_HEIGHT, C_BG_LIGHT);
        M5Cardputer.Display.drawFastVLine(PLAYLIST_WIDTH, HEADER_HEIGHT, M5Cardputer.Display.height() - HEADER_HEIGHT - BOTTOM_BAR_HEIGHT, C_BG_DARK);

        for (int i = 0; i < rowCount; i++) {
            int actualIdx = startIdx + i;
            bool isCurrentAlbum = (audioApp.currentAlbum.length() > 0 && albumNames[i] == audioApp.currentAlbum);

            if (actualIdx == g_albumCursor) {
                M5Cardputer.Display.fillRect(xPos + 2, yPos, PLAYLIST_WIDTH - 6, ROW_HEIGHT, C_ACCENT);
                M5Cardputer.Display.setTextColor(C_BG_DARK);
            } else if (isCurrentAlbum) {
                M5Cardputer.Display.setTextColor(C_PLAYING);
            } else {
                M5Cardputer.Display.setTextColor(C_TEXT_DIM);
            }

            String dispName = albumNames[i];
            if (dispName.length() > 16) dispName = dispName.substring(0, 15) + "~";

            M5Cardputer.Display.setCursor(xPos + 5, yPos + 3);
            if (isCurrentAlbum && actualIdx != g_albumCursor) M5Cardputer.Display.print("> ");
            M5Cardputer.Display.print(dispName);
            yPos += ROW_HEIGHT;
        }
    }

    static void drawAlbumSongsHeader() {
        M5Cardputer.Display.fillRect(0, 0, M5Cardputer.Display.width(), HEADER_HEIGHT, C_HEADER);
        M5Cardputer.Display.setFont(&fonts::Font0);
        M5Cardputer.Display.setTextColor(C_TEXT_MAIN, C_HEADER);
        M5Cardputer.Display.setCursor(5, 5);
        String hdr = g_currentAlbumName;
        if (hdr.length() > 30) hdr = hdr.substring(0, 29) + "~";
        M5Cardputer.Display.print(hdr);
    }

    static void drawAlbumSongsFooter() {
        int footerY = M5Cardputer.Display.height() - BOTTOM_BAR_HEIGHT;
        M5Cardputer.Display.fillRect(0, footerY, M5Cardputer.Display.width(), BOTTOM_BAR_HEIGHT, C_HEADER);
        M5Cardputer.Display.setTextColor(C_TEXT_DIM, C_HEADER);
        M5Cardputer.Display.setCursor(5, footerY + 4);
        M5Cardputer.Display.print("Enter:Play  ;/.:Scroll  `:Back");
    }

    // Redraws only the song list rows — not the header or footer.
    // This is the function called on scroll actions.
    static void drawAlbumSongsList() {
        M5Cardputer.Display.setFont(&fonts::Font0);
        int totalSongs = g_albumSongOffsets.size();

        // Content area bounds (between header and footer)
        int contentY = HEADER_HEIGHT + 4;
        int contentH = M5Cardputer.Display.height() - HEADER_HEIGHT - BOTTOM_BAR_HEIGHT;

        // Pre-read all visible row data from SD into RAM before touching the display
        struct RowData { String songPath; String displayName; };
        RowData rows[MAX_VISIBLE_ROWS];
        int rowCount = 0;
        String currentPlayingPath = audioApp.getSongPath(audioApp.currentIndex);

        if (totalSongs > 0) {
            File idxFile = SD.open(ALBUM_INDEX_FILE);
            for (int i = 0; i < MAX_VISIBLE_ROWS; i++) {
                int ri = g_albumSongScrollOffset + i;
                if (ri >= totalSongs) break;
                if (idxFile) {
                    idxFile.seek(g_albumSongOffsets[ri]);
                    String line = idxFile.readStringUntil('\n'); line.trim();
                    int pipePos = line.indexOf('|');
                    if (pipePos >= 0) {
                        String trackStr = line.substring(0, pipePos);
                        rows[rowCount].songPath = line.substring(pipePos + 1); rows[rowCount].songPath.trim();
                        if (rows[rowCount].songPath.length() > 0 && !rows[rowCount].songPath.startsWith("/"))
                            rows[rowCount].songPath = "/" + rows[rowCount].songPath;
                        String sp = rows[rowCount].songPath;
                        int slash = sp.lastIndexOf('/');
                        String fname = (slash >= 0) ? sp.substring(slash + 1) : sp;
                        int dotPos = fname.lastIndexOf('.');
                        if (dotPos > 0) fname = fname.substring(0, dotPos);
                        rows[rowCount].displayName = trackStr + " " + fname;
                        if (rows[rowCount].displayName.length() > 30)
                            rows[rowCount].displayName = rows[rowCount].displayName.substring(0, 29) + "~";
                    }
                }
                rowCount++;
            }
            if (idxFile) idxFile.close();
        }

        // All data in RAM — now clear only the content area and draw rows
        M5Cardputer.Display.fillRect(0, HEADER_HEIGHT, M5Cardputer.Display.width(), contentH, C_BG_DARK);

        int yPos = contentY;

        if (totalSongs == 0) {
            M5Cardputer.Display.setCursor(10, yPos);
            M5Cardputer.Display.setTextColor(C_TEXT_DIM);
            M5Cardputer.Display.print("No songs.");
        } else {
            for (int i = 0; i < rowCount; i++) {
                int ri = g_albumSongScrollOffset + i;
                bool isSelected = (ri == g_albumSongCursor);
                bool isPlaying = (rows[i].songPath == currentPlayingPath);

                if (isSelected) {
                    M5Cardputer.Display.fillRect(2, yPos - 1, M5Cardputer.Display.width() - 4, ROW_HEIGHT, C_ACCENT);
                    M5Cardputer.Display.setTextColor(C_BG_DARK);
                } else if (isPlaying) {
                    M5Cardputer.Display.setTextColor(C_PLAYING);
                } else {
                    M5Cardputer.Display.setTextColor(C_TEXT_MAIN);
                }

                M5Cardputer.Display.setCursor(6, yPos + 2);
                if (isPlaying && !isSelected) M5Cardputer.Display.print("> ");
                M5Cardputer.Display.print(rows[i].displayName);
                yPos += ROW_HEIGHT;
            }

            // Scroll arrows
            M5Cardputer.Display.setTextColor(C_ACCENT);
            if (g_albumSongScrollOffset > 0)
                M5Cardputer.Display.drawString("^", M5Cardputer.Display.width() - 10, HEADER_HEIGHT + 4);
            if (g_albumSongScrollOffset + MAX_VISIBLE_ROWS < totalSongs)
                M5Cardputer.Display.drawString("v", M5Cardputer.Display.width() - 10, HEADER_HEIGHT + (MAX_VISIBLE_ROWS * ROW_HEIGHT) - 6);
        }
    }

    // Full draw — called once when entering the album songs view
    static void drawAlbumSongs() {
        drawAlbumSongsHeader();
        drawAlbumSongsList();
        drawAlbumSongsFooter();
    }

    static void drawNowPlaying() {
        int xStart = PLAYLIST_WIDTH + 5, yStart = HEADER_HEIGHT + 5;
        M5Cardputer.Display.fillRect(xStart, yStart, M5Cardputer.Display.width() - xStart, 50, C_BG_DARK);
        
        M5Cardputer.Display.setFont(&fonts::lgfxJapanGothic_12); M5Cardputer.Display.setTextColor(audioApp.isPaused ? TFT_ORANGE : C_PLAYING);
        M5Cardputer.Display.setCursor(xStart + 5, yStart); M5Cardputer.Display.print(audioApp.isPaused ? "[PAUSED]" : "PLAYING >");

        M5Cardputer.Display.setCursor(M5Cardputer.Display.width() - 55, yStart);
        if (audioApp.isShuffle) { M5Cardputer.Display.setTextColor(C_HIGHLIGHT); M5Cardputer.Display.print("SHF "); } else { M5Cardputer.Display.setTextColor(C_BG_LIGHT); M5Cardputer.Display.print("___ "); }

        switch(audioApp.loopMode) {
            case NO_LOOP: M5Cardputer.Display.setTextColor(C_BG_LIGHT); M5Cardputer.Display.print("1X"); break;
            case LOOP_ALL: M5Cardputer.Display.setTextColor(C_ACCENT); M5Cardputer.Display.print("ALL"); break;
            case LOOP_ONE: M5Cardputer.Display.setTextColor(C_HIGHLIGHT); M5Cardputer.Display.print("ONE"); break;
        }

        M5Cardputer.Display.setTextColor(C_TEXT_MAIN); M5Cardputer.Display.setCursor(xStart + 5, yStart + 16);
        if (audioApp.currentTitle.length() > 0) M5Cardputer.Display.print(audioApp.currentTitle.substring(0, 15));
        
        {
            int maxW = M5Cardputer.Display.width() - xStart - 10;
            float pos = 0, size = 1;
            if (audioApp.isPaused && audioApp.pausedSize > 0) {
                pos = audioApp.paused_at; size = audioApp.pausedSize;
            } else if (audioApp.id3 && audioApp.file) {
                pos = audioApp.id3->getPos(); size = audioApp.id3->getSize();
            }
            int curW = (size > 0) ? (int)(pos / size * maxW) : 0;
            M5Cardputer.Display.fillRect(xStart-3, yStart+30-3, maxW+6, 9, C_BG_DARK); M5Cardputer.Display.fillRect(xStart, yStart+30, maxW, 3, C_BG_LIGHT);
            M5Cardputer.Display.fillRect(xStart, yStart+30, min(curW, maxW), 3, C_HIGHLIGHT); M5Cardputer.Display.fillCircle(xStart + min(curW, maxW), yStart+30+1, 3, C_TEXT_MAIN);
        }

        int volY = yStart + 42; M5Cardputer.Display.setCursor(xStart + 5, volY); M5Cardputer.Display.setFont(&fonts::Font0);
        M5Cardputer.Display.setTextColor(C_ACCENT); M5Cardputer.Display.print("VOL "); M5Cardputer.Display.drawRect(xStart + 30, volY, 60, 6, C_BG_LIGHT);
        M5Cardputer.Display.fillRect(xStart + 31, volY + 1, (M5Cardputer.Speaker.getVolume() * 58) / 255, 4, C_ACCENT);
    }

    static void drawVisNowPlayingInfo(int textX, int maxChars) {
        String artist = audioApp.currentArtist.length() > 0 ? audioApp.currentArtist : "Unknown Artist";
        String album = audioApp.currentAlbum.length() > 0 ? audioApp.currentAlbum : "Unknown Album";
        visSprite.setFont(&fonts::Font0);
        visSprite.setTextColor(C_TEXT_MAIN); visSprite.setCursor(textX, 4); visSprite.print(artist.substring(0, maxChars));
        visSprite.setTextColor(C_TEXT_DIM); visSprite.setCursor(textX, 16); visSprite.print(album.substring(0, maxChars));
        int elapsedSec = 0, totalSec = 0;
        if (audioApp.isPaused && audioApp.pausedSize > 0) {
            elapsedSec = audioApp.paused_at / 16000;
            totalSec = audioApp.pausedSize / 16000;
        } else if (audioApp.id3 && audioApp.id3->getSize() > 0) {
            elapsedSec = audioApp.id3->getPos() / 16000;
            totalSec = audioApp.id3->getSize() / 16000;
        }
        char timeStr[16];
        sprintf(timeStr, "%02d:%02d/%02d:%02d", elapsedSec / 60, elapsedSec % 60, totalSec / 60, totalSec % 60);
        visSprite.setTextColor(C_HIGHLIGHT); visSprite.setCursor(textX, 28); visSprite.print(timeStr);
    }

    static void drawVisualizer() {
        if (!showVisualizer) return;
        // Info modes (3, 4) render even when paused; audio modes (0-2) need active decoder
        bool isInfoMode = (userSettings.visMode == 3 || userSettings.visMode == 4);
        if (!isInfoMode && (!audioApp.decoder || !audioApp.decoder->isRunning() || audioApp.isPaused)) return;
        if (userSettings.visMode >= NUM_VIS_MODES) return; // OFF

        if (userSettings.visMode == 3) {
            // Animated art + info text
            visSprite.fillScreen(C_BG_DARK);
            visSprite.fillRoundRect(2, 2, 38, 38, 4, C_BG_LIGHT);
            visSprite.drawRoundRect(2, 2, 38, 38, 4, C_ACCENT);
            int cx = 21, cy = 21;
            int animType = audioApp.currentIndex % 4;
            switch (animType) {
                case 0: {
                    int r = 16; float angle = millis() / 400.0;
                    visSprite.fillCircle(cx, cy, r, C_BG_DARK); visSprite.drawCircle(cx, cy, r, C_TEXT_DIM);
                    visSprite.drawCircle(cx, cy, r - 4, 0x0000); visSprite.drawCircle(cx, cy, r - 8, 0x0000);
                    visSprite.fillCircle(cx, cy, 6, C_ACCENT);
                    visSprite.fillCircle(cx + (cos(angle) * 3), cy + (sin(angle) * 3), 2, C_BG_DARK);
                    visSprite.fillCircle(cx, cy, 2, C_BG_DARK);
                    visSprite.drawLine(35, 5, 26, 15, C_TEXT_MAIN); visSprite.fillCircle(35, 5, 3, C_TEXT_DIM); visSprite.fillRect(24, 14, 4, 6, C_HIGHLIGHT);
                    break;
                }
                case 1: {
                    float angle = millis() / 200.0;
                    visSprite.fillRoundRect(cx - 14, cy - 9, 28, 18, 2, C_TEXT_DIM); visSprite.fillRoundRect(cx - 8, cy - 3, 16, 6, 1, C_BG_DARK);
                    int lx = cx - 5, ly = cy; visSprite.drawCircle(lx, ly, 3, C_TEXT_MAIN);
                    visSprite.drawLine(lx - cos(angle)*3, ly - sin(angle)*3, lx + cos(angle)*3, ly + sin(angle)*3, C_TEXT_MAIN);
                    int rx = cx + 5, ry = cy; visSprite.drawCircle(rx, ry, 3, C_TEXT_MAIN);
                    visSprite.drawLine(rx - cos(angle)*3, ry - sin(angle)*3, rx + cos(angle)*3, ry + sin(angle)*3, C_TEXT_MAIN);
                    visSprite.drawLine(cx - 6, cy + 7, cx + 6, cy + 7, C_BG_DARK); visSprite.drawLine(cx - 4, cy + 8, cx + 4, cy + 8, C_BG_DARK);
                    break;
                }
                case 2: {
                    float pulse = sin(millis() / 150.0); int r = 10 + (pulse * 2);
                    visSprite.fillRect(cx - 12, cy - 15, 24, 30, C_TEXT_DIM); visSprite.drawRect(cx - 12, cy - 15, 24, 30, C_TEXT_MAIN);
                    visSprite.fillCircle(cx, cy - 8, 4, C_BG_DARK); visSprite.drawCircle(cx, cy - 8, 2, C_BG_LIGHT);
                    visSprite.fillCircle(cx, cy + 4, 12, C_BG_DARK); visSprite.fillCircle(cx, cy + 4, r, C_TEXT_DIM);
                    visSprite.fillCircle(cx, cy + 4, r - 3, C_BG_DARK); visSprite.fillCircle(cx, cy + 4, 3, C_ACCENT);
                    break;
                }
                case 3: {
                    int r = 16; float angle = millis() / 300.0;
                    visSprite.fillCircle(cx, cy, r, C_TEXT_MAIN); visSprite.drawCircle(cx, cy, r, C_TEXT_DIM);
                    float a2 = angle + PI/4;
                    visSprite.fillTriangle(cx, cy, cx + cos(angle)*r, cy + sin(angle)*r, cx + cos(a2)*r, cy + sin(a2)*r, C_HIGHLIGHT);
                    float a3 = angle + PI, a4 = angle + PI + PI/4;
                    visSprite.fillTriangle(cx, cy, cx + cos(a3)*r, cy + sin(a3)*r, cx + cos(a4)*r, cy + sin(a4)*r, C_ACCENT);
                    visSprite.fillCircle(cx, cy, 6, C_BG_DARK); visSprite.drawCircle(cx, cy, 6, C_TEXT_DIM); visSprite.fillCircle(cx, cy, 2, C_BG_LIGHT);
                    break;
                }
            }
            drawVisNowPlayingInfo(45, 12);
            visSprite.pushSprite(PLAYLIST_WIDTH + 2, HEADER_HEIGHT + 55);
            return;
        }

        if (userSettings.visMode == 4) {
            // Text-only info — no animation, full width for text
            visSprite.fillScreen(C_BG_DARK);
            drawVisNowPlayingInfo(4, 18);
            visSprite.pushSprite(PLAYLIST_WIDTH + 2, HEADER_HEIGHT + 55);
            return;
        }

        auto buf = out->getBuffer();
        if (buf) {
            memcpy(raw_data, buf, WAVE_SIZE * 2 * sizeof(int16_t));
            fft.exec(raw_data);
            visSprite.fillScreen(C_BG_DARK);
            int visW = visSprite.width(), visH = visSprite.height();

            if (userSettings.visMode == 0) {
                for (size_t bx = 0; bx < min((int)(visW / 4), FFT_SIZE / 2); ++bx) {
                    int32_t barH = min((int)((fft.get(bx) * visH) >> 16), visH);
                    uint16_t color = barH < visH * 0.4 ? C_ACCENT : (barH < visH * 0.7 ? C_HIGHLIGHT : TFT_RED);
                    visSprite.fillRect(bx * 4, visH - barH, 3, barH, color);
                }
            }
            else if (userSettings.visMode == 1) {
                int prevX = 0, prevY = visH;
                int numPoints = min((int)visW, FFT_SIZE / 2);
                float step = (float)visW / numPoints;
                for (size_t bx = 0; bx < (size_t)numPoints; ++bx) {
                    int32_t val = min((int)((fft.get(bx) * visH) >> 16), visH);
                    int x = (int)(bx * step), y = visH - val;
                    if (bx > 0) visSprite.drawLine(prevX, prevY, x, y, C_ACCENT);
                    visSprite.drawLine(x, y, x, visH, C_BG_LIGHT);
                    prevX = x; prevY = y;
                }
            }
            else if (userSettings.visMode == 2) {
                int cx = visW / 2, cy = visH / 2, baseR = 12, numBins = 32;
                float angleStep = 2.0 * PI / numBins;
                for (int i = 0; i < numBins; i++) {
                    int32_t val = min((int)((fft.get(i) * (visH / 2)) >> 16), visH / 2);
                    float angle = i * angleStep;
                    int x1 = cx + cos(angle) * baseR, y1 = cy + sin(angle) * baseR;
                    int x2 = cx + cos(angle) * (baseR + val), y2 = cy + sin(angle) * (baseR + val);
                    uint16_t color = val < (visH / 4) ? C_ACCENT : C_HIGHLIGHT;
                    visSprite.drawLine(x1, y1, x2, y2, color);
                    float mirrorAngle = angle + PI;
                    int mx1 = cx + cos(mirrorAngle) * baseR, my1 = cy + sin(mirrorAngle) * baseR;
                    int mx2 = cx + cos(mirrorAngle) * (baseR + val), my2 = cy + sin(mirrorAngle) * (baseR + val);
                    visSprite.drawLine(mx1, my1, mx2, my2, color);
                }
                visSprite.drawCircle(cx, cy, baseR, C_PLAYING);
            }
            visSprite.pushSprite(PLAYLIST_WIDTH + 2, HEADER_HEIGHT + 55);
        }
    }

    static void drawSettings() {
        drawPopup("SETTINGS", "Press 'Esc' to Exit");
        int startY = 45, gap = 20;
        int items = numSettings;
        for (int i = 0; i < 4; i++) {
            int idx = menuScrollOffset + i; if (idx >= items) break;
            M5Cardputer.Display.setCursor(25, startY + (i * gap));
            M5Cardputer.Display.setTextColor(idx == settingsCursor ? (idx == 4 ? TFT_RED : C_HIGHLIGHT) : C_TEXT_MAIN);

            switch (idx) {
                case 0: M5Cardputer.Display.printf("Brightness: %d", userSettings.brightness); break;
                case 1: M5Cardputer.Display.printf("Screen Off: %s", timeoutLabels[userSettings.timeoutIndex]); break;
                case 2: M5Cardputer.Display.printf("Resume Play: %s", userSettings.resumePlay ? "ON" : "OFF"); break;
                case 3: M5Cardputer.Display.printf("DAC Rate: %s", sampleRateLabels[userSettings.spkRateIndex]); break;
                case 4: M5Cardputer.Display.printf("Seek Value: %d", userSettings.seek); break;
                case 5: M5Cardputer.Display.printf("Wi-Fi Power: %s", userSettings.wifiEnabled ? "ON" : "OFF"); break;
                case 6: M5Cardputer.Display.printf("Wi-Fi Mode: %s", userSettings.isAPMode ? "AP (Host)" : "STA (Client)"); break; 
                case 7: M5Cardputer.Display.printf("Power Saver: %s", powerModeLabels[userSettings.powerSaverMode]); break;
                case 8: M5Cardputer.Display.printf("Theme: %s", themeLabels[userSettings.themeIndex]); break;
                case 9: M5Cardputer.Display.printf("Visualizer: %s", visModeLabels[userSettings.visMode]); break;
                case 10: M5Cardputer.Display.print("> Setup Wi-Fi Network"); break; 
                case 11: M5Cardputer.Display.print("> Setup AP (Host)"); break;
                case 12: M5Cardputer.Display.print("[ RESCAN LIBRARY ]"); break;    
                case 13: M5Cardputer.Display.print("[ EXPORT CONFIG TO SD ]"); break; 
                case 14: M5Cardputer.Display.print("[ IMPORT FROM SD ]"); break;
                case 15: M5Cardputer.Display.printf("Splash Screen: %s", userSettings.showSplash ? "ON" : "OFF"); break;
            }
        }
    }

    static void drawWifiScanner() {
        M5Cardputer.Display.fillScreen(C_BG_DARK); M5Cardputer.Display.fillRect(0, 0, M5Cardputer.Display.width(), HEADER_HEIGHT, C_HEADER);
        M5Cardputer.Display.setTextColor(C_TEXT_MAIN); M5Cardputer.Display.setCursor(5, 5); M5Cardputer.Display.print("Select Wi-Fi Network");
        int yPos = HEADER_HEIGHT + 5;
        if (wifiNetworkCount == 0) { M5Cardputer.Display.setCursor(10, yPos); M5Cardputer.Display.print("No networks found."); return; }
        for (int i = 0; i < 6; i++) {
            int idx = wifiScrollOffset + i; if (idx >= wifiNetworkCount) break;
            if (idx == wifiCursor) { M5Cardputer.Display.fillRect(2, yPos - 2, M5Cardputer.Display.width() - 4, ROW_HEIGHT, C_ACCENT); M5Cardputer.Display.setTextColor(C_BG_DARK); } 
            else M5Cardputer.Display.setTextColor(C_TEXT_MAIN);
            M5Cardputer.Display.setCursor(10, yPos); M5Cardputer.Display.print(WiFi.SSID(idx).substring(0, 30)); yPos += ROW_HEIGHT;
        }
    }

    static void drawTextInput() {
        M5Cardputer.Display.fillScreen(C_BG_DARK); M5Cardputer.Display.fillRect(0, 0, M5Cardputer.Display.width(), HEADER_HEIGHT, C_HEADER);
        M5Cardputer.Display.setTextColor(C_TEXT_MAIN); M5Cardputer.Display.setCursor(5, 5); 
        if (textInputTarget == 0) M5Cardputer.Display.print("Enter Client Password:");
        else if (textInputTarget == 1) M5Cardputer.Display.print("Set AP Name (SSID):");
        else if (textInputTarget == 2) M5Cardputer.Display.print("Set AP Password (8+ chars):");

        M5Cardputer.Display.drawRect(10, HEADER_HEIGHT + 35, M5Cardputer.Display.width() - 20, 25, C_TEXT_MAIN);
        M5Cardputer.Display.setTextColor(C_TEXT_MAIN); M5Cardputer.Display.setCursor(15, HEADER_HEIGHT + 40);
        
        String displayStr = enteredText;
        if (textInputTarget == 0) { displayStr = ""; for(int i=0; i<(int)enteredText.length(); i++) displayStr += "*"; }
        M5Cardputer.Display.print(displayStr + "_"); 
        M5Cardputer.Display.setCursor(10, M5Cardputer.Display.height() - 20); M5Cardputer.Display.setTextColor(C_TEXT_DIM); M5Cardputer.Display.print("ENTER to Save  |  ` to Cancel");
    }

    static void drawBaseUI() {
        M5Cardputer.Display.fillRect(0, HEADER_HEIGHT, M5Cardputer.Display.width(), M5Cardputer.Display.height() - HEADER_HEIGHT, C_BG_DARK);
        drawHeader(); drawPlaylist(); drawNowPlaying(); drawBottomBar();
        if(!showVisualizer) { visSprite.fillScreen(C_BG_DARK); visSprite.pushSprite(PLAYLIST_WIDTH + 2, HEADER_HEIGHT + 55); }
    }
};

int UIManager::settingsCursor = 0; int UIManager::menuScrollOffset = 0; bool UIManager::showVisualizer = true;
int UIManager::wifiCursor = 0; int UIManager::wifiScrollOffset = 0; int UIManager::wifiNetworkCount = 0;
int UIManager::helpScrollOffset = 0;

void AudioEngine::MDCallback(void *cbData, const char *type, bool isUnicode, const char *string) {
    if (string[0] == 0) return;
    if (strcmp(type, "Title") == 0) { audioApp.currentTitle = String(string); if (currentState == UI_PLAYER) UIManager::drawNowPlaying(); } 
    else if (strcmp(type, "Artist") == 0 || strcmp(type, "Performer") == 0) { audioApp.currentArtist = String(string); }
    else if (strcmp(type, "Album") == 0) { audioApp.currentAlbum = String(string); }
}

// ==========================================
// WEB SERVER MANAGER
// ==========================================
class WebServerManager {
public:
    static void setup() {
        if (!userSettings.wifiEnabled) { WiFi.mode(WIFI_OFF); return; }
        M5Cardputer.Display.fillScreen(C_BG_DARK); M5Cardputer.Display.setCursor(10, 40); M5Cardputer.Display.setTextColor(C_ACCENT);

        if (userSettings.isAPMode) {
            M5Cardputer.Display.print("Starting AP Mode..."); WiFi.mode(WIFI_AP);
            WiFi.softAP(userSettings.apSSID.c_str(), userSettings.apPass.c_str()); delay(1500);
        } else if (userSettings.wifiSSID.length() > 0) {
            M5Cardputer.Display.print("Connecting to Wi-Fi..."); WiFi.mode(WIFI_STA);
            WiFi.begin(userSettings.wifiSSID.c_str(), userSettings.wifiPass.c_str()); int retry = 0;
            while (WiFi.status() != WL_CONNECTED && retry < 15) { delay(500); M5Cardputer.Display.print("."); retry++; }
            if (WiFi.status() != WL_CONNECTED) { M5Cardputer.Display.print("\nFailed! Offline Mode."); delay(2000); return; }
        }

        server.on("/", []() {
            String html = R"rawliteral(
<!DOCTYPE html><html lang="en"><head><meta charset="UTF-8"><meta name="viewport" content="width=device-width, initial-scale=1.0"><title>Sam Music Player</title>
<script src="https://cdnjs.cloudflare.com/ajax/libs/jsmediatags/3.9.5/jsmediatags.min.js"></script>
<style>:root{--bg-dark:#15181C;--bg-light:#252830;--slate-blue:#4A5B82;--cyan:#00E5FF;--green:#00FF66;--magenta:#FF007F;}
body{font-family:'Segoe UI',sans-serif;background-color:var(--bg-dark);color:#fff;padding:20px;max-width:650px;margin:0 auto;}
h1{color:var(--cyan);text-align:center;font-weight:300;letter-spacing:2px;}
.player-card{background:var(--bg-light);padding:20px;border-radius:12px;box-shadow:0 8px 20px rgba(0,0,0,0.6);position:sticky;top:10px;z-index:100;border:1px solid #333;}
.now-playing-container{display:flex;align-items:center;gap:20px;margin-bottom:20px;}
.cover-wrapper{width:100px;height:100px;flex-shrink:0;background:var(--bg-dark);border-radius:8px;display:flex;justify-content:center;align-items:center;overflow:hidden;border:1px solid var(--slate-blue);}
#cover-art{width:100%;height:100%;object-fit:cover;display:none;} #cover-placeholder{color:var(--slate-blue);font-size:30px;}
.track-details{flex-grow:1;overflow:hidden;}
#track-title{font-size:1.3em;color:var(--green);white-space:nowrap;overflow:hidden;text-overflow:ellipsis;font-weight:bold;}
#track-artist{font-size:1em;color:var(--cyan);white-space:nowrap;overflow:hidden;text-overflow:ellipsis;}
#track-album{font-size:0.85em;color:#aaa;white-space:nowrap;overflow:hidden;text-overflow:ellipsis;}
audio{width:100%;margin-bottom:15px;outline:none;border-radius:5px;height:40px;}
.controls{display:flex;justify-content:center;gap:12px;margin-bottom:5px;}
button{background:var(--slate-blue);color:#fff;border:none;padding:10px 18px;border-radius:6px;cursor:pointer;font-size:16px;transition:0.2s;}
button:hover{background:var(--cyan);color:var(--bg-dark);transform:scale(1.05);} button.active{background:var(--magenta);color:#fff;}
.search-box{margin-top:25px;text-align:center;}
#searchBar{width:100%;padding:12px 20px;border-radius:25px;border:1px solid var(--slate-blue);background:var(--bg-dark);color:#fff;font-size:16px;outline:none;box-sizing:border-box;}
ul{list-style:none;padding:0;margin-top:20px;}
li{display:flex;justify-content:space-between;align-items:center;background:var(--bg-light);margin-bottom:8px;border-radius:6px;border-left:4px solid transparent;}
li:hover{background:var(--slate-blue);} li.playing{background:#1A2933;border-left:4px solid var(--cyan);}
.song-info{flex-grow:1;padding:14px;cursor:pointer;display:flex;align-items:center;}
.track-num{color:#888;margin-right:15px;font-size:0.85em;width:25px;text-align:right;}
li.playing .track-num, li.playing .track-name{color:var(--cyan);font-weight:bold;}
.dl-btn{background:#333;color:#fff;text-decoration:none;padding:8px 12px;border-radius:4px;margin-right:10px;font-size:1.1em;}
.dl-btn:hover{background:var(--magenta);}
</style></head><body>
<h1>SAM MUSIC PLAYER</h1>
<div class="player-card"><div class="now-playing-container"><div class="cover-wrapper"><div id="cover-placeholder">🎵</div><img id="cover-art" src=""></div>
<div class="track-details"><div id="track-title">Select a song</div><div id="track-artist"></div><div id="track-album"></div></div></div>
<audio id="player" controls></audio>
<div class="controls"><button id="btnPrev">⏮</button><button id="btnNext">⏭</button><button id="btnShuffle">🔀</button><button id="btnLoop">🔁</button></div></div>
<div class="search-box"><input type="text" id="searchBar" placeholder="Search..."></div>
<ul id="playlist"><li>Loading library...</li></ul>
<script>
let songs=[],currentIndex=-1,isShuffle=!1,loopMode=0;
const player=document.getElementById('player'),btnPrev=document.getElementById('btnPrev'),btnNext=document.getElementById('btnNext');
const btnShuffle=document.getElementById('btnShuffle'),btnLoop=document.getElementById('btnLoop'),playlistEl=document.getElementById('playlist'),searchBar=document.getElementById('searchBar');
const titleEl=document.getElementById('track-title'),artistEl=document.getElementById('track-artist'),albumEl=document.getElementById('track-album'),coverArt=document.getElementById('cover-art'),coverPlaceholder=document.getElementById('cover-placeholder');
fetch('/api/songs').then(r=>r.json()).then(data=>{songs=data;playlistEl.innerHTML='';
songs.forEach((song,idx)=>{let li=document.createElement('li');li.id='song-'+idx;li.className='song-item';
let infoDiv=document.createElement('div');infoDiv.className='song-info';infoDiv.innerHTML=`<span class="track-num">${idx+1}</span> <span class="track-name">${song}</span>`;
infoDiv.onclick=()=>playSong(idx);let dlLink=document.createElement('a');dlLink.className='dl-btn';dlLink.href='/download?id='+idx;dlLink.innerText='💾';
li.appendChild(infoDiv);li.appendChild(dlLink);playlistEl.appendChild(li);});}).catch(e=>playlistEl.innerHTML='Error');
searchBar.addEventListener('input',e=>{const term=e.target.value.toLowerCase();Array.from(playlistEl.getElementsByClassName('song-item')).forEach(i=>{i.style.display=i.querySelector('.track-name').innerText.toLowerCase().includes(term)?'flex':'none';});});
function playSong(index){if(index<0||index>=songs.length)return;if(currentIndex!==-1){let o=document.getElementById('song-'+currentIndex);if(o)o.classList.remove('playing');}
currentIndex=index;let n=document.getElementById('song-'+currentIndex);if(n){n.classList.add('playing');n.scrollIntoView({behavior:'smooth',block:'center'});}
const fileUrl='/stream?id='+currentIndex; player.src=''; titleEl.innerText='Loading...';artistEl.innerText='';albumEl.innerText='';coverArt.style.display='none';coverPlaceholder.style.display='block';
fetch(fileUrl+'&meta=1').then(r=>r.blob()).then(blob=>{window.jsmediatags.read(blob,{onSuccess:t=>{const tags=t.tags;titleEl.innerText=tags.title||songs[currentIndex];artistEl.innerText=tags.artist||'Unknown Artist';albumEl.innerText=tags.album||'';
if(tags.picture){try{const b=new Blob([new Uint8Array(tags.picture.data)],{type:tags.picture.format});coverArt.src=URL.createObjectURL(b);coverArt.style.display='block';coverPlaceholder.style.display='none';}catch(e){}}
player.src=fileUrl;player.play();},onError:e=>{titleEl.innerText=songs[currentIndex];player.src=fileUrl;player.play();}});}).catch(e=>{player.src=fileUrl;player.play();});}
function playNext(){if(!songs.length)return;if(loopMode===1){playSong(currentIndex);return;}if(isShuffle){let r;do{r=Math.floor(Math.random()*songs.length);}while(r===currentIndex&&songs.length>1);playSong(r);}else playSong((currentIndex+1)%songs.length);}
function playPrev(){if(!songs.length)return;if(currentIndex===-1)currentIndex=0;playSong((currentIndex-1+songs.length)%songs.length);}
btnNext.onclick=playNext;btnPrev.onclick=playPrev;btnShuffle.onclick=()=>{isShuffle=!isShuffle;btnShuffle.classList.toggle('active',isShuffle);};btnLoop.onclick=()=>{loopMode=(loopMode+1)%2;btnLoop.innerText=loopMode===1?'🔂':'🔁';btnLoop.classList.toggle('active',loopMode===1);};
player.addEventListener('ended',playNext);
</script></body></html>
)rawliteral";
            server.send(200, "text/html", html);
        });

        server.on("/api/songs", []() {
            if (!SD.exists(PLAYLIST_FILE)) { server.send(500, "text/plain", "No Playlist"); return; }
            server.setContentLength(CONTENT_LENGTH_UNKNOWN); server.send(200, "application/json", ""); server.sendContent("[\n");
            File f = SD.open(PLAYLIST_FILE); String jsonBuffer = ""; bool first = true;
            while (f.available()) {
                String line = f.readStringUntil('\n'); line.trim(); int slashIdx = line.lastIndexOf('/'); if (slashIdx >= 0) line = line.substring(slashIdx + 1);
                line.replace("\"", "\\\"");
                if (line.length() > 0) {
                    if (!first) jsonBuffer += ",\n"; jsonBuffer += "\"" + line + "\""; first = false;
                    if (jsonBuffer.length() > 1024) { server.sendContent(jsonBuffer); jsonBuffer = ""; }
                }
            }
            f.close(); if (jsonBuffer.length() > 0) server.sendContent(jsonBuffer);
            server.sendContent("\n]"); server.sendContent("");
        });

        server.on("/stream", []() {
            if (!server.hasArg("id")) { server.send(400, "text/plain", "Missing ID"); return; }
            String path = audioApp.getSongPath(server.arg("id").toInt()); File f = SD.open(path);
            if (!f) { server.send(404, "text/plain", "Not found"); return; }
            if (server.hasArg("meta")) {
                size_t chunkSize = min((size_t)655360, f.size()); server.setContentLength(chunkSize); server.send(200, "audio/mpeg", "");
                uint8_t buffer[2048]; size_t remaining = chunkSize;
                while (f.available() && remaining > 0) {
                    size_t bytesRead = f.read(buffer, min(remaining, sizeof(buffer)));
                    if (bytesRead == 0) break; server.client().write(buffer, bytesRead); remaining -= bytesRead;
                }
            } else server.streamFile(f, "audio/mpeg");
            f.close();
        });

        server.on("/download", []() {
            if (!server.hasArg("id")) { server.send(400, "text/plain", "Missing ID"); return; }
            String path = audioApp.getSongPath(server.arg("id").toInt()); File f = SD.open(path);
            if (!f) { server.send(404, "text/plain", "Not found"); return; }
            int slashIdx = path.lastIndexOf('/'); String filename = (slashIdx >= 0) ? path.substring(slashIdx + 1) : path;
            server.sendHeader("Content-Disposition", "attachment; filename=\"" + filename + "\""); server.streamFile(f, "audio/mpeg");
            f.close();
        });

        server.begin();
    }
};

// ==========================================
// BOOT MENU
// ==========================================
class BootMenu {
public:
    static int menuCursor;
    
    static void resetPreferences() {
        preferences.begin("sam_music", false);
        preferences.clear();
        preferences.end();
        
        M5Cardputer.Display.fillScreen(TFT_BLACK);
        M5Cardputer.Display.setFont(&fonts::Font0);
        M5Cardputer.Display.setTextColor(TFT_GREEN);
        M5Cardputer.Display.setCursor(40, 50);
        M5Cardputer.Display.print("Preferences Reset!");
        M5Cardputer.Display.setTextColor(TFT_WHITE);
        M5Cardputer.Display.setCursor(30, 70);
        M5Cardputer.Display.print("Restarting in 2 sec...");
        delay(2000);
        ESP.restart();
    }
    
    static USBMSC msc;
    static sdcard_type_t sdCardType;
    static uint32_t sdSectorCount;
    static uint32_t sdSectorSize;
    static SPIClass* sdSPI;
    
    static int32_t onMSCRead(uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize) {
        // Optimized multi-sector read
        uint32_t count = bufsize / 512;
        uint8_t* buf = (uint8_t*)buffer;
        
        // Try batch read first for better performance
        for (uint32_t i = 0; i < count; i++) {
            if (!SD.readRAW(buf + (i * 512), lba + i)) {
                return -1;
            }
        }
        return bufsize;
    }
    
    static int32_t onMSCWrite(uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize) {
        // Optimized multi-sector write
        uint32_t count = bufsize / 512;
        
        for (uint32_t i = 0; i < count; i++) {
            if (!SD.writeRAW(buffer + (i * 512), lba + i)) {
                return -1;
            }
        }
        return bufsize;
    }
    
    static bool onMSCStartStop(uint8_t power_condition, bool start, bool load_eject) {
        return true;
    }
    
    static void enableMassStorage() {
        M5Cardputer.Display.fillScreen(TFT_BLACK);
        M5Cardputer.Display.setFont(&fonts::Font0);
        M5Cardputer.Display.setTextColor(0x07FF);
        M5Cardputer.Display.setCursor(30, 25);
        M5Cardputer.Display.print("USB Mass Storage Mode");
        M5Cardputer.Display.setTextColor(TFT_WHITE);
        M5Cardputer.Display.setCursor(10, 50);
        M5Cardputer.Display.print("Initializing USB MSC...");
        
        // Get SD card info
        sdCardType = SD.cardType();
        sdSectorSize = 512;
        sdSectorCount = SD.cardSize() / sdSectorSize;
        
        if (sdCardType == CARD_NONE || sdSectorCount == 0) {
            M5Cardputer.Display.fillScreen(TFT_BLACK);
            M5Cardputer.Display.setTextColor(TFT_RED);
            M5Cardputer.Display.setCursor(30, 50);
            M5Cardputer.Display.print("SD Card Error!");
            M5Cardputer.Display.setTextColor(TFT_WHITE);
            M5Cardputer.Display.setCursor(20, 75);
            M5Cardputer.Display.print("Press RESET to retry");
            while(true) { delay(100); }
        }
        
        // Increase SPI speed for faster transfers
        SPI.setFrequency(40000000); // 40MHz for faster USB transfers
        
        msc.vendorID("M5Stack");
        msc.productID("Cardputer");
        msc.productRevision("1.0");
        msc.onRead(onMSCRead);
        msc.onWrite(onMSCWrite);
        msc.onStartStop(onMSCStartStop);
        msc.mediaPresent(true);
        msc.isWritable(true);
        msc.begin(sdSectorCount, sdSectorSize);
        
        USB.begin();
        
        M5Cardputer.Display.fillScreen(TFT_BLACK);
        M5Cardputer.Display.setTextColor(0x07E0);
        M5Cardputer.Display.setCursor(20, 25);
        M5Cardputer.Display.print("USB MSC Active!");
        M5Cardputer.Display.setTextColor(TFT_WHITE);
        M5Cardputer.Display.setCursor(10, 50);
        M5Cardputer.Display.print("SD Card: ");
        M5Cardputer.Display.print(SD.cardSize() / (1024 * 1024));
        M5Cardputer.Display.print(" MB");
        M5Cardputer.Display.setCursor(10, 70);
        M5Cardputer.Display.print("Connect USB to PC now");
        M5Cardputer.Display.setTextColor(TFT_YELLOW);
        M5Cardputer.Display.setCursor(15, 95);
        M5Cardputer.Display.print("Press RESET to exit");
        M5Cardputer.Display.setTextColor(0x07FF);
        M5Cardputer.Display.setCursor(30, 115);
        M5Cardputer.Display.print("Mode Active...");
        
        while(true) {
            M5Cardputer.update();
            delay(100);
        }
    }
    
    static void draw() {
        M5Cardputer.Display.fillScreen(TFT_BLACK);
        
        // Header
        M5Cardputer.Display.fillRect(0, 0, 240, 25, 0x18E3);
        M5Cardputer.Display.setFont(&fonts::Font0);
        M5Cardputer.Display.setTextColor(0x07FF, 0x18E3); // Cyan
        M5Cardputer.Display.setCursor(60, 8);
        M5Cardputer.Display.print("BOOT MENU");
        
        // Menu options
        const char* options[] = {
            "Continue Normal Boot",
            "Reset All Preferences", 
            "USB Mass Storage Mode"
        };
        int numOptions = 3;
        
        int startY = 40;
        int rowHeight = 25;
        
        for (int i = 0; i < numOptions; i++) {
            int y = startY + (i * rowHeight);
            
            if (i == menuCursor) {
                M5Cardputer.Display.fillRoundRect(10, y, 220, 20, 3, 0x05BF); // Accent blue
                M5Cardputer.Display.setTextColor(TFT_BLACK);
            } else {
                M5Cardputer.Display.setTextColor(TFT_WHITE);
            }
            
            M5Cardputer.Display.setCursor(20, y + 5);
            M5Cardputer.Display.print(options[i]);
            
            // Add icons/indicators
            if (i == 1) {
                M5Cardputer.Display.setTextColor(i == menuCursor ? TFT_BLACK : TFT_RED);
                M5Cardputer.Display.setCursor(200, y + 5);
                M5Cardputer.Display.print("!");
            } else if (i == 2) {
                M5Cardputer.Display.setTextColor(i == menuCursor ? TFT_BLACK : TFT_GREEN);
                M5Cardputer.Display.setCursor(200, y + 5);
                M5Cardputer.Display.print("U");
            }
        }
        
        // Footer instructions
        M5Cardputer.Display.fillRect(0, 117, 240, 18, 0x18E3);
        M5Cardputer.Display.setTextColor(0x9492, 0x18E3);
        M5Cardputer.Display.setCursor(5, 121);
        M5Cardputer.Display.print(";/. Navigate  Enter:Select");
    }
    
    static bool show() {
        menuCursor = 0;
        draw();
        
        while (true) {
            M5Cardputer.update();
            
            if (M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed()) {
                if (M5Cardputer.Keyboard.isKeyPressed(';')) {
                    // Up
                    menuCursor--;
                    if (menuCursor < 0) menuCursor = 2;
                    draw();
                }
                else if (M5Cardputer.Keyboard.isKeyPressed('.')) {
                    // Down
                    menuCursor++;
                    if (menuCursor > 2) menuCursor = 0;
                    draw();
                }
                else if (M5Cardputer.Keyboard.isKeyPressed(KEY_ENTER)) {
                    switch (menuCursor) {
                        case 0: // Continue normal boot
                            return true;
                        case 1: // Reset preferences
                            resetPreferences();
                            return false; // Won't reach here due to restart
                        case 2: // USB Mass Storage
                            enableMassStorage();
                            return false; // Won't reach here
                    }
                }
                else if (M5Cardputer.Keyboard.isKeyPressed('`')) {
                    // Escape - continue normal boot
                    return true;
                }
            }
            delay(50);
        }
        return true;
    }
};

int BootMenu::menuCursor = 0;
USBMSC BootMenu::msc;
sdcard_type_t BootMenu::sdCardType = CARD_NONE;
uint32_t BootMenu::sdSectorCount = 0;
uint32_t BootMenu::sdSectorSize = 512;

// ==========================================
// SPLASH SCREEN ANIMATION - 8-BIT NES STYLE
// ==========================================
class SplashScreen {
public:
    // NES-style color palette (limited colors like real NES)
    static const uint16_t NES_BLACK = 0x0000;
    static const uint16_t NES_WHITE = 0xFFFF;
    static const uint16_t NES_SKY = 0x5DBF;      // Light blue sky
    static const uint16_t NES_SKIN = 0xFCC0;     // Peach skin
    static const uint16_t NES_BROWN = 0x8200;    // Brown
    static const uint16_t NES_RED = 0xF800;      // Bright red
    static const uint16_t NES_BLUE = 0x001F;     // Blue
    static const uint16_t NES_GREEN = 0x07C0;    // Green
    static const uint16_t NES_DKGREEN = 0x03C0;  // Dark green
    static const uint16_t NES_YELLOW = 0xFFE0;   // Yellow
    static const uint16_t NES_ORANGE = 0xFC00;   // Orange
    static const uint16_t NES_GRAY = 0x8410;     // Gray
    static const uint16_t NES_DKGRAY = 0x4208;   // Dark gray
    static const uint16_t NES_CYAN = 0x07FF;     // Cyan
    static const uint16_t NES_MAGENTA = 0xF81F;  // Magenta
    
    // Pixel size for chunky 8-bit look (2x2 or 3x3 pixels)
    static const int PX = 2;
    
    // Draw a single "big pixel" (NES style chunky pixel)
    static void px(int x, int y, uint16_t c) {
        M5Cardputer.Display.fillRect(x * PX, y * PX, PX, PX, c);
    }
    
    // Draw 8-bit style character sprite (16x24 pixels, scaled)
    // Frame 0 = standing/walk1, Frame 1 = walk2
    static void drawBoy8bit(int x, int y, int frame) {
        // Clear sprite area first
        M5Cardputer.Display.fillRect(x * PX - PX, y * PX, 18 * PX, 26 * PX, NES_SKY);
        
        int f = frame % 4; // 4 frame walk cycle
        
        // Hair (dark brown) - row 0-2
        for(int i = 2; i < 7; i++) px(x+i, y, NES_DKGRAY);
        for(int i = 1; i < 8; i++) px(x+i, y+1, NES_DKGRAY);
        for(int i = 1; i < 8; i++) px(x+i, y+2, NES_DKGRAY);
        
        // Face (skin color) - row 3-6
        for(int i = 1; i < 8; i++) px(x+i, y+3, NES_SKIN);
        // Eyes row
        px(x+1, y+4, NES_SKIN); px(x+2, y+4, NES_BLACK); px(x+3, y+4, NES_SKIN);
        px(x+4, y+4, NES_SKIN); px(x+5, y+4, NES_BLACK); px(x+6, y+4, NES_SKIN);
        px(x+7, y+4, NES_SKIN);
        // Below eyes
        for(int i = 1; i < 8; i++) px(x+i, y+5, NES_SKIN);
        // Mouth row - smile
        px(x+1, y+6, NES_SKIN); px(x+2, y+6, NES_SKIN); px(x+3, y+6, NES_BLACK);
        px(x+4, y+6, NES_BLACK); px(x+5, y+6, NES_SKIN); px(x+6, y+6, NES_SKIN);
        px(x+7, y+6, NES_SKIN);
        
        // Headphones - RED ear cups
        px(x, y+3, NES_RED); px(x, y+4, NES_RED); px(x, y+5, NES_RED);
        px(x+8, y+3, NES_RED); px(x+8, y+4, NES_RED); px(x+8, y+5, NES_RED);
        // Headphone band on top
        for(int i = 1; i < 8; i++) px(x+i, y-1, NES_RED);
        
        // Body/Shirt (Blue) - row 7-11
        for(int j = 7; j <= 11; j++) {
            for(int i = 2; i < 7; i++) px(x+i, y+j, NES_BLUE);
        }
        
        // Arms (skin) - animated swing
        int armOffset = (f < 2) ? 0 : 1;
        // Left arm
        px(x+1, y+7+armOffset, NES_SKIN); px(x+1, y+8+armOffset, NES_SKIN);
        px(x+1, y+9+armOffset, NES_SKIN);
        // Right arm
        px(x+7, y+7+(1-armOffset), NES_SKIN); px(x+7, y+8+(1-armOffset), NES_SKIN);
        px(x+7, y+9+(1-armOffset), NES_SKIN);
        
        // Pants (dark gray) - row 12-14
        for(int j = 12; j <= 14; j++) {
            for(int i = 2; i < 7; i++) px(x+i, y+j, NES_DKGRAY);
        }
        
        // Legs with walk animation
        int legL = 0, legR = 0;
        switch(f) {
            case 0: legL = 0; legR = 2; break;  // Left back, right forward
            case 1: legL = 1; legR = 1; break;  // Both center
            case 2: legL = 2; legR = 0; break;  // Left forward, right back
            case 3: legL = 1; legR = 1; break;  // Both center
        }
        // Left leg + shoe
        px(x+2+legL, y+15, NES_DKGRAY); px(x+2+legL, y+16, NES_DKGRAY);
        px(x+2+legL, y+17, NES_BROWN); px(x+3+legL, y+17, NES_BROWN); // shoe
        // Right leg + shoe
        px(x+5-legR, y+15, NES_DKGRAY); px(x+5-legR, y+16, NES_DKGRAY);
        px(x+5-legR, y+17, NES_BROWN); px(x+6-legR, y+17, NES_BROWN); // shoe
    }
    
    // 8-bit music note (simple pixelated)
    static void drawNote8bit(int x, int y, uint16_t color) {
        // Note head
        px(x, y+2, color); px(x+1, y+2, color);
        px(x, y+3, color); px(x+1, y+3, color);
        // Stem
        px(x+1, y, color); px(x+1, y+1, color);
        // Flag
        px(x+2, y, color); px(x+2, y+1, color);
    }
    
    // 8-bit double note (beamed)
    static void drawDoubleNote8bit(int x, int y, uint16_t color) {
        // First note head
        px(x, y+2, color); px(x+1, y+2, color);
        // Second note head
        px(x+3, y+2, color); px(x+4, y+2, color);
        // Stems
        px(x+1, y, color); px(x+1, y+1, color);
        px(x+4, y, color); px(x+4, y+1, color);
        // Beam
        px(x+1, y, color); px(x+2, y, color); px(x+3, y, color); px(x+4, y, color);
    }
    
    // 8-bit house
    static void drawHouse8bit(int hx, int hy) {
        // Roof (red/brown triangle approximation in pixels)
        for(int i = 0; i < 8; i++) {
            for(int j = 8-i; j <= 8+i; j++) {
                px(hx+j, hy+i, NES_RED);
            }
        }
        // Walls (gray)
        for(int j = 8; j < 16; j++) {
            for(int i = 1; i < 16; i++) {
                px(hx+i, hy+j, NES_GRAY);
            }
        }
        // Door (brown)
        for(int j = 10; j < 16; j++) {
            px(hx+7, hy+j, NES_BROWN); px(hx+8, hy+j, NES_BROWN); px(hx+9, hy+j, NES_BROWN);
        }
        // Door knob
        px(hx+9, hy+13, NES_YELLOW);
        // Windows (cyan with white cross)
        px(hx+3, hy+10, NES_CYAN); px(hx+4, hy+10, NES_CYAN);
        px(hx+3, hy+11, NES_CYAN); px(hx+4, hy+11, NES_CYAN);
        px(hx+12, hy+10, NES_CYAN); px(hx+13, hy+10, NES_CYAN);
        px(hx+12, hy+11, NES_CYAN); px(hx+13, hy+11, NES_CYAN);
        // Chimney
        px(hx+12, hy+2, NES_BROWN); px(hx+13, hy+2, NES_BROWN);
        px(hx+12, hy+3, NES_BROWN); px(hx+13, hy+3, NES_BROWN);
        px(hx+12, hy+4, NES_BROWN); px(hx+13, hy+4, NES_BROWN);
    }
    
    // 8-bit tree
    static void drawTree8bit(int tx, int ty) {
        // Trunk (brown)
        px(tx+2, ty+6, NES_BROWN); px(tx+3, ty+6, NES_BROWN);
        px(tx+2, ty+7, NES_BROWN); px(tx+3, ty+7, NES_BROWN);
        px(tx+2, ty+8, NES_BROWN); px(tx+3, ty+8, NES_BROWN);
        // Leaves (green - triangle-ish)
        for(int i = 0; i < 6; i++) px(tx+i, ty+5, NES_GREEN);
        for(int i = 0; i < 6; i++) px(tx+i, ty+4, NES_GREEN);
        for(int i = 1; i < 5; i++) px(tx+i, ty+3, NES_GREEN);
        for(int i = 1; i < 5; i++) px(tx+i, ty+2, NES_DKGREEN);
        px(tx+2, ty+1, NES_DKGREEN); px(tx+3, ty+1, NES_DKGREEN);
        px(tx+2, ty, NES_GREEN); px(tx+3, ty, NES_GREEN);
    }
    
    // 8-bit cloud
    static void drawCloud8bit(int cx, int cy) {
        // Simple blocky cloud
        for(int i = 1; i < 6; i++) px(cx+i, cy, NES_WHITE);
        for(int i = 0; i < 7; i++) px(cx+i, cy+1, NES_WHITE);
        for(int i = 1; i < 6; i++) px(cx+i, cy+2, NES_WHITE);
    }
    
    // 8-bit sun
    static void drawSun8bit(int sx, int sy, int frame) {
        // Sun body
        for(int j = 0; j < 4; j++) {
            for(int i = 0; i < 4; i++) {
                px(sx+i, sy+j, NES_YELLOW);
            }
        }
        // Animated rays (alternating pattern)
        int rayPhase = (frame / 5) % 2;
        if(rayPhase == 0) {
            px(sx+1, sy-1, NES_YELLOW); px(sx+2, sy-1, NES_YELLOW);
            px(sx-1, sy+1, NES_YELLOW); px(sx+4, sy+1, NES_YELLOW);
            px(sx-1, sy+2, NES_YELLOW); px(sx+4, sy+2, NES_YELLOW);
            px(sx+1, sy+4, NES_YELLOW); px(sx+2, sy+4, NES_YELLOW);
        } else {
            px(sx-1, sy-1, NES_ORANGE); px(sx+4, sy-1, NES_ORANGE);
            px(sx+1, sy-1, NES_YELLOW); px(sx+2, sy-1, NES_YELLOW);
            px(sx-1, sy+4, NES_ORANGE); px(sx+4, sy+4, NES_ORANGE);
            px(sx+1, sy+4, NES_YELLOW); px(sx+2, sy+4, NES_YELLOW);
        }
    }
    
    // 8-bit ground with grass detail
    static void drawGround8bit() {
        // Grass layer
        int groundY = 52;
        for(int y = groundY; y < groundY + 5; y++) {
            for(int x = 0; x < 120; x++) {
                px(x, y, NES_GREEN);
            }
        }
        // Grass detail (darker patches)
        for(int x = 0; x < 120; x += 5) {
            px(x, groundY, NES_DKGREEN);
            px(x+2, groundY, NES_DKGREEN);
        }
        // Path/road
        for(int y = groundY + 5; y < 68; y++) {
            for(int x = 0; x < 120; x++) {
                px(x, y, NES_DKGRAY);
            }
        }
        // Road stripes (yellow dashes)
        for(int x = 0; x < 120; x += 10) {
            for(int i = 0; i < 5; i++) {
                px(x+i, groundY + 8, NES_YELLOW);
            }
        }
    }
    
    // Draw retro 8-bit title with pixel font effect
    static void drawTitle8bit(int frame) {
        // Title background bar
        M5Cardputer.Display.fillRect(0, 0, 240, 22, NES_BLACK);
        
        // "SAM MUSIC PLAYER" - simple pixel text with color cycling
        uint16_t colors[] = {NES_CYAN, NES_WHITE, NES_YELLOW, NES_MAGENTA};
        uint16_t titleColor = colors[(frame / 10) % 4];
        
        M5Cardputer.Display.setFont(&fonts::Font0);
        M5Cardputer.Display.setTextColor(titleColor);
        M5Cardputer.Display.setCursor(45, 7);
        M5Cardputer.Display.print("SAM MUSIC PLAYER");
        
        // Decorative pixels around title (NES style)
        px(10, 3, NES_CYAN); px(11, 4, NES_MAGENTA); px(10, 5, NES_YELLOW);
        px(108, 3, NES_CYAN); px(109, 4, NES_MAGENTA); px(108, 5, NES_YELLOW);
    }
    
    // Draw "LOADING" with animated dots
    static void drawLoading8bit(int frame) {
        M5Cardputer.Display.fillRect(75, 122, 90, 12, NES_BLACK);
        M5Cardputer.Display.setTextColor(NES_WHITE);
        M5Cardputer.Display.setCursor(80, 124);
        M5Cardputer.Display.print("LOADING");
        
        int dots = (frame / 8) % 4;
        for(int d = 0; d < dots; d++) {
            M5Cardputer.Display.print(".");
        }
        
        // Animated loading bar (NES style)
        int barX = 80;
        int barY = 118;
        int barW = 80;
        int progress = (frame * barW) / 60;
        if(progress > barW) progress = barW;
        
        M5Cardputer.Display.drawRect(barX, barY, barW, 4, NES_WHITE);
        M5Cardputer.Display.fillRect(barX + 1, barY + 1, progress - 2, 2, NES_CYAN);
    }
    
    // Sprite-based drawing functions for double buffering
    static LGFX_Sprite* splashSprite;
    
    static void spx(int x, int y, uint16_t c) {
        splashSprite->fillRect(x * PX, y * PX, PX, PX, c);
    }
    
    static void drawBoy8bitSprite(int x, int y, int frame) {
        int f = frame % 4;
        
        // Hair
        for(int i = 2; i < 7; i++) spx(x+i, y, NES_DKGRAY);
        for(int i = 1; i < 8; i++) spx(x+i, y+1, NES_DKGRAY);
        for(int i = 1; i < 8; i++) spx(x+i, y+2, NES_DKGRAY);
        
        // Face
        for(int i = 1; i < 8; i++) spx(x+i, y+3, NES_SKIN);
        spx(x+1, y+4, NES_SKIN); spx(x+2, y+4, NES_BLACK); spx(x+3, y+4, NES_SKIN);
        spx(x+4, y+4, NES_SKIN); spx(x+5, y+4, NES_BLACK); spx(x+6, y+4, NES_SKIN);
        spx(x+7, y+4, NES_SKIN);
        for(int i = 1; i < 8; i++) spx(x+i, y+5, NES_SKIN);
        spx(x+1, y+6, NES_SKIN); spx(x+2, y+6, NES_SKIN); spx(x+3, y+6, NES_BLACK);
        spx(x+4, y+6, NES_BLACK); spx(x+5, y+6, NES_SKIN); spx(x+6, y+6, NES_SKIN);
        spx(x+7, y+6, NES_SKIN);
        
        // Headphones
        spx(x, y+3, NES_RED); spx(x, y+4, NES_RED); spx(x, y+5, NES_RED);
        spx(x+8, y+3, NES_RED); spx(x+8, y+4, NES_RED); spx(x+8, y+5, NES_RED);
        for(int i = 1; i < 8; i++) spx(x+i, y-1, NES_RED);
        
        // Shirt
        for(int j = 7; j <= 11; j++) {
            for(int i = 2; i < 7; i++) spx(x+i, y+j, NES_BLUE);
        }
        
        // Arms
        int armOffset = (f < 2) ? 0 : 1;
        spx(x+1, y+7+armOffset, NES_SKIN); spx(x+1, y+8+armOffset, NES_SKIN);
        spx(x+1, y+9+armOffset, NES_SKIN);
        spx(x+7, y+7+(1-armOffset), NES_SKIN); spx(x+7, y+8+(1-armOffset), NES_SKIN);
        spx(x+7, y+9+(1-armOffset), NES_SKIN);
        
        // Pants
        for(int j = 12; j <= 14; j++) {
            for(int i = 2; i < 7; i++) spx(x+i, y+j, NES_DKGRAY);
        }
        
        // Legs
        int legL = 0, legR = 0;
        switch(f) {
            case 0: legL = 0; legR = 2; break;
            case 1: legL = 1; legR = 1; break;
            case 2: legL = 2; legR = 0; break;
            case 3: legL = 1; legR = 1; break;
        }
        spx(x+2+legL, y+15, NES_DKGRAY); spx(x+2+legL, y+16, NES_DKGRAY);
        spx(x+2+legL, y+17, NES_BROWN); spx(x+3+legL, y+17, NES_BROWN);
        spx(x+5-legR, y+15, NES_DKGRAY); spx(x+5-legR, y+16, NES_DKGRAY);
        spx(x+5-legR, y+17, NES_BROWN); spx(x+6-legR, y+17, NES_BROWN);
    }
    
    static void drawNote8bitSprite(int x, int y, uint16_t color) {
        spx(x, y+2, color); spx(x+1, y+2, color);
        spx(x, y+3, color); spx(x+1, y+3, color);
        spx(x+1, y, color); spx(x+1, y+1, color);
        spx(x+2, y, color); spx(x+2, y+1, color);
    }
    
    static void drawDoubleNote8bitSprite(int x, int y, uint16_t color) {
        spx(x, y+2, color); spx(x+1, y+2, color);
        spx(x+3, y+2, color); spx(x+4, y+2, color);
        spx(x+1, y, color); spx(x+1, y+1, color);
        spx(x+4, y, color); spx(x+4, y+1, color);
        spx(x+1, y, color); spx(x+2, y, color); spx(x+3, y, color); spx(x+4, y, color);
    }
    
    static void drawHouse8bitSprite(int hx, int hy) {
        for(int i = 0; i < 8; i++) {
            for(int j = 8-i; j <= 8+i; j++) {
                spx(hx+j, hy+i, NES_RED);
            }
        }
        for(int j = 8; j < 16; j++) {
            for(int i = 1; i < 16; i++) {
                spx(hx+i, hy+j, NES_GRAY);
            }
        }
        for(int j = 10; j < 16; j++) {
            spx(hx+7, hy+j, NES_BROWN); spx(hx+8, hy+j, NES_BROWN); spx(hx+9, hy+j, NES_BROWN);
        }
        spx(hx+9, hy+13, NES_YELLOW);
        spx(hx+3, hy+10, NES_CYAN); spx(hx+4, hy+10, NES_CYAN);
        spx(hx+3, hy+11, NES_CYAN); spx(hx+4, hy+11, NES_CYAN);
        spx(hx+12, hy+10, NES_CYAN); spx(hx+13, hy+10, NES_CYAN);
        spx(hx+12, hy+11, NES_CYAN); spx(hx+13, hy+11, NES_CYAN);
        spx(hx+12, hy+2, NES_BROWN); spx(hx+13, hy+2, NES_BROWN);
        spx(hx+12, hy+3, NES_BROWN); spx(hx+13, hy+3, NES_BROWN);
        spx(hx+12, hy+4, NES_BROWN); spx(hx+13, hy+4, NES_BROWN);
    }
    
    static void drawTree8bitSprite(int tx, int ty) {
        spx(tx+2, ty+6, NES_BROWN); spx(tx+3, ty+6, NES_BROWN);
        spx(tx+2, ty+7, NES_BROWN); spx(tx+3, ty+7, NES_BROWN);
        spx(tx+2, ty+8, NES_BROWN); spx(tx+3, ty+8, NES_BROWN);
        for(int i = 0; i < 6; i++) spx(tx+i, ty+5, NES_GREEN);
        for(int i = 0; i < 6; i++) spx(tx+i, ty+4, NES_GREEN);
        for(int i = 1; i < 5; i++) spx(tx+i, ty+3, NES_GREEN);
        for(int i = 1; i < 5; i++) spx(tx+i, ty+2, NES_DKGREEN);
        spx(tx+2, ty+1, NES_DKGREEN); spx(tx+3, ty+1, NES_DKGREEN);
        spx(tx+2, ty, NES_GREEN); spx(tx+3, ty, NES_GREEN);
    }
    
    static void drawCloud8bitSprite(int cx, int cy) {
        for(int i = 1; i < 6; i++) spx(cx+i, cy, NES_WHITE);
        for(int i = 0; i < 7; i++) spx(cx+i, cy+1, NES_WHITE);
        for(int i = 1; i < 6; i++) spx(cx+i, cy+2, NES_WHITE);
    }
    
    static void drawSun8bitSprite(int sx, int sy, int frame) {
        for(int j = 0; j < 4; j++) {
            for(int i = 0; i < 4; i++) {
                spx(sx+i, sy+j, NES_YELLOW);
            }
        }
        int rayPhase = (frame / 5) % 2;
        if(rayPhase == 0) {
            spx(sx+1, sy-1, NES_YELLOW); spx(sx+2, sy-1, NES_YELLOW);
            spx(sx-1, sy+1, NES_YELLOW); spx(sx+4, sy+1, NES_YELLOW);
            spx(sx-1, sy+2, NES_YELLOW); spx(sx+4, sy+2, NES_YELLOW);
            spx(sx+1, sy+4, NES_YELLOW); spx(sx+2, sy+4, NES_YELLOW);
        } else {
            spx(sx-1, sy-1, NES_ORANGE); spx(sx+4, sy-1, NES_ORANGE);
            spx(sx+1, sy-1, NES_YELLOW); spx(sx+2, sy-1, NES_YELLOW);
            spx(sx-1, sy+4, NES_ORANGE); spx(sx+4, sy+4, NES_ORANGE);
            spx(sx+1, sy+4, NES_YELLOW); spx(sx+2, sy+4, NES_YELLOW);
        }
    }
    
    static void drawGround8bitSprite() {
        int groundY = 52;
        for(int y = groundY; y < groundY + 5; y++) {
            for(int x = 0; x < 120; x++) {
                spx(x, y, NES_GREEN);
            }
        }
        for(int x = 0; x < 120; x += 5) {
            spx(x, groundY, NES_DKGREEN);
            spx(x+2, groundY, NES_DKGREEN);
        }
        for(int y = groundY + 5; y < 68; y++) {
            for(int x = 0; x < 120; x++) {
                spx(x, y, NES_DKGRAY);
            }
        }
        for(int x = 0; x < 120; x += 10) {
            for(int i = 0; i < 5; i++) {
                spx(x+i, groundY + 8, NES_YELLOW);
            }
        }
    }
    
    static void show() {
        // Create full-screen sprite for double buffering (no flicker)
        LGFX_Sprite frameBuffer(&M5Cardputer.Display);
        frameBuffer.setColorDepth(16);
        frameBuffer.createSprite(240, 135);
        splashSprite = &frameBuffer;
        
        // Animation variables
        int boyX = 35;  // Fixed position - boy does not move
        int boyY = 32;
        
        // Note tracking
        struct Note8 {
            int x, y;
            int type;
            uint16_t color;
            int floatY;
            bool active;
        };
        Note8 notes[3] = {{0,0,0,0,0,false},{0,0,0,0,0,false},{0,0,0,0,0,false}};
        uint16_t noteColors[] = {NES_CYAN, NES_MAGENTA, NES_YELLOW, NES_RED};
        
        // Main animation loop - 60 frames
        for(int frame = 0; frame < 60; frame++) {
            // Clear entire frame buffer with sky color
            frameBuffer.fillScreen(NES_SKY);
            
            // Draw all static elements to buffer
            drawGround8bitSprite();
            drawSun8bitSprite(100, 5, frame);
            drawCloud8bitSprite(10, 8);
            drawCloud8bitSprite(60, 12);
            drawTree8bitSprite(5, 40);
            drawTree8bitSprite(70, 42);
            drawHouse8bitSprite(85, 30);
            
            // Draw the walking boy (stationary position, only animates walk cycle)
            drawBoy8bitSprite(boyX, boyY, frame);
            
            // Spawn new note every 12 frames
            if(frame % 12 == 0) {
                for(int n = 0; n < 3; n++) {
                    if(!notes[n].active) {
                        notes[n].x = boyX + 8;
                        notes[n].y = boyY - 2;
                        notes[n].type = random(0, 2);
                        notes[n].color = noteColors[random(0, 4)];
                        notes[n].floatY = 0;
                        notes[n].active = true;
                        break;
                    }
                }
            }
            
            // Update and draw notes
            for(int n = 0; n < 3; n++) {
                if(notes[n].active) {
                    int noteDrawX = notes[n].x + ((notes[n].floatY / 3) % 3) - 1;
                    int noteDrawY = notes[n].y - notes[n].floatY;
                    
                    if(noteDrawY > 5 && noteDrawY < 50) {
                        if(notes[n].type == 0) {
                            drawNote8bitSprite(noteDrawX, noteDrawY, notes[n].color);
                        } else {
                            drawDoubleNote8bitSprite(noteDrawX, noteDrawY, notes[n].color);
                        }
                    }
                    
                    notes[n].floatY += 1;
                    if(notes[n].floatY > 25) {
                        notes[n].active = false;
                    }
                }
            }
            
            // Title bar
            frameBuffer.fillRect(0, 0, 240, 22, NES_BLACK);
            uint16_t colors[] = {NES_CYAN, NES_WHITE, NES_YELLOW, NES_MAGENTA};
            uint16_t titleColor = colors[(frame / 10) % 4];
            frameBuffer.setFont(&fonts::Font0);
            frameBuffer.setTextColor(titleColor);
            frameBuffer.setCursor(45, 7);
            frameBuffer.print("SAM MUSIC PLAYER");
            spx(10, 3, NES_CYAN); spx(11, 4, NES_MAGENTA); spx(10, 5, NES_YELLOW);
            spx(108, 3, NES_CYAN); spx(109, 4, NES_MAGENTA); spx(108, 5, NES_YELLOW);
            
            // Loading bar
            frameBuffer.fillRect(75, 118, 90, 16, NES_BLACK);
            int barW = 80;
            int progress = (frame * barW) / 60;
            frameBuffer.drawRect(80, 118, barW, 4, NES_WHITE);
            if(progress > 2) frameBuffer.fillRect(81, 119, progress - 2, 2, NES_CYAN);
            
            frameBuffer.setTextColor(NES_WHITE);
            frameBuffer.setCursor(80, 124);
            frameBuffer.print("LOADING");
            int dots = (frame / 8) % 4;
            for(int d = 0; d < dots; d++) frameBuffer.print(".");
            
            // Press any key hint (blinking)
            if((frame / 15) % 2 == 0) {
                frameBuffer.setTextColor(NES_DKGRAY);
                frameBuffer.setCursor(50, 108);
                frameBuffer.print("PRESS ANY KEY...");
            }
            
            // Push entire frame to display at once (no flicker!)
            frameBuffer.pushSprite(0, 0);
            
            delay(50);
            
            // Check for key press
            M5Cardputer.update();
            if(M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed()) {
                frameBuffer.deleteSprite();
                if(!BootMenu::show()) {
                    return;
                }
                // Recreate sprite and continue
                frameBuffer.createSprite(240, 135);
                splashSprite = &frameBuffer;
            }
        }
        
        // Delete sprite buffer
        frameBuffer.deleteSprite();
        
        // End animation - flash effect (NES style)
        for(int i = 0; i < 3; i++) {
            M5Cardputer.Display.fillScreen(NES_WHITE);
            delay(50);
            M5Cardputer.Display.fillScreen(NES_BLACK);
            delay(50);
        }
        
        M5Cardputer.Display.setBrightness(userSettings.brightness);
    }
};

// Static member definition
LGFX_Sprite* SplashScreen::splashSprite = nullptr;

// ==========================================
// MAIN SETUP & LOOP
// ==========================================
void setup() {
    auto cfg = M5.config(); cfg.external_speaker.hat_spk = true; M5Cardputer.begin(cfg);
    
    if (nvs_flash_init() != ESP_OK) { nvs_flash_erase(); nvs_flash_init(); }
    ConfigManager::load(); applyCpuFrequency();
    applyTheme(userSettings.themeIndex);
    auto spk_cfg = M5Cardputer.Speaker.config(); spk_cfg.sample_rate = sampleRateValues[userSettings.spkRateIndex];
    spk_cfg.task_pinned_core = APP_CPU_NUM; spk_cfg.dma_buf_count = 8; spk_cfg.dma_buf_len = 256; spk_cfg.task_priority = 3;    
    M5Cardputer.Speaker.config(spk_cfg);

    M5Cardputer.Display.setRotation(1); visSprite.setColorDepth(16); 
    visSprite.createSprite(M5Cardputer.Display.width() - PLAYLIST_WIDTH - 2, M5Cardputer.Display.height() - HEADER_HEIGHT - 55 - BOTTOM_BAR_HEIGHT);

    SPI.begin(SD_SPI_SCK_PIN, SD_SPI_MISO_PIN, SD_SPI_MOSI_PIN, SD_SPI_CS_PIN);
    if (!SD.begin(SD_SPI_CS_PIN, SPI, 25000000)) while(1);

    WebServerManager::setup();
    M5Cardputer.Display.setBrightness(userSettings.brightness);

    // Show splash screen animation
    if (userSettings.showSplash) SplashScreen::show();
    
    out = new AudioOutputM5Speaker(&M5Cardputer.Speaker, 0);
    out->begin();
    M5Cardputer.Speaker.setVolume(userSettings.volume);

    if (audioApp.loadPlaylist()) {
        if (userSettings.resumePlay && userSettings.lastIndex < (int)audioApp.songOffsets.size())
            audioApp.play(userSettings.lastIndex, userSettings.lastPos);
        else audioApp.play(0);
    } else {
        // Cache missing — fall back to full scan
        audioApp.performFullScan();
        if(audioApp.songOffsets.size() > 0) audioApp.play(0);
    }

    // Load album index for sidebar
    loadAlbumIndex();
    
    UIManager::drawBaseUI(); lastInputTime = millis();
}

void loop() {
    M5Cardputer.update(); server.handleClient(); audioApp.loopTasks();
    static int lastPlayingIndex = -1;
    if (lastPlayingIndex != audioApp.currentIndex && currentState == UI_PLAYER && !isScreenOff) {
        lastPlayingIndex = audioApp.currentIndex;
        UIManager::drawHeader();
        UIManager::drawPlaylist();
        UIManager::drawNowPlaying();
        UIManager::drawBottomBar();
    }
    if (currentState == UI_PLAYER && !isScreenOff) {
        static unsigned long lastVis = 0; if (millis() - lastVis > 30) { UIManager::drawVisualizer(); lastVis = millis(); }
        static unsigned long lastBat = 0; if (millis() - lastBat > 10000) { UIManager::drawBattery(); lastBat = millis(); }
        static unsigned long lastProg = 0; if (millis() - lastProg > 1000) { UIManager::drawNowPlaying(); lastProg = millis(); }
    }

    if (userSettings.timeoutIndex > 0 && !isScreenOff) {
        if (millis() - lastInputTime > timeoutValues[userSettings.timeoutIndex]) { M5Cardputer.Display.setBrightness(0); M5Cardputer.Display.sleep(); isScreenOff = true; }
    }

    if(M5Cardputer.BtnA.wasDecideClickCount()){
        int clicks = M5Cardputer.BtnA.getClickCount();
        if (clicks == 1) { audioApp.togglePause(); ConfigManager::save(audioApp.id3 ? audioApp.id3->getPos() : 0, audioApp.currentIndex); }
        else if (clicks == 2) audioApp.next();
        else if (clicks == 3) audioApp.prev();
        if (currentState == UI_PLAYER && !isScreenOff) UIManager::drawNowPlaying();
    }

    if (M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed()) {
        if (isScreenOff) { M5Cardputer.Display.wakeup(); M5Cardputer.Display.setBrightness(userSettings.brightness); isScreenOff = false; lastInputTime = millis(); UIManager::drawBaseUI(); return; }
        lastInputTime = millis();

        switch (currentState) {
            case UI_PLAYER:
                if (M5Cardputer.Keyboard.isKeyPressed('`')) { currentState = UI_SETTINGS; UIManager::drawSettings(); }
                else if (M5Cardputer.Keyboard.isKeyPressed('i')) { currentState = UI_HELP; UIManager::drawHelp(); }
                else if (M5Cardputer.Keyboard.isKeyPressed(';')) {
                    // Scroll albums up
                    if (g_albumOffsets.size() > 0) {
                        g_albumCursor = (g_albumCursor - 1 + g_albumOffsets.size()) % g_albumOffsets.size();
                        // Adjust scroll
                        if (g_albumCursor < g_albumScrollOffset) g_albumScrollOffset = g_albumCursor;
                        if (g_albumCursor == (int)g_albumOffsets.size() - 1)
                            g_albumScrollOffset = max(0, (int)g_albumOffsets.size() - MAX_VISIBLE_ROWS);
                        UIManager::drawPlaylist();
                    }
                }
                else if (M5Cardputer.Keyboard.isKeyPressed('.')) {
                    // Scroll albums down
                    if (g_albumOffsets.size() > 0) {
                        g_albumCursor = (g_albumCursor + 1) % g_albumOffsets.size();
                        if (g_albumCursor >= g_albumScrollOffset + MAX_VISIBLE_ROWS) g_albumScrollOffset++;
                        if (g_albumCursor == 0) g_albumScrollOffset = 0;
                        UIManager::drawPlaylist();
                    }
                }
                else if (M5Cardputer.Keyboard.isKeyPressed(KEY_ENTER)) {
                    // Open selected album
                    if (g_albumOffsets.size() > 0) {
                        loadAlbumSongs(g_albumCursor);
                        currentState = UI_ALBUM_SONGS;
                        UIManager::drawAlbumSongs();
                    }
                }
                else if (M5Cardputer.Keyboard.isKeyPressed('n')) { audioApp.next(); }
                else if (M5Cardputer.Keyboard.isKeyPressed('b')) { audioApp.prev(); }
                else if (M5Cardputer.Keyboard.isKeyPressed('s')) {
                    g_searchQuery = ""; g_searchResults.clear(); g_searchCursor = 0; g_searchScrollOffset = 0;
                    currentState = UI_SEARCH; UIManager::drawSearch();
                }
                else if (M5Cardputer.Keyboard.isKeyPressed('f')) { audioApp.isShuffle = !audioApp.isShuffle; UIManager::drawNowPlaying(); }
                else if (M5Cardputer.Keyboard.isKeyPressed('l')) { audioApp.loopMode = (LoopState)((audioApp.loopMode + 1) % 3); UIManager::drawNowPlaying(); }
                else if (M5Cardputer.Keyboard.isKeyPressed('v')) {
                    userSettings.visMode = (userSettings.visMode + 1) % NUM_VIS_MODES;
                    UIManager::drawBaseUI();
                }
                else if (M5Cardputer.Keyboard.isKeyPressed('/')) { audioApp.seek(userSettings.seek); UIManager::drawNowPlaying(); }
                else if (M5Cardputer.Keyboard.isKeyPressed(',')) { audioApp.seek(-userSettings.seek); UIManager::drawNowPlaying(); }
                else if (M5Cardputer.Keyboard.isKeyPressed(']')) { M5Cardputer.Speaker.setVolume(min(255, M5Cardputer.Speaker.getVolume() + 10)); userSettings.volume = M5Cardputer.Speaker.getVolume(); ConfigManager::save(); UIManager::drawNowPlaying(); }
                else if (M5Cardputer.Keyboard.isKeyPressed('[')) { M5Cardputer.Speaker.setVolume(max(0, M5Cardputer.Speaker.getVolume() - 10)); userSettings.volume = M5Cardputer.Speaker.getVolume(); ConfigManager::save(); UIManager::drawNowPlaying(); }
                break;

            case UI_SETTINGS:
                if (M5Cardputer.Keyboard.isKeyPressed('`')) { ConfigManager::save(); currentState = UI_PLAYER; UIManager::drawBaseUI(); }
                else if (M5Cardputer.Keyboard.isKeyPressed(';')) { 
                    UIManager::settingsCursor = (UIManager::settingsCursor - 1 + numSettings) % numSettings;
                    if (UIManager::settingsCursor < UIManager::menuScrollOffset) UIManager::menuScrollOffset = UIManager::settingsCursor;
                    else if (UIManager::settingsCursor == (numSettings - 1)) UIManager::menuScrollOffset = (numSettings-4);
                    UIManager::drawSettings(); 
                }
                else if (M5Cardputer.Keyboard.isKeyPressed('.')) { 
                    UIManager::settingsCursor = (UIManager::settingsCursor + 1) % numSettings; 
                    if (UIManager::settingsCursor >= UIManager::menuScrollOffset + 4) UIManager::menuScrollOffset++;
                    else if (UIManager::settingsCursor == 0) UIManager::menuScrollOffset = 0;
                    UIManager::drawSettings(); 
                }
                else if (M5Cardputer.Keyboard.isKeyPressed('/') || M5Cardputer.Keyboard.isKeyPressed(',')) {
                    bool right = M5Cardputer.Keyboard.isKeyPressed('/');
                    switch (UIManager::settingsCursor) {
                        case 0: userSettings.brightness = constrain(userSettings.brightness + (right?25:-25), 5, 255); M5Cardputer.Display.setBrightness(userSettings.brightness); break;
                        case 1: userSettings.timeoutIndex = (userSettings.timeoutIndex + (right?1:-1) + 5) % 5; break;
                        case 2: userSettings.resumePlay = !userSettings.resumePlay; break;
                        case 3: userSettings.spkRateIndex = (userSettings.spkRateIndex + (right?1:-1) + 5) % 5; 
                                { auto c = M5Cardputer.Speaker.config(); c.sample_rate = sampleRateValues[userSettings.spkRateIndex]; M5Cardputer.Speaker.config(c); } break;
                        case 4: userSettings.seek = constrain(userSettings.seek + (right?5:-5), 5, 60); break;
                        case 5: userSettings.wifiEnabled = !userSettings.wifiEnabled; break;
                        case 6: userSettings.isAPMode = !userSettings.isAPMode; break;
                        case 7: userSettings.powerSaverMode = (userSettings.powerSaverMode + (right?1:-1) + 3) % 3; applyCpuFrequency(); break;
                        case 8:
                            userSettings.themeIndex = (userSettings.themeIndex + (right?1:-1) + NUM_THEMES) % NUM_THEMES;
                            applyTheme(userSettings.themeIndex);
                            UIManager::drawBaseUI();
                            break;
                        case 9:
                            userSettings.visMode = (userSettings.visMode + (right?1:-1) + NUM_VIS_MODES) % NUM_VIS_MODES;
                            UIManager::drawBaseUI();
                            break;
                        case 15: userSettings.showSplash = !userSettings.showSplash; break;
                    }
                    UIManager::drawSettings();
                }
                else if (M5Cardputer.Keyboard.isKeyPressed(KEY_ENTER)) {
                    switch(UIManager::settingsCursor) {
                        case 10: // Wi-Fi Setup
                            currentState = UI_WIFI_SCAN; WiFi.mode(WIFI_STA); WiFi.disconnect(); delay(100);
                            UIManager::wifiNetworkCount = WiFi.scanNetworks(); UIManager::wifiCursor = 0; UIManager::wifiScrollOffset = 0;
                            UIManager::drawWifiScanner(); break;
                        case 11: // AP Setup
                            currentState = UI_TEXT_INPUT; textInputTarget = 1; enteredText = userSettings.apSSID;
                            UIManager::drawTextInput(); break;
                        case 12: audioApp.performFullScan(); loadAlbumIndex(); currentState = UI_PLAYER; UIManager::drawBaseUI(); break;
                        case 13: ConfigManager::exportToSD(); currentState = UI_PLAYER; UIManager::drawBaseUI(); break;
                        case 14: ConfigManager::importFromSD(); currentState = UI_PLAYER; UIManager::drawBaseUI(); break;
                    }
                }
                break;

            case UI_HELP:
                if (M5Cardputer.Keyboard.isKeyPressed('i') || M5Cardputer.Keyboard.isKeyPressed('`')) { 
                    currentState = UI_PLAYER; UIManager::drawBaseUI(); 
                }
                else if (M5Cardputer.Keyboard.isKeyPressed(';')) {
                    UIManager::helpScrollOffset--;
                    if (UIManager::helpScrollOffset < 0) UIManager::helpScrollOffset = 0;
                    UIManager::drawHelp();
                }
                else if (M5Cardputer.Keyboard.isKeyPressed('.')) {
                    UIManager::helpScrollOffset++;
                    if (UIManager::helpScrollOffset > numHelpLines - 7) UIManager::helpScrollOffset = numHelpLines - 7;
                    UIManager::drawHelp();
                }
                break;

            case UI_WIFI_SCAN:
                if (M5Cardputer.Keyboard.isKeyPressed('`')) { currentState = UI_SETTINGS; UIManager::drawSettings(); }
                else if (M5Cardputer.Keyboard.isKeyPressed(';')) {
                    UIManager::wifiCursor--; if (UIManager::wifiCursor < 0) UIManager::wifiCursor = UIManager::wifiNetworkCount - 1;
                    if (UIManager::wifiCursor < UIManager::wifiScrollOffset) UIManager::wifiScrollOffset = UIManager::wifiCursor;
                    if (UIManager::wifiCursor == UIManager::wifiNetworkCount - 1) UIManager::wifiScrollOffset = max(0, UIManager::wifiNetworkCount - 6);
                    UIManager::drawWifiScanner();
                }
                else if (M5Cardputer.Keyboard.isKeyPressed('.')) {
                    UIManager::wifiCursor++; if (UIManager::wifiCursor >= UIManager::wifiNetworkCount) { UIManager::wifiCursor = 0; UIManager::wifiScrollOffset = 0; }
                    if (UIManager::wifiCursor >= UIManager::wifiScrollOffset + 6) UIManager::wifiScrollOffset++;
                    UIManager::drawWifiScanner();
                }
                else if (M5Cardputer.Keyboard.isKeyPressed(KEY_ENTER)) {
                    userSettings.wifiSSID = WiFi.SSID(UIManager::wifiCursor);
                    enteredText = ""; currentState = UI_TEXT_INPUT; textInputTarget = 0;
                    UIManager::drawTextInput();
                }
                break;

            case UI_TEXT_INPUT:
                {
                auto status = M5Cardputer.Keyboard.keysState(); bool redraw = false;
                if (status.del && enteredText.length() > 0) { enteredText.remove(enteredText.length() - 1); redraw = true; } 
                else if (status.enter) {
                    if (textInputTarget == 0) { userSettings.wifiPass = enteredText; userSettings.wifiEnabled = true; ConfigManager::save(); ESP.restart(); } 
                    else if (textInputTarget == 1) { userSettings.apSSID = enteredText; textInputTarget = 2; enteredText = userSettings.apPass; redraw = true; } 
                    else if (textInputTarget == 2) {
                        if (enteredText.length() < 8) { M5Cardputer.Display.setCursor(15, HEADER_HEIGHT + 65); M5Cardputer.Display.setTextColor(TFT_RED); M5Cardputer.Display.print("Must be 8+ chars!"); delay(1000); redraw = true; } 
                        else { userSettings.apPass = enteredText; userSettings.isAPMode = true; userSettings.wifiEnabled = true; ConfigManager::save(); ESP.restart(); }
                    }
                } 
                else if (M5Cardputer.Keyboard.isKeyPressed('`')) { currentState = UI_SETTINGS; UIManager::drawSettings(); } 
                else { for (char c : status.word) { enteredText += c; redraw = true; } }
                if (redraw) UIManager::drawTextInput();
                }
                break;

            // --------------------------------------------------
            // SEARCH STATE
            // --------------------------------------------------
            case UI_SEARCH:
                {
                auto status = M5Cardputer.Keyboard.keysState(); bool redraw = false;
                if (M5Cardputer.Keyboard.isKeyPressed('`')) {
                    currentState = UI_PLAYER; UIManager::drawBaseUI();
                } else if (status.del && g_searchQuery.length() > 0) {
                    g_searchQuery.remove(g_searchQuery.length() - 1);
                    UIManager::rebuildSearchResults(); redraw = true;
                } else if (M5Cardputer.Keyboard.isKeyPressed(';')) {
                    if (g_searchResults.size() > 0) {
                        g_searchCursor--;
                        if (g_searchCursor < 0) g_searchCursor = (int)g_searchResults.size() - 1;
                        if (g_searchCursor < g_searchScrollOffset) g_searchScrollOffset = g_searchCursor;
                        if (g_searchCursor == (int)g_searchResults.size() - 1)
                            g_searchScrollOffset = max(0, (int)g_searchResults.size() - MAX_VISIBLE_ROWS);
                    }
                    redraw = true;
                } else if (M5Cardputer.Keyboard.isKeyPressed('.')) {
                    if (g_searchResults.size() > 0) {
                        g_searchCursor++;
                        if (g_searchCursor >= (int)g_searchResults.size()) { g_searchCursor = 0; g_searchScrollOffset = 0; }
                        if (g_searchCursor >= g_searchScrollOffset + MAX_VISIBLE_ROWS) g_searchScrollOffset++;
                    }
                    redraw = true;
                } else if (status.enter) {
                    if (g_searchResults.size() > 0) {
                        int songIdx = g_searchResults[g_searchCursor];
                        g_albumPlaybackActive = false; // Exit album-scoped playback
                        audioApp.play(songIdx);
                        currentState = UI_PLAYER;
                        UIManager::drawBaseUI();
                    }
                } else {
                    for (char c : status.word) {
                        g_searchQuery += c; redraw = true;
                    }
                    if (redraw) UIManager::rebuildSearchResults();
                }
                if (redraw && currentState == UI_SEARCH) UIManager::drawSearch();
                }
                break;

            // --------------------------------------------------
            // ALBUM SONGS STATE
            // --------------------------------------------------
            case UI_ALBUM_SONGS:
                if (M5Cardputer.Keyboard.isKeyPressed('`')) {
                    // Back to player (album list)
                    currentState = UI_PLAYER;
                    UIManager::drawBaseUI();
                }
                else if (M5Cardputer.Keyboard.isKeyPressed(';')) {
                    if (g_albumSongOffsets.size() > 0) {
                        g_albumSongCursor--;
                        if (g_albumSongCursor < 0) g_albumSongCursor = (int)g_albumSongOffsets.size() - 1;
                        if (g_albumSongCursor < g_albumSongScrollOffset) g_albumSongScrollOffset = g_albumSongCursor;
                        if (g_albumSongCursor == (int)g_albumSongOffsets.size() - 1)
                            g_albumSongScrollOffset = max(0, (int)g_albumSongOffsets.size() - MAX_VISIBLE_ROWS);
                    }
                    UIManager::drawAlbumSongsList();
                }
                else if (M5Cardputer.Keyboard.isKeyPressed('.')) {
                    if (g_albumSongOffsets.size() > 0) {
                        g_albumSongCursor++;
                        if (g_albumSongCursor >= (int)g_albumSongOffsets.size()) { g_albumSongCursor = 0; g_albumSongScrollOffset = 0; }
                        if (g_albumSongCursor >= g_albumSongScrollOffset + MAX_VISIBLE_ROWS) g_albumSongScrollOffset++;
                    }
                    UIManager::drawAlbumSongsList();
                }
                else if (M5Cardputer.Keyboard.isKeyPressed(KEY_ENTER)) {
                    if (g_albumSongOffsets.size() > 0) {
                        String songPath = getAlbumSongPath(g_albumSongCursor);
                        int globalIdx = findSongInPlaylist(songPath);
                        if (globalIdx >= 0) {
                            audioApp.play(globalIdx);
                            g_albumPlaybackActive = true;
                            g_albumPlaybackIndex = g_albumSongCursor;
                            ConfigManager::save(audioApp.id3 ? audioApp.id3->getPos() : 0, audioApp.currentIndex);
                        }
                        currentState = UI_PLAYER;
                        UIManager::drawBaseUI();
                    }
                }
                break;
        }
    }
}
