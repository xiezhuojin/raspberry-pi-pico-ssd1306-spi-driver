#ifndef SSD1306_H
#define SSD1306_H

#include "hardware/spi.h"

// ssd1306 commands, taken from adafruit library
#define SSD1306_MEMORYMODE 0x20          ///< See datasheet
#define SSD1306_COLUMNADDR 0x21          ///< See datasheet
#define SSD1306_PAGEADDR 0x22            ///< See datasheet
#define SSD1306_SETCONTRAST 0x81         ///< See datasheet
#define SSD1306_CHARGEPUMP 0x8D          ///< See datasheet
#define SSD1306_SEGREMAP 0xA0            ///< See datasheet
#define SSD1306_DISPLAYALLON_RESUME 0xA4 ///< See datasheet
#define SSD1306_DISPLAYALLON 0xA5        ///< Not currently used
#define SSD1306_NORMALDISPLAY 0xA6       ///< See datasheet
#define SSD1306_INVERTDISPLAY 0xA7       ///< See datasheet
#define SSD1306_SETMULTIPLEX 0xA8        ///< See datasheet
#define SSD1306_DISPLAYOFF 0xAE          ///< See datasheet
#define SSD1306_DISPLAYON 0xAF           ///< See datasheet
#define SSD1306_COMSCANINC 0xC0          ///< Not currently used
#define SSD1306_COMSCANDEC 0xC8          ///< See datasheet
#define SSD1306_SETDISPLAYOFFSET 0xD3    ///< See datasheet
#define SSD1306_SETDISPLAYCLOCKDIV 0xD5  ///< See datasheet
#define SSD1306_SETPRECHARGE 0xD9        ///< See datasheet
#define SSD1306_SETCOMPINS 0xDA          ///< See datasheet
#define SSD1306_SETVCOMDETECT 0xDB       ///< See datasheet

#define SSD1306_SETLOWCOLUMN 0x00  ///< Not currently used
#define SSD1306_SETHIGHCOLUMN 0x10 ///< Not currently used
#define SSD1306_SETSTARTLINE 0x40  ///< See datasheet

#define SSD1306_EXTERNALVCC 0x01  ///< External display voltage source
#define SSD1306_SWITCHCAPVCC 0x02 ///< Gen. display voltage from 3.3V

#define SSD1306_RIGHT_HORIZONTAL_SCROLL 0x26              ///< Init rt scroll
#define SSD1306_LEFT_HORIZONTAL_SCROLL 0x27               ///< Init left scroll
#define SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL 0x29 ///< Init diag scroll
#define SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL 0x2A  ///< Init diag scroll
#define SSD1306_DEACTIVATE_SCROLL 0x2E                    ///< Stop scroll
#define SSD1306_ACTIVATE_SCROLL 0x2F                      ///< Start scroll
#define SSD1306_SET_VERTICAL_SCROLL_AREA 0xA3             ///< Set scroll range


enum SSD1306PixelColor {
    SSD1306_COLOR_OFF = 0,
    SSD1306_COLOR_ON = 1,
    SSD1306_COLOR_INVERTED = 2
};

typedef struct
{
    uint8_t* buffer;        // display buffer

    uint8_t width;			// width of display
    uint8_t height;		    // height of display
    spi_inst_t* spi;		// pico spi instance
    uint32_t baudrate;		// spi baudrate in Hz

    uint8_t sclk_pin;		// spi clock pin
    uint8_t mosi_pin;		// spi tx pin
    uint8_t cs_pin;		    // chip select pin
    uint8_t dc_pin;		    // data/command pin
    uint8_t rst_pin;		// reset pin
} SSD1306;

#define ssd1306_buffer_size(width, height) ((width) * (((height) + 7) / 8))
extern void ssd1306_init(SSD1306 *ssd1306, uint8_t width, uint8_t height, 
    spi_inst_t *spi, uint32_t baudrate, uint8_t sclk_pin, uint8_t mosi_pin, 
    uint8_t dc_pin, uint8_t rst_pin, uint8_t cs_pin, uint8_t *buffer);
extern void ssd1306_fini(SSD1306 *ssd1306);

extern void ssd1306_start(SSD1306 *ssd1306);

extern void ssd1306_send_command(SSD1306 *ssd1306, uint8_t command);
extern void ssd1306_send_commands(SSD1306 *ssd1306, const uint8_t* commands, size_t size);

extern void ssd1306_draw_pixel(SSD1306 *ssd1306, int16_t x, int16_t y, uint8_t color);
extern void ssd1306_update(SSD1306 *ssd1306);
extern void ssd1306_clear(SSD1306 *ssd1306);

#endif