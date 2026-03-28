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
#define SEARCH_INDEX_FILE "/search.idx"

struct TagInfo {
    String album;
    String artist;
    String title;
    uint8_t trackNo;
    uint16_t duration;      // seconds (0 = unknown)
    uint32_t audioOffset;   // byte offset where audio data begins (after ID3/metadata headers)
};

// U8g2 font: 7x13 fixed-width with Cyrillic (Public domain)
// Source: https://github.com/olikraus/u8g2 — Misc-Fixed-Medium-R-Normal
static const uint8_t u8g2_font_7x13_cyrillic[4543] PROGMEM =
"M\0\3\2\3\4\3\5\4\7\15\0\376\11\376\11\0\1p\2\344\4: \5\0\356\7!\7\311\303"
  "\307!\11\42\7\33\333Gb\11#\16\275\306O)I\6\245\62(\245$\1$\14\275\306W\66(\331"
  "\226\14Z\4%\20N\302O\246DI\24\246a)\211\22-\1&\15>\302\217\26\265iQ\242EK"
  "\0'\6\231\333\307\0(\13K\303W\22%Q-\312\2)\14K\303G\26eQ%J\42\0*\14"
  ".\312O\224I\303\244E\11\0+\12\255\312W\30\15R\30\1,\10\234\276\317\242d\0-\6\215\322"
  "\307 .\10\33\277O\262$\0/\13\315\302g-\314\212Y\30\2\60\15N\302\227\26%\241\307$\312"
  "$\0\61\13\315\302W&%aO\203\0\62\16N\302\317\220\204b\32fZ\230\16\3\63\15N\302\307"
  "\65,\316\251\230\14\11\0\64\17N\302g\250%Q%K\262d\30\323\4\65\16N\302\307!\255,Z"
  "\232\212\311\220\0\66\16N\302\327\24\246\225E\23\305dH\0\67\14N\302\307\65\254\206i\230\206\0\70"
  "\17N\302\317\220\204\306dHBc\62$\0\71\15N\302\317\220\204\66eI\213\321\4:\13C\277O"
  "\262\344H\262$\0;\13\304\276W\64\345\330\242d\0<\10\315\302g\326\265\3=\10&\312\307\35\35"
  "\6>\11\315\302G\332\255#\0\77\15N\302\317\220\204b\32Vs\60\2@\20N\302\317\220\204\246!"
  "\211\224dJ\344!\1A\15N\302\227\26%\241q\30Dc\0B\22N\302\307 eI\226d\311\20"
  "eI\226d\303\2C\14N\302\317\220\204j\217\311\220\0D\22N\302\307 eI\226dI\226dI"
  "\226d\303\2E\14N\302\307!\255\16QZ\35\6F\13N\302\307!\255\16Q\332\12G\15N\302\317"
  "\220\204j\323\20j\312\22H\13N\302G\350\70\14\242c\0I\12\315\302\307 \205}\32\4J\13N"
  "\302\337\230\366\222E\23\0K\20N\302G\250%Q%\23\223,\252%a\0L\11N\302G\332\257\303"
  "\0M\13N\302G\70\15\321\342\321\61N\15N\302G(nJ$%\332h\14O\13N\302\317\220\204"
  "~L\206\4P\14N\302\307\240\204\306aI[\1Q\16V\276\317\220\204\276DR\222\14q\0R\16"
  "N\302\307\240\204\306a\251E\265$\14S\17N\302\317\220\204j<\304\251\230\14\11\0T\11\315\302\307"
  " \205\375\4U\12N\302G\350\217\311\220\0V\14N\302GhL\242\336D\23\0W\13N\302G\350"
  "\305e\32\242\61X\16N\302G(&Q\233\26\265\204b\0Y\14\315\302G\246%\245$\13;\1Z"
  "\13N\302\307\65,\212\255\303\0[\11\334\276\307\240\365\267!\134\13\315\302G\230\206i\61\15\3]\11"
  "\334\276\307\220\365\267A^\10\235\332W\226\324\2_\6\16\276\307\1`\6\22\343G\24a\14\66\302\317"
  "\20'\303\250)K\0b\15N\302G\332\262h\242\270)\13\0c\14\66\302\317\220\204j\61\31\22\0"
  "d\13N\302o\313\242\215\66e\11e\15\66\302\317\220\204\303\240\206\311\220\0f\15N\302\327\224%i"
  "\70di#\0g\17F\272\317\242%Y\64\305C\22&C\2h\13N\302G\332\262h\242c\0i"
  "\12\305\302W\216\210\235\6\1j\14\325\272g\216\210\215\232\226,\0k\15N\302G\332\226D\321\26\325"
  "\222\60l\11\315\302\217\330O\203\0m\15\265\302\207\322\242$J\242$Z\0n\12\66\302G\262h\242"
  "c\0o\13\66\302\317\220\204\216\311\220\0p\14F\272G\262h\342\246,i\25q\13F\272\317\242\215"
  "\232\262\244\15r\12\66\302G\62eI\332\10s\15\66\302\317\220\204\211\254\204\311\220\0t\14F\302O"
  "\32\16Y\332\26-\0u\11\66\302G\350MY\2v\13\265\302GfKJI\26\1w\13\265\302G"
  "fI\224D\351\2x\14\66\302G\230D\231\250EI\30y\15F\272G\350\246,\251\230\14\11\0z"
  "\10\66\302\307\261\343\60{\14\335\276\327\22V#\65\13\253\3|\6\311\303\307\3}\15\335\276\307\32\326"
  "R)\15+\23\0~\11\235\332O\244$R\2\0\0\0\10\4d\5\210\377\377\4\0\20V\302W\234"
  "#\303\220\246C\224\246\303\0\4\1\17V\302O\324>\14i:Di:\14\4\2\16^\272\307\240\245"
  "\325-\352\327L\1\4\3\14^\302_\230C\303\220\366\25\4\4\17N\302\327\224\251\351\20\245q\26-"
  "\0\4\5\20N\302\317\220\204j<\304\251\230\14\11\0\4\6\13\315\302\307 \205}\32\4\4\7\15\325"
  "\302G\246\245\203\24v\32\4\4\10\14N\302\337\230\366\222E\23\0\4\11\21N\302\317\226dI\226d"
  "\211\224\364\213\244\0\4\12\20N\302G\324O\203\22%R\42%\222\2\4\13\14N\302\307\240\245\325-"
  "\352\7\4\14\22^\302_\230\205Z\22U\62\61\311\242Z\22\6\4\15\17^\302W\34\205\266)Q\242"
  "M\64\6\4\16\21^\302O\224\351HhL\6\65\25\223!\1\4\17\14]\272G\346o\303\24F\0"
  "\4\20\16N\302\227\26%\241q\30Dc\0\4\21\16N\302\307\240\244\325A\11\215\303\2\4\22\16N"
  "\302\307\240\204\306a\11\215\303\2\4\23\12N\302\307!\355W\0\4\24\20V\276\327\226dI\226D\375"
  "e\30\302\0\4\25\15N\302\307!\255\16QZ\35\6\4\26\22\315\302G\222(\211\222(\225\245\242$"
  "J\242\24\4\27\17N\302\317\220\204iiN\305dH\0\4\30\16N\302Gh\233\22%\332Dc\0"
  "\4\31\20^\302O\224I\241mJ\224h\23\215\1\4\32\21N\302G\250%Q%\23\223,\252%a"
  "\0\4\33\23N\302\317\240dI\226dI\226dI\226db\0\4\34\14N\302G\70\15\321\342\321\61"
  "\4\35\14N\302G\350\70\14\242c\0\4\36\14N\302\317\220\204~L\206\4\4\37\12N\302\307!\364"
  "\307\0\4 \15N\302\307\240\204\306aI[\1\4!\15N\302\317\220\204j\217\311\220\0\4\42\12\315"
  "\302\307 \205\375\4\4#\16N\302G\350\230\14j*&C\2\4$\24\335\276W\266T\224DI\224"
  "DI\224D\251l\21\0\4%\17N\302G(&Q\233\26\265\204b\0\4&\24^\272G\226dI"
  "\226dI\226dI\226d\311\260\26\4'\13N\302G\350\230\14j\7\4(\23\315\302G\222(\211\222"
  "(\211\222(\211\222(\311\60\4)\14^\272G\322\377\377/\303Z\4*\14N\302\307\332\272E=-"
  "\0\4+\20N\302G\350\70DR\42%R\62D\1\4,\15\315\302G\330\70$\231mP\0\4-"
  "\17N\302\317\224\305i\64\244\251\26M\0\4.\23N\302G\244\224\224H\211\206H\211\224H\211,\0"
  "\4/\17N\302\317\60\32\223AK\242J&\6\4\60\15\66\302\317\20'\303\250)K\0\4\61\20N"
  "\302o\62$i\262h\242\61\31\22\0\4\62\15\66\302\307\240\204\303\22\212\303\2\4\63\11\66\302\307!"
  "\355\12\4\64\13>\276\327\324_\206!\14\4\65\16\66\302\317\220\204\303\240\206\311\220\0\4\66\16\265\302"
  "G\222(\225\245\242$J\1\4\67\15\66\302\317\220\204\321,&C\2\4\70\15\66\302G\250M\211\22"
  "mb\0\4\71\20N\302O\224\351H\250M\211\22mb\0\4:\15\66\302G\250%C\224%\241\30"
  "\4;\16\66\302\327\220dI\226dI&\6\4<\13\265\302G\266,\211f\13\4=\13\66\302G("
  "\16\203h\14\4>\14\66\302\317\220\204\216\311\220\0\4\77\11\66\302\307!\364\30\4@\16F\272G\262"
  "h\242\270)K\232\2\4A\15\66\302\317\220\204j\61\31\22\0\4B\12\265\302\307 \205\235\0\4C"
  "\16F\272G\350\246,\251\230\14\11\0\4D\21\325\272W\230-\25%Q\22\245\262\205\21\0\4E\15"
  "\66\302G\230D\231\250EI\30\4F\20F\272G\226dI\226dI\226\14k\1\4G\13\66\302G"
  "hL\6\265\0\4H\17\265\302G\222(\211\222(\211\222\14\3\4I\13F\272G\322\377\227a-\4"
  "J\13\66\302\307\232nQ\323\2\4K\16\66\302G(\16\221\224H\311\20\5\4L\15\265\302G\30\16"
  "I\246\15\12\0\4M\15\66\302\317\220\204\331*&C\2\4N\16\66\302G\244\224\206H\211\224\310\2"
  "\4O\15\66\302\317\60\212\311\240db\0\4P\20N\302W\234CC\22\16\203\32&C\2\4Q\21"
  "N\302O\324\216\14I\70\14j\230\14\11\0\4R\22V\272O\32\16Y:DY\222%Y\232)\0"
  "\4S\14N\302_\230C\303\220v\5\4T\16\66\302\317\220\204C\226\206\311\220\0\4U\16\66\302\317"
  "\220\204\211\254\204\311\220\0\4V\13\305\302W\216\210\235\6\1\4W\14\315\302G\224D\271\330i\20\4"
  "X\15\325\272g\216\210\215\232\226,\0\4Y\17\67\302\317\230\204\311\224DIIZ\0\4Z\16\66\302"
  "G\324\64(Q\42%\222\2\4[\20F\302O\32\16Y:DY\222%Y\0\4\134\20N\302_\230"
  "C\241\226\14Q\226\204b\0\4]\17N\302W\234#\241\66%J\264\211\1\4^\21^\272O\224\351"
  "H\350\246,\251\230\14\11\0\4_\13\305\272G\346m\230\302\10\4`\15N\302O\224\204~QJ\11"
  "\0\4a\14\265\302O\233\222(\211\322\5\4b\22N\302O\70di:DY\222%Y\62$\0\4"
  "c\20F\302O\32\16Y:DY\222%C\2\4d\22N\302G\244\224\224,\311\6\245\226dId"
  "\1\4e\16\66\302G\244\224\6)\311\222\310\2\4f\20\315\302WXK\242$J\222aI\224\2\4"
  "g\15\265\302W\230%Q\222\14K\1\4h\17N\302G\324\227\312\322e\30\224d)\4i\15\66\302"
  "G\324\264t\31\6\245\0\4j\17\315\302\307MKj\331RQ\22\245\0\4k\14\265\302\307-)-"
  "\25\245\0\4l\21N\302\307A\333\224\312\20%\213\222,\311R\4m\16\66\302\307AS*\203\242$"
  "K\1\4n\23n\272O\224\351\320\220\204i\64\247\225!\211\207\4\4o\23^\272O\224\351\320\220\204"
  "\321\234&C\22\17\11\0\4p\22\315\302G\222(\211\222(\211\222(\225-\214\0\4q\20\305\272G"
  "\222(\211\222(\211R\331\302\10\4r\17N\302\317\220\204\306a\20\215\311\220\0\4s\16\66\302\317\220"
  "\204\303 \212\311\220\0\4t\20N\302G\250%Q\226dI\226\204i\15\4u\15\265\302G&%Q"
  "%J\262\10\4v\23^\302G\224Ey\250%Q\226dI\226\204i\6\4w\17\315\302G\324\232I"
  "IT\211\222,\2\4x\21^\272O\230dI\226d\211\277\224\322\60\2\4y\14>\272O\305/\245"
  "\64\214\0\4z\14\335\276W\266d~K\266\10\4{\13\275\302W\266dn\311\2\4|\17^\302_"
  "\230cQ\22\372\242\224\22\0\4}\16\315\302_\35iS\22%Q\272\0\4~\17V\302\317\220#Q"
  "\22\372\242\224\22\0\4\177\16\305\302\317\236\324\224DI\224.\0\4\200\14V\276\327\224\251\235\207\264\2"
  "\4\201\14F\272\317\220\204jy-\1\4\202\16F\302gT\223\42S\244EE\0\4\203\10\33\343\327"
  " \1\4\204\10\234\342WE\3\4\205\11\33\343G\64D\0\4\206\10\33\343\327\20\5\4\210\20o\272"
  "O\232Di\222\363K\232Di\22\4\211\16\355\272O\226-\241\316\232eK(\4\214\20M\302O\66"
  "Da\70U\242$J\26\0\4\215\14\65\302O\66D\341TY\0\4\216\17N\302\307\240\204R\242%"
  "C\242\266\2\4\217\16F\272G\262hR\62%\211\265\12\4\220\13^\302o:\14j\277\2\4\221\13"
  "F\302o:\14jW\0\4\222\14N\302\307!\255\16Q\332\12\4\223\12\265\302\307\61\234\302\20\4\224"
  "\15]\272\307\61\234*\231[I\2\4\225\14\305\272\307q\252dZI\2\4\226\16^\272G\322\377\64"
  "%\375\27\265\0\4\227\14F\272G\322\323\224\364\213Z\4\230\20^\272\317\220\204iiN\305d\10k"
  "\0\4\231\16F\272\317\220\204\321,&CX\3\4\232\21^\272G\250%Q%\23\223,\252%aZ"
  "\4\233\16F\272G\250%C\224%\241\230\26\4\234\21N\302G\250\264HI\266%Y\42%\215\1\4"
  "\235\15\66\302G\250T\206(i\24\3\4\236\21N\302O\66DIT\311\304$\213jI\26\4\237\15"
  "\66\302O\66DI\24mQ-\4\240\20N\302\207\226D\225LL\305$\213j\1\4\241\15\265\302\207"
  "\224\224\264$\252D\1\4\242\24^\272G\226dI\226d\311\240dI\226dI\246\26\4\243\20F\272"
  "G\226d\311\240dI\226dj\1\4\244\24O\302G\66dQ\26e\321 eQ\26eQ\26\1\4"
  "\245\20\67\302G\66d\321 eQ\26e\21\0\4\246\24\335\272\307\224DI\224(\25%Q\22%Q"
  "\212Y\2\4\247\20\305\272\307\224D\211RQ\22\245\230%\0\4\250\20V\276\327\224\251%\245\244D\311"
  "\220\305\3\4\251\17F\272\317\220\204j\244\224\222!\213\7\4\252\17^\272\317\220\204j\217\311\220\305\12"
  "\0\4\253\16F\272\317\220\204j\61\31\262X\1\4\254\14\335\272\307 \205}\24+\0\4\255\13\305\272"
  "\307 \205\215b\5\4\256\15\335\272G\246%\245$\13\373\4\4\257\13\305\272G\226\324\302\236\0\4\260"
  "\20\335\272G\246%\245$\13K\203\24\226\0\4\261\16\305\272G\226\324\302\322 \205\21\0\4\262\22_"
  "\272G\230\204Q-\12\305(\213\212I(\27\4\263\15G\272G\30\25U\61*\312\5\4\264\14^\272"
  "\307\22\365\377iP\13\4\265\13F\272\307\22\365\247A-\4\266\20^\272G\226dI\226d\321\220\266"
  "\252\5\4\267\16F\272G\226dI\26\15i\252\26\4\270\17\315\302GfI\224\312\20%Q\22\26\4"
  "\271\14\265\302Gf\251\14Q\22\6\4\272\14N\302G\332:(\241c\0\4\273\13\66\302G\232\16J"
  "h\14\4\274\20N\302\237\26%\222\22%C\226\226\62\5\4\275\15\66\302\237RJ\6)\215\62\5\4"
  "\276\21^\272\237\26%\222\22%C\226\226\62\61\26\4\277\16F\272\237RJ\6)\215\62\61\26\4\300"
  "\13\315\302\307 \205}\32\4\4\301\23\335\302G\226\314I\242$Je\251(\211\222(\5\4\302\20\315"
  "\302G\226\314I\242T\226\212\222(\5\4\303\23^\272G\250%Q%\23\223,\252%a\32-\0\4"
  "\304\20F\272G\250%C\224%\241\230F\13\0\4\307\16^\272G\350\70\14\242cTS\0\4\310\16"
  "F\272G(\16\203h\214j\12\0\4\313\16^\272G\350\230\14j\243\230&\0\4\314\15F\272Gh"
  "L\6\65\24\323\4\4\320\21V\302G\230\14\71\244EI(\16\203(\6\4\321\20N\302G\230\14\71"
  "\62\304\311\60j\312\22\4\322\20V\302O\324\16iQ\22\212\303 \212\1\4\323\17N\302O\324\216\14"
  "q\62\214\232\262\4\4\324\23N\302O\62$Y\222%YrK\262$K\206\0\4\325\16\66\302\217\22"
  "&\311\60\265$J\2\4\326\20V\302G\230\14\371\60\244\351\20\245\351\60\4\327\22N\302G\230\14\71"
  "\62$\341\60\250a\62$\0\4\330\20N\302\317\220\204i:\14\242\61\31\22\0\4\331\16\66\302\317\220"
  "\204\351\60\210\311\220\0\4\332\21V\302O\324\216\14I\230\16\203(&C\2\4\333\21N\302O\324\216"
  "\14I\230\16\203\230\14\11\0\4\334\23\335\302O)\211\223DI\224\312RQ\22%Q\12\4\335\20\315"
  "\302O)\211\223D\251,\25%Q\12\4\336\21^\302O\324\216\14I\230Fs*&C\2\4\337\20"
  "N\302O\324\216\14I\30\315b\62$\0\4\340\16N\302\307 \26\347\252\230\14\11\0\4\341\15F\272"
  "\307\261\70\247b\62$\0\4\342\17^\302\317\220\207\266)Q\242M\64\6\4\343\16F\302\317\220\207\332"
  "\224(\321&\6\4\344\17^\302O\324\36jS\242D\233h\14\4\345\16N\302O\324\36jS\242D"
  "\233\30\4\346\17V\302O\324\216\14I\350\61\31\22\0\4\347\17N\302O\324\216\14I\350\230\14\11\0"
  "\4\350\17N\302\317\220\204\306a\20\215\311\220\0\4\351\16\66\302\317\220\204\303 \212\311\220\0\4\352\21"
  "^\302O\324\216\14I(\16\203hL\206\4\4\353\21N\302O\324\216\14I\70\14\242\230\14\11\0\4"
  "\354\21^\302O\324\216LY\34\15i\252E\23\0\4\355\21N\302O\324\216\14I\30\15\251\230\14\11"
  "\0\4\356\20^\302\317\220\207\216\311\240\246b\62$\0\4\357\17V\272\317\220\207n\312\222\212\311\220\0"
  "\4\360\20^\302O\324\36\32\223AM\305dH\0\4\361\17^\272O\324\36\272)K*&C\2\4"
  "\362\21^\302W%\312Cc\62\250\251\230\14\11\0\4\363\20^\272W%\312C\67eI\305dH\0"
  "\4\364\15^\302O\324\36\32\223A\355\0\4\365\14N\302O\324\36\32\223A-\4\370\23^\302W\222"
  "%yh\34\42)\221\22)\31\242\0\4\371\21N\302W\222%y(\16\221\224H\311\20\5\0";

static const lgfx::U8g2font tagFont(u8g2_font_7x13_cyrillic);
#define TAG_FONT &tagFont

// Truncate a UTF-8 string to fit within maxPixels at 6px per glyph.
// Works correctly for both Latin (1 byte) and Cyrillic (2 bytes) UTF-8.
String truncateToFit(const String& s, int maxPixels) {
    int charWidth = 7; // 7x13 fixed-width font
    int maxChars = maxPixels / charWidth;
    int count = 0, bytePos = 0;
    while (bytePos < (int)s.length() && count < maxChars) {
        uint8_t c = (uint8_t)s[bytePos];
        if (c < 0x80) bytePos += 1;           // ASCII: 1 byte
        else if ((c & 0xE0) == 0xC0) bytePos += 2; // 2-byte UTF-8
        else if ((c & 0xF0) == 0xE0) bytePos += 3; // 3-byte UTF-8
        else bytePos += 4;                     // 4-byte UTF-8
        count++;
    }
    if (bytePos >= (int)s.length()) return s;  // fits entirely
    // Need truncation — back up one char for "~"
    String result = s.substring(0, bytePos);
    // Remove last char to make room for "~"
    int lastCharStart = bytePos;
    int pos2 = 0, prevPos = 0;
    int c2 = 0;
    while (pos2 < bytePos) {
        prevPos = pos2;
        uint8_t ch = (uint8_t)s[pos2];
        if (ch < 0x80) pos2 += 1;
        else if ((ch & 0xE0) == 0xC0) pos2 += 2;
        else if ((ch & 0xF0) == 0xE0) pos2 += 3;
        else pos2 += 4;
    }
    return s.substring(0, prevPos) + "~";
}

// ==========================================
// KEYBOARD LAYOUT SYSTEM
// ==========================================
// Russian ЙЦУКЕН layout: maps QWERTY key presses to Cyrillic UTF-8
const char* mapRussian(char c) {
    static const char* lower[26] = {
        "\xd1\x84", "\xd0\xb8", "\xd1\x81", "\xd0\xb2", "\xd1\x83", // a-e: ф и с в у
        "\xd0\xb0", "\xd0\xbf", "\xd1\x80", "\xd1\x88", "\xd0\xbe", // f-j: а п р ш о
        "\xd0\xbb", "\xd0\xb4", "\xd1\x8c", "\xd1\x82", "\xd1\x89", // k-o: л д ь т щ
        "\xd0\xb7", "\xd0\xb9", "\xd0\xba", "\xd0\xb5", "\xd0\xbd", // p-t: з й к е н
        "\xd0\xb3", "\xd0\xbc", "\xd1\x86", "\xd1\x87", "\xd1\x85", // u-y: г м ц ч х
        "\xd1\x8f"                                                     // z:   я
    };
    static const char* upper[26] = {
        "\xd0\xa4", "\xd0\x98", "\xd0\xa1", "\xd0\x92", "\xd0\xa3", // A-E: Ф И С В У
        "\xd0\x90", "\xd0\x9f", "\xd0\xa0", "\xd0\xa8", "\xd0\x9e", // F-J: А П Р Ш О
        "\xd0\x9b", "\xd0\x94", "\xd0\xac", "\xd0\xa2", "\xd0\xa9", // K-O: Л Д Ь Т Щ
        "\xd0\x97", "\xd0\x99", "\xd0\x9a", "\xd0\x95", "\xd0\x9d", // P-T: З Й К Е Н
        "\xd0\x93", "\xd0\x9c", "\xd0\xa6", "\xd0\xa7", "\xd0\xa5", // U-Y: Г М Ц Ч Х
        "\xd0\xaf"                                                     // Z:   Я
    };
    if (c >= 'a' && c <= 'z') return lower[c - 'a'];
    if (c >= 'A' && c <= 'Z') return upper[c - 'A'];
    return nullptr;
}

// Layout table — nullptr means pass-through (English)
typedef const char* (*LayoutMapFn)(char);
LayoutMapFn g_layouts[] = { nullptr, mapRussian };
const char* g_layoutNames[] = { "EN", "RU" };
#define NUM_LAYOUTS 2

// Remove last UTF-8 character from a String (handles multi-byte Cyrillic)
void removeLastUtf8Char(String& s) {
    int len = s.length();
    if (len == 0) return;
    int pos = len - 1;
    while (pos > 0 && ((uint8_t)s[pos] & 0xC0) == 0x80) pos--;
    s.remove(pos);
}

// Lowercase a UTF-8 string handling both ASCII and Cyrillic
// Arduino's toLowerCase() only handles ASCII; Cyrillic bytes are untouched
String utf8ToLower(const String& s) {
    String result;
    result.reserve(s.length());
    for (int i = 0; i < (int)s.length(); i++) {
        uint8_t b = (uint8_t)s[i];
        // ASCII uppercase
        if (b >= 'A' && b <= 'Z') { result += (char)(b + 32); continue; }
        // Cyrillic uppercase: 2-byte UTF-8 sequences
        if (b == 0xD0 && i + 1 < (int)s.length()) {
            uint8_t b2 = (uint8_t)s[i + 1];
            // А-П (U+0410-U+041F): \xD0\x90-\xD0\x9F → \xD0\xB0-\xD0\xBF
            if (b2 >= 0x90 && b2 <= 0x9F) {
                result += (char)0xD0; result += (char)(b2 + 0x20); i++; continue;
            }
            // Р-Я (U+0420-U+042F): \xD0\xA0-\xD0\xAF → \xD1\x80-\xD1\x8F
            if (b2 >= 0xA0 && b2 <= 0xAF) {
                result += (char)0xD1; result += (char)(b2 - 0x20); i++; continue;
            }
            // Ё (U+0401): \xD0\x81 → \xD1\x91
            if (b2 == 0x81) {
                result += (char)0xD1; result += (char)0x91; i++; continue;
            }
        }
        result += (char)b;
    }
    return result;
}

#define PLAYLIST_WIDTH 120
#define ROW_HEIGHT 15
#define HEADER_HEIGHT 20
#define BOTTOM_BAR_HEIGHT 18
#define MAX_VISIBLE_ROWS 6  

// --- DYNAMIC COLORS & THEMES ---
uint16_t C_BG_DARK, C_BG_LIGHT, C_HEADER, C_ACCENT, C_PLAYING, C_HIGHLIGHT, C_TEXT_MAIN, C_TEXT_DIM;

const int NUM_THEMES = 4;
const char* themeLabels[] = { "Gunmetal Blue", "Cyberpunk", "Retro Amber", "Hacker Green" };
const int NUM_VIS_MODES = 6;
const char* visModeLabels[] = { "Classic Bars", "Waveform Line", "Circular Spikes", "Art + Info", "Now Playing", "OFF" };

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
enum ShuffleMode { SHUF_OFF, SHUF_ALBUM, SHUF_ARTIST, SHUF_GLOBAL };
enum UIState { UI_PLAYER, UI_SETTINGS, UI_HELP, UI_WIFI_SCAN, UI_TEXT_INPUT, UI_SEARCH, UI_ALBUM_SONGS, UI_TRACK_INFO };

const uint32_t sampleRateValues[] = { 44100, 48000, 88200, 96000, 128000 };
const char* sampleRateLabels[] = { "44.1k", "48k", "88.2k", "96k", "128k" };
const long timeoutValues[] = { 0, 30000, 60000, 120000, 300000 };
const char* timeoutLabels[] = { "Always On", "30 Sec", "1 Min", "2 Min", "5 Min" };
const char* powerModeLabels[] = { "OFF", "BASIC (160)", "ULTRA (80)" };
const char* helpLines[] = {
  "--- MUSIC PLAYER ---",
  "Enter: Open Album  P: Pause",
  "; / . : Scroll Albums/Songs",
  "[ / ] : Volume - / +",
  "N / B : Next / Prev Song",
  "/ / , : Seek +Xs / -Xs",
  "S: Search",
  "F: Shuffle (SA/SR/SG)",
  "L: Loop (1x/LP/1T)",
  "E: Expand  Q: Collapse all",
  "Esc / ` : Settings",
  "V: Visualizer  T: Track Info",
  "I: Close Help",
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
  "Hold Esc on boot: Reset",
  "--- ABOUT ---",
  "Fork of SaM player",
  "made by Oleg Proshkin.",
  "original author",
  "Sanchit Minda.",
  "---",
  "GH: github.com/sanchitminda",
  "Share your suggestions!"
};
const int numHelpLines = 33;
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
    int lastAlbumIdx = -1;      // album index for resume (-1 = no album context)
    int lastAlbumTrack = 0;     // track index within album
    int kbLayoutIndex = 0;      // keyboard layout: 0=EN, 1=RU
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
std::vector<String> g_artistNames;          // unique artist names in order
std::vector<String> g_albumNames;           // album names parallel to g_albumOffsets
std::vector<int> g_albumArtistMap;          // artist index per album
std::vector<bool> g_artistExpanded;         // collapse state per artist

// Sidebar display rows
struct SidebarRow {
    bool isArtist;    // true = artist header, false = album
    int dataIdx;      // artist index (if isArtist) or album index (if !isArtist)
};
std::vector<SidebarRow> g_sidebarRows;
int g_sidebarCursor = 0;
int g_sidebarScrollOffset = 0;

// Album songs state
std::vector<uint32_t> g_albumSongOffsets;   // byte offsets of song lines in current album
int g_albumSongCursor = 0;
int g_albumSongScrollOffset = 0;
String g_trackNumInput = "";               // accumulated digit string for track jump
unsigned long g_trackNumLastInput = 0;     // millis of last digit press
#define TRACK_NUM_TIMEOUT 1500             // ms before digit input resets

// Apply entered track number to cursor (1-indexed for user, 0-indexed internally)
void applyTrackNumInput() {
    if (g_trackNumInput.length() == 0 || g_albumSongOffsets.size() == 0) return;
    int trackNum = g_trackNumInput.toInt();
    if (trackNum < 1) trackNum = 1;
    int idx = trackNum - 1; // convert to 0-indexed
    if (idx >= (int)g_albumSongOffsets.size()) idx = (int)g_albumSongOffsets.size() - 1;
    g_albumSongCursor = idx;
    // Adjust scroll to keep cursor visible
    if (g_albumSongCursor < g_albumSongScrollOffset) g_albumSongScrollOffset = g_albumSongCursor;
    if (g_albumSongCursor >= g_albumSongScrollOffset + MAX_VISIBLE_ROWS)
        g_albumSongScrollOffset = g_albumSongCursor - MAX_VISIBLE_ROWS + 1;
}
String g_currentAlbumName = "";
String g_currentAlbumArtist = "";
int g_currentAlbumIdx = -1;                 // album index currently loaded in g_albumSongOffsets
bool g_albumPlaybackActive = false;         // true when playing within an album
int g_albumPlaybackIndex = 0;               // index into g_albumSongOffsets for current track

// Shuffle queue — shared by all shuffle modes (album/artist/global)
// Contents depend on mode:
//   SHUF_ALBUM: indices into g_albumSongOffsets (0..albumSize-1)
//   SHUF_ARTIST/SHUF_GLOBAL: global songOffsets indices
std::vector<int> g_shuffleQueue;
int g_shufflePos = 0;

// Fisher-Yates shuffle a vector in-place, placing startVal at position 0
void fisherYatesShuffle(std::vector<int>& v, int startVal) {
    int n = v.size();
    for (int i = n - 1; i > 0; i--) {
        int j = random(0, i + 1);
        int tmp = v[i]; v[i] = v[j]; v[j] = tmp;
    }
    if (startVal >= 0) {
        for (int i = 0; i < n; i++) {
            if (v[i] == startVal) { int tmp = v[0]; v[0] = v[i]; v[i] = tmp; break; }
        }
    }
}

// Forward declaration for findSongInPlaylist (defined after AudioEngine)
int findSongInPlaylist(const String& targetPath);

// Build shuffle queue for the current mode.
// globalSongCount: pass audioApp.songOffsets.size() for SHUF_GLOBAL
void buildShuffleQueue(ShuffleMode mode, int currentVal, int globalSongCount = 0) {
    g_shuffleQueue.clear();
    g_shufflePos = 0;
    if (mode == SHUF_ALBUM) {
        for (int i = 0; i < (int)g_albumSongOffsets.size(); i++) g_shuffleQueue.push_back(i);
    } else if (mode == SHUF_ARTIST) {
        // Collect global song indices for the current artist by scanning search index once.
        // Search index lines match songOffsets order: line N = song index N.
        // Format: filepath\tartist\ttitle\talbum\tduration\taudioOffset
        String targetArtist = "";
        if (g_currentAlbumIdx >= 0 && g_currentAlbumIdx < (int)g_albumArtistMap.size()) {
            int ai = g_albumArtistMap[g_currentAlbumIdx];
            if (ai >= 0 && ai < (int)g_artistNames.size()) targetArtist = g_artistNames[ai];
        }
        if (targetArtist.length() > 0 && SD.exists(SEARCH_INDEX_FILE)) {
            String targetLower = targetArtist; targetLower.toLowerCase();
            File f = SD.open(SEARCH_INDEX_FILE);
            if (f) {
                int idx = 0;
                while (f.available()) {
                    String line = f.readStringUntil('\n'); line.trim();
                    int t1 = line.indexOf('\t');
                    int t2 = (t1 >= 0) ? line.indexOf('\t', t1 + 1) : -1;
                    if (t1 >= 0 && t2 >= 0) {
                        String artist = line.substring(t1 + 1, t2); artist.toLowerCase();
                        if (artist == targetLower) g_shuffleQueue.push_back(idx);
                    }
                    idx++;
                }
                f.close();
            }
        }
    } else if (mode == SHUF_GLOBAL) {
        for (int i = 0; i < globalSongCount; i++) g_shuffleQueue.push_back(i);
    }
    fisherYatesShuffle(g_shuffleQueue, currentVal);
}

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
        userSettings.lastAlbumIdx = preferences.getInt("lastAlbIdx", -1);
        userSettings.lastAlbumTrack = preferences.getInt("lastAlbTrk", 0);
        userSettings.kbLayoutIndex = preferences.getInt("kbLayout", 0);
        preferences.end();

        // Validate all settings to prevent crashes from corrupted NVS
        userSettings.brightness = constrain(userSettings.brightness, 5, 255);
        userSettings.timeoutIndex = constrain(userSettings.timeoutIndex, 0, 4);
        userSettings.spkRateIndex = constrain(userSettings.spkRateIndex, 0, 4);
        userSettings.powerSaverMode = constrain(userSettings.powerSaverMode, 0, 2);
        userSettings.themeIndex = constrain(userSettings.themeIndex, 0, NUM_THEMES - 1);
        userSettings.visMode = constrain(userSettings.visMode, 0, NUM_VIS_MODES - 1);
        userSettings.seek = constrain(userSettings.seek, 5, 60);
        userSettings.volume = constrain(userSettings.volume, 0, 255);
        if (userSettings.lastIndex < 0) userSettings.lastIndex = 0;
        userSettings.kbLayoutIndex = constrain(userSettings.kbLayoutIndex, 0, NUM_LAYOUTS - 1);
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
        preferences.putInt("lastAlbIdx", userSettings.lastAlbumIdx);
        preferences.putInt("lastAlbTrk", userSettings.lastAlbumTrack);
        preferences.putInt("kbLayout", userSettings.kbLayoutIndex);
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
            file.println(userSettings.showSplash ? 1 : 0);
            file.println(userSettings.lastAlbumIdx); file.println(userSettings.lastAlbumTrack);
            file.println(userSettings.kbLayoutIndex); file.close();
            
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
            if(file.available()) userSettings.lastAlbumIdx = file.readStringUntil('\n').toInt();
            if(file.available()) userSettings.lastAlbumTrack = file.readStringUntil('\n').toInt();
            if(file.available()) userSettings.kbLayoutIndex = file.readStringUntil('\n').toInt();
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
void rebuildSidebarRows() {
    g_sidebarRows.clear();
    for (size_t ai = 0; ai < g_artistNames.size(); ai++) {
        g_sidebarRows.push_back({true, (int)ai});
        if (g_artistExpanded[ai]) {
            for (size_t alb = 0; alb < g_albumOffsets.size(); alb++) {
                if (g_albumArtistMap[alb] == (int)ai) {
                    g_sidebarRows.push_back({false, (int)alb});
                }
            }
        }
    }
}

bool loadAlbumIndex() {
    g_albumOffsets.clear();
    g_artistNames.clear();
    g_albumNames.clear();
    g_albumArtistMap.clear();
    g_artistExpanded.clear();
    g_sidebarCursor = 0;
    g_sidebarScrollOffset = 0;
    if (!SD.exists(ALBUM_INDEX_FILE)) return false;
    File f = SD.open(ALBUM_INDEX_FILE);
    if (!f) return false;
    String header = f.readStringUntil('\n'); header.trim();
    if (header != "ALBUM_INDEX_V2" && header != "ALBUM_INDEX_V1") { f.close(); return false; }
    int currentArtistIdx = -1;
    while (f.available()) {
        uint32_t pos = f.position();
        String line = f.readStringUntil('\n'); line.trim();
        if (line.startsWith("#")) {
            String artistName = line.substring(1);
            g_artistNames.push_back(artistName);
            g_artistExpanded.push_back(true);
            currentArtistIdx = g_artistNames.size() - 1;
        } else if (line.startsWith("@")) {
            g_albumOffsets.push_back(pos);
            g_albumNames.push_back(line.substring(1));
            g_albumArtistMap.push_back(currentArtistIdx >= 0 ? currentArtistIdx : 0);
        }
    }
    f.close();
    // If V1 format (no artist headers), create a single "[Unknown]" artist
    if (g_artistNames.empty() && g_albumOffsets.size() > 0) {
        g_artistNames.push_back("[Unknown]");
        g_artistExpanded.push_back(true);
        for (size_t i = 0; i < g_albumArtistMap.size(); i++) g_albumArtistMap[i] = 0;
    }
    rebuildSidebarRows();
    return (g_albumOffsets.size() > 0);
}

String getAlbumName(int albumIdx) {
    if (albumIdx < 0 || (size_t)albumIdx >= g_albumNames.size()) return "";
    return g_albumNames[albumIdx];
}

void loadAlbumSongs(int albumIdx) {
    g_albumSongOffsets.clear();
    g_albumSongCursor = 0;
    g_albumSongScrollOffset = 0;
    g_currentAlbumIdx = albumIdx;
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
    g_currentAlbumArtist = (albumIdx >= 0 && albumIdx < (int)g_albumArtistMap.size() && g_albumArtistMap[albumIdx] >= 0 && g_albumArtistMap[albumIdx] < (int)g_artistNames.size()) ? g_artistNames[g_albumArtistMap[albumIdx]] : "";
}

String getAlbumSongPath(int songIdx) {
    if (songIdx < 0 || (size_t)songIdx >= g_albumSongOffsets.size()) return "";
    File f = SD.open(ALBUM_INDEX_FILE);
    if (!f) return "";
    f.seek(g_albumSongOffsets[songIdx]);
    String line = f.readStringUntil('\n'); line.trim();
    f.close();
    // Format: TT|/path/to/file.mp3|title
    int pipePos = line.indexOf('|');
    if (pipePos < 0) return "";
    String rest = line.substring(pipePos + 1);
    int pipe2 = rest.indexOf('|');
    String path = (pipe2 >= 0) ? rest.substring(0, pipe2) : rest;
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
    out.album = ""; out.artist = ""; out.title = "";
    out.trackNo = 0; out.duration = 0; out.audioOffset = 0;
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
                if (strcmp(frameId, "TALB") == 0 || strcmp(frameId, "TRCK") == 0 ||
                    strcmp(frameId, "TPE1") == 0 || strcmp(frameId, "TPE2") == 0 || strcmp(frameId, "TIT2") == 0) {
                    uint8_t enc; f.read(&enc, 1);
                    char buf[128]; memset(buf, 0, sizeof(buf));
                    int toRead = min((uint32_t)(frameSize - 1), (uint32_t)127);
                    f.read((uint8_t*)buf, toRead);
                    String val;
                    if (enc == 1 || enc == 2) {
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
                    else if (strcmp(frameId, "TIT2") == 0) out.title = val;
                    else if (strcmp(frameId, "TPE1") == 0 || strcmp(frameId, "TPE2") == 0) { if (out.artist.length() == 0) out.artist = val; }
                    else if (strcmp(frameId, "TRCK") == 0) {
                        int slashPos = val.indexOf('/');
                        if (slashPos > 0) val = val.substring(0, slashPos);
                        out.trackNo = (uint8_t)val.toInt();
                    }
                }
                pos += 10 + frameSize;
                if (out.album.length() > 0 && out.trackNo > 0 && out.artist.length() > 0 && out.title.length() > 0) break;
            }
        }
        // ID3v1 fallback for missing fields
        if ((out.album.length() == 0 || out.title.length() == 0 || out.artist.length() == 0) && fsize > 128) {
            f.seek(fsize - 128);
            uint8_t tag[128]; f.read(tag, 128);
            if (tag[0] == 'T' && tag[1] == 'A' && tag[2] == 'G') {
                char buf31[31]; memset(buf31, 0, 31);
                if (out.title.length() == 0) { memcpy(buf31, tag + 3, 30); String t = String(buf31); t.trim(); if (t.length() > 0) out.title = t; }
                if (out.artist.length() == 0) { memset(buf31, 0, 31); memcpy(buf31, tag + 33, 30); String a = String(buf31); a.trim(); if (a.length() > 0) out.artist = a; }
                if (out.album.length() == 0) { memset(buf31, 0, 31); memcpy(buf31, tag + 63, 30); String al = String(buf31); al.trim(); if (al.length() > 0) out.album = al; }
                if (tag[125] == 0 && tag[126] != 0 && out.trackNo == 0)
                    out.trackNo = tag[126];
            }
        }
        // Extract duration from first MP3 frame (Xing/Info header for VBR, or CBR calculation)
        if (out.duration == 0) {
            // Find first sync word after ID3v2 tag
            uint32_t audioStart = 0;
            f.seek(0); uint8_t id3h[10]; f.read(id3h, 10);
            if (id3h[0] == 'I' && id3h[1] == 'D' && id3h[2] == '3')
                audioStart = 10 + (((uint32_t)(id3h[6] & 0x7F) << 21) | ((uint32_t)(id3h[7] & 0x7F) << 14) | ((uint32_t)(id3h[8] & 0x7F) << 7) | (id3h[9] & 0x7F));
            f.seek(audioStart);
            // Scan for sync word (max 4KB search)
            uint32_t scanLimit = min(audioStart + 4096, (uint32_t)fsize);
            uint8_t sb[2]; bool foundSync = false;
            while (f.position() < scanLimit - 1) {
                f.read(sb, 1);
                if (sb[0] == 0xFF) { f.read(sb + 1, 1); if ((sb[1] & 0xE0) == 0xE0) { foundSync = true; break; } }
            }
            if (foundSync) {
                uint32_t frameStart = f.position() - 2;
                out.audioOffset = frameStart;
                f.seek(frameStart); uint8_t fh[4]; f.read(fh, 4);
                int version = (fh[1] >> 3) & 3;   // 3=MPEG1, 2=MPEG2, 0=MPEG2.5
                int brIdx = (fh[2] >> 4) & 0xF;
                int srIdx = (fh[2] >> 2) & 3;
                static const int brTable[] = {0,32,40,48,56,64,80,96,112,128,160,192,224,256,320,0};
                static const int srTable[] = {44100, 48000, 32000};
                if (brIdx > 0 && brIdx < 15 && srIdx < 3) {
                    int bitrate = brTable[brIdx];
                    int sampleRate = srTable[srIdx];
                    int samplesPerFrame = (version == 3) ? 1152 : 576;
                    int frameSize = samplesPerFrame * bitrate * 1000 / 8 / sampleRate;
                    // Read frame and look for Xing/Info header
                    f.seek(frameStart);
                    uint8_t frameBuf[512]; int toRead = min(frameSize + 4, 512);
                    f.read(frameBuf, toRead);
                    int xingOff = -1;
                    for (int i = 4; i < toRead - 8; i++) {
                        if ((frameBuf[i] == 'X' && frameBuf[i+1] == 'i' && frameBuf[i+2] == 'n' && frameBuf[i+3] == 'g') ||
                            (frameBuf[i] == 'I' && frameBuf[i+1] == 'n' && frameBuf[i+2] == 'f' && frameBuf[i+3] == 'o')) {
                            xingOff = i; break;
                        }
                    }
                    if (xingOff >= 0 && xingOff + 8 < toRead) {
                        uint32_t flags = ((uint32_t)frameBuf[xingOff+4]<<24) | ((uint32_t)frameBuf[xingOff+5]<<16) | ((uint32_t)frameBuf[xingOff+6]<<8) | frameBuf[xingOff+7];
                        if (flags & 1) { // frames field present
                            uint32_t frames = ((uint32_t)frameBuf[xingOff+8]<<24) | ((uint32_t)frameBuf[xingOff+9]<<16) | ((uint32_t)frameBuf[xingOff+10]<<8) | frameBuf[xingOff+11];
                            out.duration = (uint16_t)((uint32_t)frames * samplesPerFrame / sampleRate);
                        }
                    }
                    if (out.duration == 0) {
                        // CBR fallback: (fileSize - audioStart) / (bitrate * 1000 / 8)
                        uint32_t audioBytes = fsize - audioStart;
                        out.duration = (uint16_t)(audioBytes / (bitrate * 1000 / 8));
                    }
                }
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
            if (blockType == 0 && blockLen >= 34) { // STREAMINFO
                // Bytes 10-17: sample rate (20 bits), channels, bps, total samples (36 bits)
                uint8_t si[18]; f.read(si, 18);
                uint32_t sr = ((uint32_t)si[10] << 12) | ((uint32_t)si[11] << 4) | (si[12] >> 4);
                uint32_t totalSamples = ((uint32_t)(si[13] & 0x0F) << 32) | ((uint32_t)si[14] << 24) | ((uint32_t)si[15] << 16) | ((uint32_t)si[16] << 8) | si[17];
                if (sr > 0 && totalSamples > 0) out.duration = (uint16_t)(totalSamples / sr);
                f.seek(f.position() + blockLen - 18);
            }
            else if (blockType == 4) { // VORBIS_COMMENT
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
                    if (commentUpper.startsWith("ALBUM=")) { out.album = comment.substring(6); out.album.trim(); }
                    else if (commentUpper.startsWith("ARTIST=")) { out.artist = comment.substring(7); out.artist.trim(); }
                    else if (commentUpper.startsWith("TITLE=")) { out.title = comment.substring(6); out.title.trim(); }
                    else if (commentUpper.startsWith("TRACKNUMBER=")) { out.trackNo = (uint8_t)comment.substring(12).toInt(); }
                    if (out.album.length() > 0 && out.trackNo > 0 && out.artist.length() > 0 && out.title.length() > 0) break;
                }
                break; // done with vorbis comment block
            } else {
                f.seek(f.position() + blockLen);
            }
            if (lastBlock) out.audioOffset = f.position();
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
            // mvhd atom: contains timescale and duration
            if (strcmp(atomType, "mvhd") == 0 && atomSize >= 28) {
                f.seek(pos + 8); uint8_t ver; f.read(&ver, 1);
                if (ver == 0) { // version 0: 4-byte fields
                    f.seek(pos + 20); uint8_t td[8]; f.read(td, 8);
                    uint32_t timescale = ((uint32_t)td[0]<<24)|((uint32_t)td[1]<<16)|((uint32_t)td[2]<<8)|td[3];
                    uint32_t dur = ((uint32_t)td[4]<<24)|((uint32_t)td[5]<<16)|((uint32_t)td[6]<<8)|td[7];
                    if (timescale > 0) out.duration = (uint16_t)(dur / timescale);
                }
                pos += atomSize; continue;
            }
            // Check for text atoms: \xA9alb (album), \xA9ART (artist), \xA9nam (title)
            if (buf[4] == 0xA9 && pos + atomSize <= searchLimit && atomSize > 24) {
                bool isAlbum = (buf[5] == 'a' && buf[6] == 'l' && buf[7] == 'b');
                bool isArtist = (buf[5] == 'A' && buf[6] == 'R' && buf[7] == 'T');
                bool isTitle = (buf[5] == 'n' && buf[6] == 'a' && buf[7] == 'm');
                if (isAlbum || isArtist || isTitle) {
                    f.seek(pos + 24);
                    int dataLen = min((uint32_t)(atomSize - 24), (uint32_t)127);
                    char dbuf[128]; memset(dbuf, 0, 128);
                    f.read((uint8_t*)dbuf, dataLen);
                    String val = String(dbuf); val.trim();
                    if (isAlbum) out.album = val;
                    else if (isArtist) out.artist = val;
                    else if (isTitle) out.title = val;
                }
                pos += atomSize; continue;
            }
            // Check for track number atom: trkn
            if (strcmp(atomType, "trkn") == 0 && atomSize > 24) {
                f.seek(pos + 24);
                uint8_t trkData[4]; f.read(trkData, 4);
                out.trackNo = (uint8_t)((trkData[2] << 8) | trkData[3]);
                pos += atomSize; continue;
            }
            // mdat atom: actual audio data
            if (strcmp(atomType, "mdat") == 0) { out.audioOffset = pos + 8; }
            pos += atomSize;
            if (out.album.length() > 0 && out.trackNo > 0 && out.artist.length() > 0 && out.title.length() > 0 && out.audioOffset > 0) break;
        }
        f.close(); return true;
    }

    // WAV: read duration from fmt chunk
    if (path.endsWith(".wav") && fsize > 44) {
        f.seek(0); uint8_t riff[12]; f.read(riff, 12);
        if (riff[0]=='R' && riff[1]=='I' && riff[2]=='F' && riff[3]=='F') {
            // Find fmt and data chunks
            uint32_t byteRate = 0;
            while (f.position() < fsize - 8) {
                uint8_t ch[8]; f.read(ch, 8);
                uint32_t ckSize = ch[4] | (ch[5]<<8) | (ch[6]<<16) | (ch[7]<<24);
                if (ch[0]=='f' && ch[1]=='m' && ch[2]=='t' && ch[3]==' ' && ckSize >= 16) {
                    uint8_t fmt[16]; f.read(fmt, 16);
                    byteRate = fmt[8] | (fmt[9]<<8) | (fmt[10]<<16) | (fmt[11]<<24);
                    f.seek(f.position() + ckSize - 16);
                    continue;
                }
                if (ch[0]=='d' && ch[1]=='a' && ch[2]=='t' && ch[3]=='a') {
                    out.audioOffset = f.position();
                    if (byteRate > 0) out.duration = (uint16_t)(ckSize / byteRate);
                    break;
                }
                f.seek(f.position() + ckSize);
            }
        }
    }
    f.close();
    return true;
}

// Forward declarations for album helpers used in AudioEngine
String getAlbumSongPath(int songIdx);
int findSongInPlaylist(const String& targetPath);
void lookupSongMeta(int songIndex);

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
    ShuffleMode shuffleMode = SHUF_OFF;
    LoopState loopMode = NO_LOOP;
    uint32_t paused_at = 0;
    uint32_t pausedSize = 0;
    uint16_t currentDuration = 0;       // pre-calculated duration in seconds (from scan index)
    uint32_t currentAudioOffset = 0;   // byte offset where audio data begins (after ID3/metadata)
    String currentTitle = "";
    String currentArtist = "";
    String currentAlbum = "";

    void listDir(fs::FS &fs, const char *dirname, uint8_t levels, File &playlistFile, File *rawAlbumFile = nullptr, File *searchIdxFile = nullptr) {
        File root = fs.open(dirname); if (!root || !root.isDirectory()) return;
        File f = root.openNextFile();
        while (f) {
            String fname = f.name();
            if (fname.startsWith(".")){
                f = root.openNextFile();
                continue;
            }
            if (f.isDirectory()) { if (levels) listDir(fs, f.path(), levels - 1, playlistFile, rawAlbumFile, searchIdxFile); }
            else {
                String filename = f.name(); String filepath = f.path();
                String filenameLower = filename; filenameLower.toLowerCase();
                if (filenameLower.endsWith(".mp3") || filenameLower.endsWith(".flac") || filenameLower.endsWith(".m4a") || filenameLower.endsWith(".aac") || filenameLower.endsWith(".wav")) {
                    playlistFile.println(filepath);
                    if (rawAlbumFile || searchIdxFile) {
                        TagInfo tag;
                        readTagInfo(filepath.c_str(), tag);
                        if (rawAlbumFile) {
                            String albumName = tag.album.length() > 0 ? tag.album : "[No Album]";
                            String artistName = tag.artist.length() > 0 ? tag.artist : "[Unknown]";
                            String titleName = tag.title.length() > 0 ? tag.title : "";
                            albumName.replace("\t", " "); artistName.replace("\t", " "); titleName.replace("\t", " ");
                            // Raw format: AlbumName\tTrackNo\tFilePath\tArtist\tTitle
                            rawAlbumFile->print(albumName); rawAlbumFile->print("\t");
                            rawAlbumFile->print(tag.trackNo); rawAlbumFile->print("\t");
                            rawAlbumFile->print(filepath); rawAlbumFile->print("\t");
                            rawAlbumFile->print(artistName); rawAlbumFile->print("\t");
                            rawAlbumFile->println(titleName);
                        }
                        if (searchIdxFile) {
                            // Format: filepath\tartist\ttitle\talbum\tduration\taudioOffset
                            searchIdxFile->print(filepath); searchIdxFile->print("\t");
                            searchIdxFile->print(tag.artist); searchIdxFile->print("\t");
                            searchIdxFile->print(tag.title); searchIdxFile->print("\t");
                            searchIdxFile->print(tag.album); searchIdxFile->print("\t");
                            searchIdxFile->print(tag.duration); searchIdxFile->print("\t");
                            searchIdxFile->println(tag.audioOffset);
                        }
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

        // Pass 1: collect unique albums with their most common artist
        // Raw format: AlbumName\tTrackNo\tFilePath\tArtist\tTitle
        struct AlbumInfo { String name; String artist; };
        std::vector<AlbumInfo> albums;
        File raw = SD.open(ALBUM_RAW_FILE);
        if (!raw) return;
        struct RawEntry { String album; String artist; uint8_t trackNo; String path; String title; };
        std::vector<RawEntry> rawEntries;
        while (raw.available()) {
            String line = raw.readStringUntil('\n'); line.trim();
            int tab1 = line.indexOf('\t');
            if (tab1 < 0) continue;
            int tab2 = line.indexOf('\t', tab1 + 1);
            if (tab2 < 0) continue;
            int tab3 = line.indexOf('\t', tab2 + 1);
            int tab4 = (tab3 >= 0) ? line.indexOf('\t', tab3 + 1) : -1;
            String albumName = line.substring(0, tab1);
            uint8_t trackNo = (uint8_t)line.substring(tab1 + 1, tab2).toInt();
            String filePath = (tab3 >= 0) ? line.substring(tab2 + 1, tab3) : line.substring(tab2 + 1);
            String artistName = (tab3 >= 0 && tab4 >= 0) ? line.substring(tab3 + 1, tab4) : ((tab3 >= 0) ? line.substring(tab3 + 1) : "[Unknown]");
            String titleName = (tab4 >= 0) ? line.substring(tab4 + 1) : "";
            artistName.trim(); filePath.trim(); titleName.trim();
            rawEntries.push_back({albumName, artistName, trackNo, filePath, titleName});
        }
        raw.close();

        // Determine unique albums with most common artist
        for (size_t i = 0; i < rawEntries.size(); i++) {
            bool found = false;
            for (size_t j = 0; j < albums.size(); j++) {
                if (albums[j].name == rawEntries[i].album) { found = true; break; }
            }
            if (!found) {
                // Count artists for this album
                String bestArtist = rawEntries[i].artist;
                int bestCount = 0;
                for (size_t k = i; k < rawEntries.size(); k++) {
                    if (rawEntries[k].album != rawEntries[i].album) continue;
                    int count = 0;
                    for (size_t m = i; m < rawEntries.size(); m++) {
                        if (rawEntries[m].album == rawEntries[i].album && rawEntries[m].artist == rawEntries[k].artist) count++;
                    }
                    if (count > bestCount) { bestCount = count; bestArtist = rawEntries[k].artist; }
                }
                albums.push_back({rawEntries[i].album, bestArtist});
            }
        }

        // Sort by artist (case-insensitive), then by album within artist. [Unknown] last.
        for (size_t i = 0; i < albums.size(); i++) {
            for (size_t j = i + 1; j < albums.size(); j++) {
                bool swapNeeded = false;
                String ai = albums[i].artist; ai.toLowerCase();
                String aj = albums[j].artist; aj.toLowerCase();
                String ni = albums[i].name; ni.toLowerCase();
                String nj = albums[j].name; nj.toLowerCase();
                if (albums[i].artist == "[Unknown]" && albums[j].artist != "[Unknown]") swapNeeded = true;
                else if (albums[i].name == "[No Album]" && albums[j].name != "[No Album]") swapNeeded = true;
                else if (albums[j].artist != "[Unknown]" && albums[j].name != "[No Album]") {
                    if (ai > aj) swapNeeded = true;
                    else if (ai == aj && ni > nj) swapNeeded = true;
                }
                if (swapNeeded) { AlbumInfo tmp = albums[i]; albums[i] = albums[j]; albums[j] = tmp; }
            }
        }

        // Pass 2: write index grouped by artist
        if (SD.exists(ALBUM_INDEX_FILE)) SD.remove(ALBUM_INDEX_FILE);
        File idx = SD.open(ALBUM_INDEX_FILE, FILE_WRITE);
        if (!idx) return;
        idx.println("ALBUM_INDEX_V2");

        String lastArtist = "";
        for (size_t ai = 0; ai < albums.size(); ai++) {
            // Write artist header if new artist
            if (albums[ai].artist != lastArtist) {
                idx.print("#"); idx.println(albums[ai].artist);
                lastArtist = albums[ai].artist;
            }

            // Collect songs for this album from in-memory raw entries
            struct SongEntry { uint8_t trackNo; String path; String title; };
            std::vector<SongEntry> songs;
            for (size_t ri = 0; ri < rawEntries.size(); ri++) {
                if (rawEntries[ri].album == albums[ai].name) {
                    songs.push_back({rawEntries[ri].trackNo, rawEntries[ri].path, rawEntries[ri].title});
                }
            }

            // Sort by track number
            for (size_t i = 0; i < songs.size(); i++) {
                for (size_t j = i + 1; j < songs.size(); j++) {
                    if (songs[j].trackNo < songs[i].trackNo) {
                        SongEntry tmp = songs[i]; songs[i] = songs[j]; songs[j] = tmp;
                    }
                }
            }

            // Write album header and songs
            idx.print("@"); idx.println(albums[ai].name);
            for (size_t si = 0; si < songs.size(); si++) {
                char trackStr[4]; sprintf(trackStr, "%02d", songs[si].trackNo);
                idx.print(trackStr); idx.print("|"); idx.print(songs[si].path); idx.print("|"); idx.println(songs[si].title);
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
        if (SD.exists(SEARCH_INDEX_FILE)) SD.remove(SEARCH_INDEX_FILE);
        File playlistFile = SD.open(PLAYLIST_FILE, FILE_WRITE);
        File rawAlbumFile = SD.open(ALBUM_RAW_FILE, FILE_WRITE);
        File searchIdxFile = SD.open(SEARCH_INDEX_FILE, FILE_WRITE);
        if (playlistFile) {
            listDir(SD, "/", 3, playlistFile,
                    rawAlbumFile ? &rawAlbumFile : nullptr,
                    searchIdxFile ? &searchIdxFile : nullptr);
            playlistFile.close();
        }
        if (rawAlbumFile) rawAlbumFile.close();
        if (searchIdxFile) searchIdxFile.close();
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
        if (fname.length() == 0) return false;

        file = new AudioFileSourceSD(fname.c_str());
        buff = new AudioFileSourceBuffer(file, 16384);
        id3 = new AudioFileSourceID3(buff);
        if (!file || !buff || !id3) { stop(); return false; }
        id3->RegisterMetadataCB(MDCallback, (void*)"ID3TAG");

        if (startPos > 0) id3->seek(startPos, 1);
        String fnameLower = fname; fnameLower.toLowerCase();
        if (fnameLower.endsWith(".flac")) decoder = new AudioGeneratorFLAC();
        else if (fnameLower.endsWith(".m4a") || fnameLower.endsWith(".aac")) decoder = new AudioGeneratorAAC();
        else if (fnameLower.endsWith(".wav")) decoder = new AudioGeneratorWAV();
        else decoder = new AudioGeneratorMP3();
        if (!decoder) { stop(); return false; }

        isPaused = false;
        lookupSongMeta(currentIndex);
        bool ok = decoder->begin(id3, out);
        if (ok) ConfigManager::save(startPos, currentIndex);
        return ok;
    }

    void togglePause() {
        if (!decoder) return;
        if (decoder->isRunning()) {
            paused_at = id3->getPos(); pausedSize = id3->getSize();
            decoder->stop(); isPaused = true;
        } else if (isPaused) {
            String savedTitle = currentTitle, savedArtist = currentArtist, savedAlbum = currentAlbum;
            uint16_t savedDur = currentDuration;
            uint32_t savedOffset = currentAudioOffset;
            play(currentIndex, paused_at);
            currentTitle = savedTitle; currentArtist = savedArtist; currentAlbum = savedAlbum;
            currentDuration = savedDur;
            currentAudioOffset = savedOffset;
        }
    }

    void seek(int seconds) {
        if (!decoder || !decoder->isRunning() || !id3) return;
        // Convert seconds to bytes using audio-only size and pre-calculated duration
        uint32_t audioSize = id3->getSize() - currentAudioOffset;
        float bps = (currentDuration > 0 && audioSize > 0) ? (float)audioSize / currentDuration : 32000;
        int32_t newPos = id3->getPos() + (int32_t)(seconds * bps);
        if (newPos < (int32_t)currentAudioOffset) newPos = currentAudioOffset;
        if (newPos > id3->getSize()) newPos = id3->getSize() - 1000;
        String savedTitle = currentTitle, savedArtist = currentArtist, savedAlbum = currentAlbum;
        uint16_t savedDur = currentDuration;
        uint32_t savedOffset = currentAudioOffset;
        play(currentIndex, newPos);
        currentTitle = savedTitle; currentArtist = savedArtist; currentAlbum = savedAlbum;
        currentDuration = savedDur;
        currentAudioOffset = savedOffset;
    }

    void next(bool autoPlay = false) {
        if (songOffsets.empty()) return;
        if (autoPlay && loopMode == LOOP_ONE) { play(currentIndex); return; }

        // Artist/Global shuffle: use shared queue with global indices
        if (shuffleMode == SHUF_ARTIST || shuffleMode == SHUF_GLOBAL) {
            if (g_shuffleQueue.empty()) buildShuffleQueue(shuffleMode, currentIndex, songOffsets.size());
            g_shufflePos++;
            if (g_shufflePos >= (int)g_shuffleQueue.size()) {
                if (loopMode == LOOP_ALL) { buildShuffleQueue(shuffleMode, -1, songOffsets.size()); g_shufflePos = 0; }
                else { stop(); return; }
            }
            play(g_shuffleQueue[g_shufflePos]);
            return;
        }

        // Album-scoped playback (sequential or album shuffle)
        if (g_albumPlaybackActive && g_albumSongOffsets.size() > 0) {
            if (shuffleMode == SHUF_ALBUM) {
                if (g_shuffleQueue.empty()) buildShuffleQueue(SHUF_ALBUM, g_albumPlaybackIndex);
                g_shufflePos++;
                if (g_shufflePos >= (int)g_shuffleQueue.size()) {
                    if (loopMode == LOOP_ALL) { buildShuffleQueue(SHUF_ALBUM, -1); g_shufflePos = 0; }
                    else {
                        g_albumPlaybackIndex = 0; userSettings.lastAlbumTrack = 0;
                        String firstPath = getAlbumSongPath(0);
                        int globalIdx = findSongInPlaylist(firstPath);
                        if (globalIdx >= 0) { play(globalIdx); togglePause(); }
                        else { stop(); g_albumPlaybackActive = false; }
                        return;
                    }
                }
                g_albumPlaybackIndex = g_shuffleQueue[g_shufflePos];
            } else {
                g_albumPlaybackIndex++;
                if (g_albumPlaybackIndex >= (int)g_albumSongOffsets.size()) {
                    if (loopMode == LOOP_ALL) g_albumPlaybackIndex = 0;
                    else {
                        g_albumPlaybackIndex = 0; userSettings.lastAlbumTrack = 0;
                        String firstPath = getAlbumSongPath(0);
                        int globalIdx = findSongInPlaylist(firstPath);
                        if (globalIdx >= 0) { play(globalIdx); togglePause(); }
                        else { stop(); g_albumPlaybackActive = false; }
                        return;
                    }
                }
            }
            userSettings.lastAlbumTrack = g_albumPlaybackIndex;
            String songPath = getAlbumSongPath(g_albumPlaybackIndex);
            int globalIdx = findSongInPlaylist(songPath);
            if (globalIdx >= 0) play(globalIdx); else { g_albumPlaybackActive = false; }
            return;
        }

        // Global sequential (no album context, no shuffle)
        currentIndex++;
        if ((size_t)currentIndex >= songOffsets.size()) { if (loopMode == LOOP_ALL) currentIndex = 0; else { stop(); currentIndex = 0; return; } }
        play(currentIndex);
    }

    void prev() {
        if (songOffsets.empty()) return;

        // Artist/Global shuffle: go back in shared queue
        if (shuffleMode == SHUF_ARTIST || shuffleMode == SHUF_GLOBAL) {
            if (g_shuffleQueue.size() > 0) {
                if (g_shufflePos > 0) g_shufflePos--;
                else g_shufflePos = g_shuffleQueue.size() - 1; // wrap to end
                play(g_shuffleQueue[g_shufflePos]);
            }
            return;
        }

        // Album-scoped playback
        if (g_albumPlaybackActive && g_albumSongOffsets.size() > 0) {
            if (shuffleMode == SHUF_ALBUM) {
                if (g_shuffleQueue.size() > 0) {
                    if (g_shufflePos > 0) g_shufflePos--;
                    else g_shufflePos = g_shuffleQueue.size() - 1; // wrap to end
                    g_albumPlaybackIndex = g_shuffleQueue[g_shufflePos];
                }
            } else {
                g_albumPlaybackIndex--;
                if (g_albumPlaybackIndex < 0) g_albumPlaybackIndex = (int)g_albumSongOffsets.size() - 1;
            }
            userSettings.lastAlbumTrack = g_albumPlaybackIndex;
            String songPath = getAlbumSongPath(g_albumPlaybackIndex);
            int globalIdx = findSongInPlaylist(songPath);
            if (globalIdx >= 0) play(globalIdx); else { g_albumPlaybackActive = false; }
            return;
        }

        // Global sequential
        currentIndex--; if (currentIndex < 0) currentIndex = songOffsets.size() - 1;
        play(currentIndex);
    }

    // Elapsed = duration * (audioPos / audioSize)
    // Subtracts audioOffset so the ratio covers only audio data, not metadata headers
    int getElapsedSec() {
        if (currentDuration == 0) return 0;
        uint32_t pos = isPaused ? paused_at : (id3 ? id3->getPos() : 0);
        uint32_t size = isPaused ? pausedSize : (id3 ? id3->getSize() : 0);
        if (size <= currentAudioOffset) return 0;
        uint32_t audioSize = size - currentAudioOffset;
        uint32_t audioPos = (pos > currentAudioOffset) ? pos - currentAudioOffset : 0;
        if (audioPos > audioSize) audioPos = audioSize;
        return (int)((uint64_t)currentDuration * audioPos / audioSize);
    }

    int getTotalSec() { return currentDuration; }

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

// Look up pre-calculated duration and audio offset from search index.
// Search index lines: filepath\tartist\ttitle\talbum\tduration\taudioOffset
// Search index format: filepath\tartist\ttitle\talbum\tduration\taudioOffset
void lookupSongMeta(int songIndex) {
    audioApp.currentDuration = 0;
    audioApp.currentAudioOffset = 0;
    if (!SD.exists(SEARCH_INDEX_FILE)) return;
    File f = SD.open(SEARCH_INDEX_FILE);
    if (!f) return;
    int idx = 0;
    while (f.available()) {
        String line = f.readStringUntil('\n');
        if (idx == songIndex) {
            f.close();
            line.trim();
            // Parse 6 tab-separated fields
            int t1 = line.indexOf('\t');                          // after filepath
            int t2 = (t1 >= 0) ? line.indexOf('\t', t1+1) : -1; // after artist
            int t3 = (t2 >= 0) ? line.indexOf('\t', t2+1) : -1; // after title
            int t4 = (t3 >= 0) ? line.indexOf('\t', t3+1) : -1; // after album
            int t5 = (t4 >= 0) ? line.indexOf('\t', t4+1) : -1; // after duration
            if (t1 >= 0 && t2 >= 0) {
                String artist = line.substring(t1+1, t2); artist.trim();
                if (artist.length() > 0 && audioApp.currentArtist.length() == 0) audioApp.currentArtist = artist;
            }
            if (t2 >= 0 && t3 >= 0) {
                String title = line.substring(t2+1, t3); title.trim();
                if (title.length() > 0 && audioApp.currentTitle.length() == 0) audioApp.currentTitle = title;
            }
            if (t3 >= 0 && t4 >= 0) {
                String album = line.substring(t3+1, t4); album.trim();
                if (album.length() > 0 && audioApp.currentAlbum.length() == 0) audioApp.currentAlbum = album;
            }
            if (t4 >= 0 && t5 >= 0) {
                audioApp.currentDuration = (uint16_t)line.substring(t4+1, t5).toInt();
            }
            if (t5 >= 0) {
                audioApp.currentAudioOffset = (uint32_t)line.substring(t5+1).toInt();
            }
            return;
        }
        idx++;
    }
    f.close();
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
    // Multi-token search: "li fa" matches "Linkin Park - Faint"
    // Each space-separated token must appear somewhere in the text.
    static bool matchAllTokens(const String& text, const String& queryLower) {
        String remaining = queryLower;
        remaining.trim();
        while (remaining.length() > 0) {
            int sp = remaining.indexOf(' ');
            String token;
            if (sp >= 0) { token = remaining.substring(0, sp); remaining = remaining.substring(sp + 1); remaining.trim(); }
            else { token = remaining; remaining = ""; }
            if (token.length() == 0) continue;
            if (text.indexOf(token) < 0) return false;
        }
        return true;
    }

    static void rebuildSearchResults() {
        g_searchResults.clear();
        g_searchCursor = 0;
        g_searchScrollOffset = 0;
        if (g_searchQuery.length() == 0) return;
        String queryLower = utf8ToLower(g_searchQuery);

        // Use search index (filepath + artist + title + album) if available
        bool useIndex = SD.exists(SEARCH_INDEX_FILE);
        File f = SD.open(useIndex ? SEARCH_INDEX_FILE : PLAYLIST_FILE);
        if (!f) return;
        int idx = 0;
        while (f.available()) {
            String line = f.readStringUntil('\n'); line.trim();
            String lineLower = utf8ToLower(line);
            // Replace tabs with spaces so all fields are searchable as one string
            lineLower.replace("\t", " ");
            if (matchAllTokens(lineLower, queryLower)) {
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
        M5Cardputer.Display.setFont(TAG_FONT);
        M5Cardputer.Display.setTextColor(C_TEXT_MAIN, C_HEADER);
        M5Cardputer.Display.setCursor(5, 5);
        M5Cardputer.Display.print("Search Songs");
        // Layout indicator on right side of header
        String layoutLabel = g_layoutNames[userSettings.kbLayoutIndex];
        M5Cardputer.Display.setTextColor(C_ACCENT, C_HEADER);
        M5Cardputer.Display.setCursor(M5Cardputer.Display.width() - 22, 5);
        M5Cardputer.Display.print(layoutLabel);

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
            // Pre-read display names from search index (artist - title) before drawing
            String displayNames[MAX_VISIBLE_ROWS];
            int songIndices[MAX_VISIBLE_ROWS];
            int visCount = 0;
            bool useIdx = SD.exists(SEARCH_INDEX_FILE);
            File idxF = useIdx ? SD.open(SEARCH_INDEX_FILE) : File();
            for (int i = 0; i < MAX_VISIBLE_ROWS; i++) {
                int ri = g_searchScrollOffset + i;
                if (ri >= totalResults) break;
                int songIdx = g_searchResults[ri];
                songIndices[visCount] = songIdx;
                // Try to get artist - title from search index
                String dispName = "";
                if (idxF && useIdx) {
                    // Read through index to find line N = songIdx
                    idxF.seek(0);
                    int lineIdx = 0;
                    while (idxF.available()) {
                        String line = idxF.readStringUntil('\n');
                        if (lineIdx == songIdx) {
                            line.trim();
                            int t1 = line.indexOf('\t');
                            int t2 = (t1 >= 0) ? line.indexOf('\t', t1+1) : -1;
                            int t3 = (t2 >= 0) ? line.indexOf('\t', t2+1) : -1;
                            String artist = (t1 >= 0 && t2 >= 0) ? line.substring(t1+1, t2) : "";
                            String title = (t2 >= 0 && t3 >= 0) ? line.substring(t2+1, t3) : "";
                            artist.trim(); title.trim();
                            if (artist.length() > 0 && title.length() > 0)
                                dispName = artist + " - " + title;
                            else if (title.length() > 0) dispName = title;
                            else if (artist.length() > 0) dispName = artist;
                            break;
                        }
                        lineIdx++;
                    }
                }
                // Fallback to filename
                if (dispName.length() == 0) {
                    String songPath = audioApp.getSongPath(songIdx);
                    int slash = songPath.lastIndexOf('/');
                    dispName = (slash >= 0) ? songPath.substring(slash + 1) : songPath;
                }
                displayNames[visCount] = truncateToFit(dispName, 228);
                visCount++;
            }
            if (idxF) idxF.close();

            // Now draw from RAM
            for (int i = 0; i < visCount; i++) {
                int ri = g_searchScrollOffset + i;
                bool isSelected = (ri == g_searchCursor);
                bool isPlaying  = (songIndices[i] == audioApp.currentIndex);

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
                M5Cardputer.Display.print(displayNames[i]);
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

    // Draw a label + value that wraps to next line if needed. Returns updated y.
    static int drawTagLine(int px, int y, int lh, int maxW, const char* label, const String& value, uint16_t labelColor, uint16_t valueColor) {
        M5Cardputer.Display.setTextColor(labelColor);
        M5Cardputer.Display.setCursor(px + 8, y);
        M5Cardputer.Display.print(label);
        int labelW = M5Cardputer.Display.textWidth(label);
        int remaining = (maxW - labelW) / 6; // chars that fit after label on same line
        M5Cardputer.Display.setTextColor(valueColor);
        M5Cardputer.Display.setCursor(px + 8 + labelW, y);
        if ((int)value.length() <= remaining) {
            M5Cardputer.Display.print(value);
            return y + lh;
        }
        M5Cardputer.Display.print(value.substring(0, remaining));
        y += lh;
        // Wrap remaining text
        int pos = remaining;
        int fullLine = maxW / 6;
        while (pos < (int)value.length()) {
            M5Cardputer.Display.setCursor(px + 8, y);
            M5Cardputer.Display.print(value.substring(pos, pos + fullLine));
            pos += fullLine;
            y += lh;
        }
        return y;
    }

    static void drawTrackInfo() {
        drawPopup("TRACK INFO", "Press 'T' to Close");
        int px = 15, py = 15;
        int y = py + 25;
        int lh = 11;
        int maxW = 194; // popup content width in pixels
        M5Cardputer.Display.setFont(TAG_FONT);

        String title = audioApp.currentTitle.length() > 0 ? audioApp.currentTitle : "(unknown)";
        String artist = audioApp.currentArtist.length() > 0 ? audioApp.currentArtist : "(unknown)";
        String album = audioApp.currentAlbum.length() > 0 ? audioApp.currentAlbum : "(unknown)";

        y = drawTagLine(px, y, lh, maxW, "Title: ", title, C_ACCENT, C_TEXT_MAIN);
        y = drawTagLine(px, y, lh, maxW, "Artist: ", artist, C_ACCENT, C_TEXT_MAIN);
        y = drawTagLine(px, y, lh, maxW, "Album: ", album, C_ACCENT, C_TEXT_MAIN);
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
        M5Cardputer.Display.setFont(TAG_FONT);
        int totalRows = g_sidebarRows.size();
        int yPos = HEADER_HEIGHT + 2, xPos = 0;

        M5Cardputer.Display.fillRect(0, HEADER_HEIGHT, PLAYLIST_WIDTH, M5Cardputer.Display.height() - HEADER_HEIGHT - BOTTOM_BAR_HEIGHT, C_BG_LIGHT);
        M5Cardputer.Display.drawFastVLine(PLAYLIST_WIDTH, HEADER_HEIGHT, M5Cardputer.Display.height() - HEADER_HEIGHT - BOTTOM_BAR_HEIGHT, C_BG_DARK);

        if (totalRows == 0) {
            M5Cardputer.Display.setTextColor(C_TEXT_DIM);
            M5Cardputer.Display.setCursor(xPos + 5, yPos + 3);
            M5Cardputer.Display.print("No albums");
            return;
        }

        int startIdx = max(0, min(g_sidebarCursor - (MAX_VISIBLE_ROWS / 2), totalRows - MAX_VISIBLE_ROWS));

        for (int i = 0; i < MAX_VISIBLE_ROWS; i++) {
            int ri = startIdx + i; if (ri >= totalRows) break;
            SidebarRow& row = g_sidebarRows[ri];
            bool isSelected = (ri == g_sidebarCursor);

            if (row.isArtist) {
                // Artist header row
                String name = (row.dataIdx >= 0 && row.dataIdx < (int)g_artistNames.size()) ? g_artistNames[row.dataIdx] : "";
                bool expanded = (row.dataIdx >= 0 && row.dataIdx < (int)g_artistExpanded.size()) ? g_artistExpanded[row.dataIdx] : true;
                if (isSelected) {
                    M5Cardputer.Display.fillRect(xPos + 2, yPos, PLAYLIST_WIDTH - 6, ROW_HEIGHT, C_ACCENT);
                    M5Cardputer.Display.setTextColor(C_BG_DARK);
                } else {
                    M5Cardputer.Display.setTextColor(C_HIGHLIGHT);
                }
                String prefix = expanded ? "- " : "+ ";
                String dispName = truncateToFit(prefix + name, 114);
                M5Cardputer.Display.setCursor(xPos + 3, yPos + 3);
                M5Cardputer.Display.print(dispName);
            } else {
                // Album row (indented)
                String name = (row.dataIdx >= 0 && row.dataIdx < (int)g_albumNames.size()) ? g_albumNames[row.dataIdx] : "";
                bool isCurrentAlbum = (audioApp.currentAlbum.length() > 0 && name == audioApp.currentAlbum);
                if (isSelected) {
                    M5Cardputer.Display.fillRect(xPos + 2, yPos, PLAYLIST_WIDTH - 6, ROW_HEIGHT, C_ACCENT);
                    M5Cardputer.Display.setTextColor(C_BG_DARK);
                } else if (isCurrentAlbum) {
                    M5Cardputer.Display.setTextColor(C_PLAYING);
                } else {
                    M5Cardputer.Display.setTextColor(C_TEXT_DIM);
                }
                String dispName = truncateToFit("  " + name, 108);
                M5Cardputer.Display.setCursor(xPos + 5, yPos + 3);
                if (isCurrentAlbum && !isSelected) M5Cardputer.Display.print(">");
                M5Cardputer.Display.print(dispName);
            }
            yPos += ROW_HEIGHT;
        }
    }

    static void drawAlbumSongsHeader() {
        M5Cardputer.Display.fillRect(0, 0, M5Cardputer.Display.width(), HEADER_HEIGHT, C_HEADER);
        M5Cardputer.Display.setFont(TAG_FONT);
        M5Cardputer.Display.setTextColor(C_TEXT_MAIN, C_HEADER);
        M5Cardputer.Display.setCursor(5, 5);
        String hdr = g_currentAlbumName;
        hdr = truncateToFit(hdr, 228);
        M5Cardputer.Display.print(hdr);
    }

    static void drawAlbumSongsFooter() {
        int footerY = M5Cardputer.Display.height() - BOTTOM_BAR_HEIGHT;
        M5Cardputer.Display.fillRect(0, footerY, M5Cardputer.Display.width(), BOTTOM_BAR_HEIGHT, C_HEADER);
        M5Cardputer.Display.setCursor(5, footerY + 4);
        if (g_trackNumInput.length() > 0) {
            M5Cardputer.Display.setTextColor(C_ACCENT, C_HEADER);
            M5Cardputer.Display.print("Track: "); M5Cardputer.Display.print(g_trackNumInput); M5Cardputer.Display.print("_");
        } else {
            M5Cardputer.Display.setTextColor(C_TEXT_DIM, C_HEADER);
            M5Cardputer.Display.print("Enter:Play ;/.:Scroll 0-9:Jump");
        }
    }

    // Redraws only the song list rows — not the header or footer.
    // This is the function called on scroll actions.
    static void drawAlbumSongsList() {
        M5Cardputer.Display.setFont(TAG_FONT);
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
                    // Format: TT|filepath|title
                    int pipePos = line.indexOf('|');
                    if (pipePos >= 0) {
                        String trackStr = line.substring(0, pipePos);
                        String rest = line.substring(pipePos + 1);
                        int pipe2 = rest.indexOf('|');
                        if (pipe2 >= 0) {
                            rows[rowCount].songPath = rest.substring(0, pipe2); rows[rowCount].songPath.trim();
                            String title = rest.substring(pipe2 + 1); title.trim();
                            // Use title tag if available, fall back to filename
                            String displayText;
                            if (title.length() > 0) {
                                displayText = title;
                            } else {
                                String sp = rows[rowCount].songPath;
                                int slash = sp.lastIndexOf('/');
                                displayText = (slash >= 0) ? sp.substring(slash + 1) : sp;
                                int dotPos = displayText.lastIndexOf('.');
                                if (dotPos > 0) displayText = displayText.substring(0, dotPos);
                                if (g_currentAlbumArtist.length() > 0) {
                                    String prefix = g_currentAlbumArtist + " - ";
                                    if (displayText.startsWith(prefix)) displayText = displayText.substring(prefix.length());
                                }
                            }
                            rows[rowCount].displayName = truncateToFit(trackStr + " " + displayText, 228);
                        } else {
                            // Old format fallback: TT|filepath (no title)
                            rows[rowCount].songPath = rest; rows[rowCount].songPath.trim();
                            String sp = rows[rowCount].songPath;
                            int slash = sp.lastIndexOf('/');
                            String fname = (slash >= 0) ? sp.substring(slash + 1) : sp;
                            int dotPos = fname.lastIndexOf('.');
                            if (dotPos > 0) fname = fname.substring(0, dotPos);
                            rows[rowCount].displayName = truncateToFit(trackStr + " " + fname, 228);
                        }
                        if (rows[rowCount].songPath.length() > 0 && !rows[rowCount].songPath.startsWith("/"))
                            rows[rowCount].songPath = "/" + rows[rowCount].songPath;
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
                    M5Cardputer.Display.setTextColor(TFT_RED);
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

    // Full redraw of the now-playing area — called on state changes (play, pause, next, etc.)
    static void drawNowPlaying() {
        int xStart = PLAYLIST_WIDTH + 5, yStart = HEADER_HEIGHT + 5;
        M5Cardputer.Display.fillRect(xStart, yStart, M5Cardputer.Display.width() - xStart, 50, C_BG_DARK);

        M5Cardputer.Display.setFont(TAG_FONT); M5Cardputer.Display.setTextColor(audioApp.isPaused ? TFT_RED : C_PLAYING);
        M5Cardputer.Display.setCursor(xStart + 5, yStart); M5Cardputer.Display.print(audioApp.isPaused ? "PAUSED" : "PLAYING");

        M5Cardputer.Display.setCursor(M5Cardputer.Display.width() - 55, yStart);
        switch(audioApp.shuffleMode) {
            case SHUF_OFF: M5Cardputer.Display.setTextColor(C_BG_LIGHT); M5Cardputer.Display.print(" "); break;
            case SHUF_ALBUM: M5Cardputer.Display.setTextColor(C_HIGHLIGHT); M5Cardputer.Display.print("SA"); break;
            case SHUF_ARTIST: M5Cardputer.Display.setTextColor(C_ACCENT); M5Cardputer.Display.print("SR"); break;
            case SHUF_GLOBAL: M5Cardputer.Display.setTextColor(C_PLAYING); M5Cardputer.Display.print("SG"); break;
        }

        M5Cardputer.Display.setCursor(M5Cardputer.Display.width() - 28, yStart);
        switch(audioApp.loopMode) {
            case NO_LOOP: M5Cardputer.Display.setTextColor(C_BG_LIGHT); M5Cardputer.Display.print("1x"); break;
            case LOOP_ALL: M5Cardputer.Display.setTextColor(C_ACCENT); M5Cardputer.Display.print("LP"); break;
            case LOOP_ONE: M5Cardputer.Display.setTextColor(C_HIGHLIGHT); M5Cardputer.Display.print("1T"); break;
        }

        M5Cardputer.Display.setTextColor(C_TEXT_MAIN); M5Cardputer.Display.setCursor(xStart + 5, yStart + 15);
        if (audioApp.currentTitle.length() > 0) {
            String t = truncateToFit(audioApp.currentTitle, M5Cardputer.Display.width() - PLAYLIST_WIDTH - 20);
            M5Cardputer.Display.print(t);
        }

        drawProgressBar();

        int volY = yStart + 42; M5Cardputer.Display.setCursor(xStart + 5, volY); M5Cardputer.Display.setFont(&fonts::Font0);
        M5Cardputer.Display.setTextColor(C_ACCENT); M5Cardputer.Display.print("VOL "); M5Cardputer.Display.drawRect(xStart + 30, volY, 60, 6, C_BG_LIGHT);
        M5Cardputer.Display.fillRect(xStart + 31, volY + 1, (M5Cardputer.Speaker.getVolume() * 58) / 255, 4, C_ACCENT);
    }

    // Progress bar only — called every second in the loop, no flicker
    static void drawProgressBar() {
        int xStart = PLAYLIST_WIDTH + 5, yStart = HEADER_HEIGHT + 5;
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

    static void drawVisNowPlayingInfo(int textX, int maxPixels) {
        String artist = audioApp.currentArtist.length() > 0 ? audioApp.currentArtist : "Unknown Artist";
        String album = audioApp.currentAlbum.length() > 0 ? audioApp.currentAlbum : "Unknown Album";
        visSprite.setFont(TAG_FONT);
        visSprite.setTextColor(C_TEXT_MAIN); visSprite.setCursor(textX, 4); visSprite.print(truncateToFit(artist, maxPixels));
        visSprite.setTextColor(C_TEXT_DIM); visSprite.setCursor(textX, 16); visSprite.print(truncateToFit(album, maxPixels));
        int elapsedSec = audioApp.getElapsedSec();
        int totalSec = audioApp.getTotalSec();
        char timeStr[16];
        sprintf(timeStr, "%02d:%02d/%02d:%02d", elapsedSec / 60, elapsedSec % 60, totalSec / 60, totalSec % 60);
        visSprite.setTextColor(C_HIGHLIGHT); visSprite.setCursor(textX, 28); visSprite.print(timeStr);
    }

    static void drawVisualizer() {
        if (!showVisualizer) return;
        // Info modes (3, 4) render even when paused; audio modes (0-2) need active decoder
        bool isInfoMode = (userSettings.visMode == 3 || userSettings.visMode == 4);
        if (!isInfoMode && (!audioApp.decoder || !audioApp.decoder->isRunning() || audioApp.isPaused)) return;
        if (userSettings.visMode >= NUM_VIS_MODES - 1) return; // OFF (last mode)

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
            drawVisNowPlayingInfo(45, 73);
            visSprite.pushSprite(PLAYLIST_WIDTH + 2, HEADER_HEIGHT + 55);
            return;
        }

        if (userSettings.visMode == 4) {
            // Text-only info — no animation, full width for text
            visSprite.fillScreen(C_BG_DARK);
            drawVisNowPlayingInfo(4, 112);
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

    // Factory reset: hold Esc/` key during boot to erase all settings
    delay(100); // let keyboard controller settle
    M5Cardputer.update();
    if (M5Cardputer.Keyboard.isKeyPressed('`')) {
        nvs_flash_erase(); nvs_flash_init();
        M5Cardputer.Display.setRotation(1);
        M5Cardputer.Display.fillScreen(TFT_BLACK);
        M5Cardputer.Display.setTextColor(TFT_RED);
        M5Cardputer.Display.setCursor(30, 55);
        M5Cardputer.Display.print("Settings Reset!");
        delay(2000);
    }

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

    // Restore album playback context if saved
    if (userSettings.lastAlbumIdx >= 0 && userSettings.lastAlbumIdx < (int)g_albumOffsets.size()) {
        loadAlbumSongs(userSettings.lastAlbumIdx);
        g_albumPlaybackActive = true;
        g_albumPlaybackIndex = constrain(userSettings.lastAlbumTrack, 0, max(0, (int)g_albumSongOffsets.size() - 1));
    }
    
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
        static unsigned long lastProg = 0; if (millis() - lastProg > 1000) { UIManager::drawProgressBar(); lastProg = millis(); }
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
        if (isScreenOff) {
            M5Cardputer.Display.wakeup(); M5Cardputer.Display.setBrightness(userSettings.brightness); isScreenOff = false; lastInputTime = millis();
            if (currentState == UI_ALBUM_SONGS) UIManager::drawAlbumSongs();
            else if (currentState == UI_SETTINGS) UIManager::drawSettings();
            else if (currentState == UI_HELP) UIManager::drawHelp();
            else if (currentState == UI_SEARCH) UIManager::drawSearch();
            else if (currentState == UI_TRACK_INFO) UIManager::drawTrackInfo();
            else UIManager::drawBaseUI();
            return;
        }
        lastInputTime = millis();

        switch (currentState) {
            case UI_PLAYER:
                if (M5Cardputer.Keyboard.isKeyPressed('`')) { currentState = UI_SETTINGS; UIManager::drawSettings(); }
                else if (M5Cardputer.Keyboard.isKeyPressed('i')) { currentState = UI_HELP; UIManager::drawHelp(); }
                else if (M5Cardputer.Keyboard.isKeyPressed('t')) { currentState = UI_TRACK_INFO; UIManager::drawTrackInfo(); }
                else if (M5Cardputer.Keyboard.isKeyPressed(';')) {
                    if (g_sidebarRows.size() > 0) {
                        g_sidebarCursor = (g_sidebarCursor - 1 + g_sidebarRows.size()) % g_sidebarRows.size();
                        if (g_sidebarCursor < g_sidebarScrollOffset) g_sidebarScrollOffset = g_sidebarCursor;
                        if (g_sidebarCursor == (int)g_sidebarRows.size() - 1)
                            g_sidebarScrollOffset = max(0, (int)g_sidebarRows.size() - MAX_VISIBLE_ROWS);
                        UIManager::drawPlaylist();
                    }
                }
                else if (M5Cardputer.Keyboard.isKeyPressed('.')) {
                    if (g_sidebarRows.size() > 0) {
                        g_sidebarCursor = (g_sidebarCursor + 1) % g_sidebarRows.size();
                        if (g_sidebarCursor >= g_sidebarScrollOffset + MAX_VISIBLE_ROWS) g_sidebarScrollOffset++;
                        if (g_sidebarCursor == 0) g_sidebarScrollOffset = 0;
                        UIManager::drawPlaylist();
                    }
                }
                else if (M5Cardputer.Keyboard.isKeyPressed(KEY_ENTER)) {
                    if (g_sidebarRows.size() > 0 && g_sidebarCursor < (int)g_sidebarRows.size()) {
                        SidebarRow& row = g_sidebarRows[g_sidebarCursor];
                        if (row.isArtist) {
                            // Toggle expand/collapse
                            if (row.dataIdx >= 0 && row.dataIdx < (int)g_artistExpanded.size()) {
                                g_artistExpanded[row.dataIdx] = !g_artistExpanded[row.dataIdx];
                                rebuildSidebarRows();
                                // Keep cursor on the same artist row
                                for (int i = 0; i < (int)g_sidebarRows.size(); i++) {
                                    if (g_sidebarRows[i].isArtist && g_sidebarRows[i].dataIdx == row.dataIdx) { g_sidebarCursor = i; break; }
                                }
                                if (g_sidebarCursor >= (int)g_sidebarRows.size()) g_sidebarCursor = g_sidebarRows.size() - 1;
                            }
                            UIManager::drawPlaylist();
                        } else {
                            // Open album
                            g_trackNumInput = "";
                            loadAlbumSongs(row.dataIdx);
                            // Position cursor on currently playing song if in this album
                            String currentPath = audioApp.getSongPath(audioApp.currentIndex);
                            for (int si = 0; si < (int)g_albumSongOffsets.size(); si++) {
                                if (getAlbumSongPath(si) == currentPath) {
                                    g_albumSongCursor = si;
                                    if (g_albumSongCursor >= MAX_VISIBLE_ROWS)
                                        g_albumSongScrollOffset = g_albumSongCursor - MAX_VISIBLE_ROWS / 2;
                                    break;
                                }
                            }
                            currentState = UI_ALBUM_SONGS;
                            UIManager::drawAlbumSongs();
                        }
                    }
                }
                else if (M5Cardputer.Keyboard.isKeyPressed('e') || M5Cardputer.Keyboard.isKeyPressed('q')) {
                    bool expand = M5Cardputer.Keyboard.isKeyPressed('e');
                    // Save which artist the cursor is on
                    int savedArtistIdx = -1;
                    if (g_sidebarCursor >= 0 && g_sidebarCursor < (int)g_sidebarRows.size()) {
                        SidebarRow& r = g_sidebarRows[g_sidebarCursor];
                        savedArtistIdx = r.isArtist ? r.dataIdx : ((r.dataIdx >= 0 && r.dataIdx < (int)g_albumArtistMap.size()) ? g_albumArtistMap[r.dataIdx] : -1);
                    }
                    for (size_t i = 0; i < g_artistExpanded.size(); i++) g_artistExpanded[i] = expand;
                    rebuildSidebarRows();
                    // Reposition cursor on the same artist
                    for (int i = 0; i < (int)g_sidebarRows.size(); i++) {
                        if (g_sidebarRows[i].isArtist && g_sidebarRows[i].dataIdx == savedArtistIdx) { g_sidebarCursor = i; break; }
                    }
                    if (g_sidebarCursor >= (int)g_sidebarRows.size()) g_sidebarCursor = max(0, (int)g_sidebarRows.size() - 1);
                    g_sidebarScrollOffset = max(0, min(g_sidebarCursor - MAX_VISIBLE_ROWS / 2, (int)g_sidebarRows.size() - MAX_VISIBLE_ROWS));
                    UIManager::drawPlaylist();
                }
                else if (M5Cardputer.Keyboard.isKeyPressed('p')) { audioApp.togglePause(); UIManager::drawNowPlaying(); }
                else if (M5Cardputer.Keyboard.isKeyPressed('n')) { audioApp.next(); }
                else if (M5Cardputer.Keyboard.isKeyPressed('b')) { audioApp.prev(); }
                else if (M5Cardputer.Keyboard.isKeyPressed('s')) {
                    g_searchQuery = ""; g_searchResults.clear(); g_searchCursor = 0; g_searchScrollOffset = 0;
                    currentState = UI_SEARCH; UIManager::drawSearch();
                }
                else if (M5Cardputer.Keyboard.isKeyPressed('f')) {
                    // Cycle: OFF -> ALBUM -> ARTIST -> GLOBAL -> OFF
                    audioApp.shuffleMode = (ShuffleMode)((audioApp.shuffleMode + 1) % 4);
                    if (audioApp.shuffleMode != SHUF_OFF) {
                        int curVal = (audioApp.shuffleMode == SHUF_ALBUM) ? g_albumPlaybackIndex : audioApp.currentIndex;
                        buildShuffleQueue(audioApp.shuffleMode, curVal, audioApp.songOffsets.size());
                    } else {
                        g_shuffleQueue.clear(); g_shufflePos = 0;
                    }
                    UIManager::drawNowPlaying();
                }
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

            case UI_TRACK_INFO:
                if (M5Cardputer.Keyboard.isKeyPressed('t') || M5Cardputer.Keyboard.isKeyPressed('`')) {
                    currentState = UI_PLAYER; UIManager::drawBaseUI();
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
                } else if (status.tab) {
                    // Tab: cycle keyboard layout
                    userSettings.kbLayoutIndex = (userSettings.kbLayoutIndex + 1) % NUM_LAYOUTS;
                    ConfigManager::save();
                    redraw = true;
                } else if (status.del && g_searchQuery.length() > 0) {
                    removeLastUtf8Char(g_searchQuery);
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
                        g_albumPlaybackActive = false;
                        userSettings.lastAlbumIdx = -1;
                        audioApp.play(songIdx);
                        currentState = UI_PLAYER;
                        UIManager::drawBaseUI();
                    }
                } else {
                    for (char c : status.word) {
                        LayoutMapFn mapper = g_layouts[userSettings.kbLayoutIndex];
                        if (mapper) {
                            const char* mapped = mapper(c);
                            if (mapped) g_searchQuery += mapped;
                            else g_searchQuery += c;
                        } else {
                            g_searchQuery += c;
                        }
                        redraw = true;
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
                {
                // Auto-clear digit input after timeout
                if (g_trackNumInput.length() > 0 && millis() - g_trackNumLastInput > TRACK_NUM_TIMEOUT) {
                    g_trackNumInput = "";
                    UIManager::drawAlbumSongsFooter(); // restore normal footer
                }

                auto status = M5Cardputer.Keyboard.keysState();

                if (M5Cardputer.Keyboard.isKeyPressed('`')) {
                    g_trackNumInput = "";
                    currentState = UI_PLAYER;
                    UIManager::drawBaseUI();
                }
                else if (status.del) {
                    if (g_trackNumInput.length() > 0) {
                        g_trackNumInput = "";
                        UIManager::drawAlbumSongsFooter();
                    }
                }
                else if (M5Cardputer.Keyboard.isKeyPressed(';')) {
                    g_trackNumInput = "";
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
                    g_trackNumInput = "";
                    if (g_albumSongOffsets.size() > 0) {
                        g_albumSongCursor++;
                        if (g_albumSongCursor >= (int)g_albumSongOffsets.size()) { g_albumSongCursor = 0; g_albumSongScrollOffset = 0; }
                        if (g_albumSongCursor >= g_albumSongScrollOffset + MAX_VISIBLE_ROWS) g_albumSongScrollOffset++;
                    }
                    UIManager::drawAlbumSongsList();
                }
                else if (status.enter) {
                    g_trackNumInput = "";
                    if (g_albumSongOffsets.size() > 0) {
                        String songPath = getAlbumSongPath(g_albumSongCursor);
                        int globalIdx = findSongInPlaylist(songPath);
                        if (globalIdx >= 0) {
                            audioApp.play(globalIdx);
                            g_albumPlaybackActive = true;
                            g_albumPlaybackIndex = g_albumSongCursor;
                            if (audioApp.shuffleMode == SHUF_ALBUM) buildShuffleQueue(SHUF_ALBUM, g_albumPlaybackIndex);
                            userSettings.lastAlbumIdx = g_currentAlbumIdx;
                            userSettings.lastAlbumTrack = g_albumPlaybackIndex;
                            ConfigManager::save(audioApp.id3 ? audioApp.id3->getPos() : 0, audioApp.currentIndex);
                        }
                        currentState = UI_PLAYER;
                        UIManager::drawBaseUI();
                    }
                }
                else {
                    // Check for digit input (max 3 digits, then restart)
                    bool digitEntered = false;
                    for (char c : status.word) {
                        if (c >= '0' && c <= '9') {
                            if (g_trackNumInput.length() >= 3) g_trackNumInput = "";
                            g_trackNumInput += c;
                            g_trackNumLastInput = millis();
                            digitEntered = true;
                        }
                    }
                    if (digitEntered) {
                        applyTrackNumInput();
                        UIManager::drawAlbumSongsList();
                        UIManager::drawAlbumSongsFooter();
                    }
                }
                }
                break;
        }
    }
}
