#define ILI9488_DRIVER

#define TFT_WIDTH  320
#define TFT_HEIGHT 480

#define TFT_MISO  -1      // Tidak dipakai
#define TFT_MOSI  23
#define TFT_SCLK  18
#define TFT_CS    15
#define TFT_DC    2
#define TFT_RST   4

#define LOAD_GFXFF

#define SPI_FREQUENCY  27000000
#define SPI_READ_FREQUENCY  16000000


#define LOAD_FONT2     // small 16 pixel high font
#define LOAD_FONT4     // medium 26 pixel high font
#define LOAD_FONT6     // large font
#define LOAD_FONT7     // 7-segment font
#define LOAD_FONT8     // large symbol font
#define LOAD_GFXFF     // FreeFonts (grafik)

#define SMOOTH_FONT    // smooth font support


