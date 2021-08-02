#include "ssd1306.h"

#include <string.h>
#include "pico/stdlib.h"

void ssd1306_init(SSD1306 *ssd1306, uint8_t width, uint8_t height, 
    spi_inst_t *spi, uint32_t baudrate, uint8_t sclk_pin, uint8_t mosi_pin, 
    uint8_t dc_pin, uint8_t rst_pin, uint8_t cs_pin, uint8_t *buffer)
{
    ssd1306->width = width;
    ssd1306->height = height;
    ssd1306->spi = spi;
    ssd1306->baudrate = baudrate;
    ssd1306->sclk_pin = sclk_pin;
    ssd1306->mosi_pin = mosi_pin;
    ssd1306->dc_pin = dc_pin;
    ssd1306->rst_pin = rst_pin;
    ssd1306->cs_pin = cs_pin;
    ssd1306->buffer = buffer;
    ssd1306_clear(ssd1306);
}

void ssd1306_fini(SSD1306 *ssd1306) {
    ssd1306->buffer = NULL;
}

void ssd1306_send_init_commands(SSD1306 *ssd1306);

void ssd1306_start(SSD1306 *ssd1306) {
    // init all gpio pins (enable I/O and set func to GPIO_FUNC_SIO)
    gpio_init_mask(1 << ssd1306->sclk_pin | 1 << ssd1306->mosi_pin | 1 << ssd1306->cs_pin | 1 << ssd1306->dc_pin | 1 << ssd1306->rst_pin);

    // rst, leave it low (reset) first
    gpio_set_dir(ssd1306->rst_pin, GPIO_OUT);
    gpio_put(ssd1306->rst_pin, 0);

    // configure spi peripheral
    spi_init(ssd1306->spi, ssd1306->baudrate);
    // spi_init calls this internally
    // spi_set_format(ssd1306->spi, /*data_bits*/ 8u, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    // spi_init also sets master mode by default

    // configure mosi and sclk pins to pull from spi peripheral
    gpio_set_function(ssd1306->mosi_pin, GPIO_FUNC_SPI);
    // gpio_set_function(ssd1306->cs_pin,   GPIO_FUNC_SIO);  // done in gpio_init
    gpio_set_function(ssd1306->sclk_pin, GPIO_FUNC_SPI);

    // cs, following example. might be able to connect this to spi peripheral...
    gpio_set_dir(ssd1306->cs_pin, GPIO_OUT);
    // just always set chip select to low, which means we're using the device
    gpio_put(ssd1306->cs_pin, 0);

    // data/command pin (dc), used to switch between data and command output
    gpio_set_dir(ssd1306->dc_pin, GPIO_OUT);
    gpio_put(ssd1306->dc_pin, 0);
    
    sleep_ms(1);
	ssd1306_send_init_commands(ssd1306);
}

void ssd1306_send_command(SSD1306 *ssd1306, uint8_t command) {
    gpio_put(ssd1306->dc_pin, 0);  // low for command mode
    spi_write_blocking(ssd1306->spi, &command, 1);
}

void ssd1306_send_commands(SSD1306 *ssd1306, const uint8_t* commands, size_t size) {
    gpio_put(ssd1306->dc_pin, 0);  // low for command mode
    spi_write_blocking(ssd1306->spi, commands, size);
}

void ssd1306_send_init_commands(SSD1306 *ssd1306) {
    // reset the display by pulling rst low then high
    gpio_put(ssd1306->rst_pin, 0);
    sleep_ms(10);
    gpio_put(ssd1306->rst_pin, 1);

    // it's dangerous to go alone, take this:
    // https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf

    // SSD1306_SWITCHCAPVCC to generate from a 3.3v source
    // or SSD1306_EXTERNALVCC for external vcc
    uint8_t vccstate = SSD1306_SWITCHCAPVCC;

    ssd1306_send_command(ssd1306, SSD1306_DISPLAYOFF);

    ssd1306_send_command(ssd1306, SSD1306_SETDISPLAYCLOCKDIV);
    ssd1306_send_command(ssd1306, 0x80); // the suggested ratio 0x80

    ssd1306_send_command(ssd1306, SSD1306_SETMULTIPLEX);
    ssd1306_send_command(ssd1306, ssd1306->height - 1);

    static const uint8_t init2[] = {
        SSD1306_SETDISPLAYOFFSET,  // 0xD3
        0x0,  // no offset
        SSD1306_SETSTARTLINE | 0x0, // line #0
        SSD1306_CHARGEPUMP
    };
    ssd1306_send_commands(ssd1306, init2, sizeof(init2) / sizeof(init2[0]));
    ssd1306_send_command(ssd1306, (vccstate == SSD1306_EXTERNALVCC) ? 0x10 : 0x14);

    static const uint8_t init3[] = {
        SSD1306_MEMORYMODE,  // 0x20
        0x00,  // 0x0 act like ks0108
        SSD1306_SEGREMAP | 0x1,
        SSD1306_COMSCANDEC
    };
    ssd1306_send_commands(ssd1306, init3, sizeof(init3) / sizeof(init3[0]));

    uint8_t comPins = 0x02;
    uint8_t contrast = 0x8F;

    if ((ssd1306->width == 128) && (ssd1306->height == 64)) {
        comPins = 0x12;
        contrast = (vccstate == SSD1306_EXTERNALVCC) ? 0x9F : 0xCF;
    } else {
        // Other screen varieties -- TBD
    }

    ssd1306_send_command(ssd1306, SSD1306_SETCOMPINS);
    ssd1306_send_command(ssd1306, comPins);
    ssd1306_send_command(ssd1306, SSD1306_SETCONTRAST);
    ssd1306_send_command(ssd1306, contrast);

    // This command is used to set the duration of the pre-charge period. The interval is counted in number of DCLK, where RESET equals 2 DCLKs.
    ssd1306_send_command(ssd1306, SSD1306_SETPRECHARGE); // 0xd9
    ssd1306_send_command(ssd1306, (vccstate == SSD1306_EXTERNALVCC) ? 0x22 : 0xF1);

    static const uint8_t init5[] = {
        SSD1306_SETVCOMDETECT,  // 0xDB
        0x40,
        SSD1306_DISPLAYALLON_RESUME,  // 0xA4 - start using gddram
        //SSD1306_DISPLAYALLON,  // 0xA5 - ignore gddram, set everything on
        SSD1306_NORMALDISPLAY,  // 0xA6
        SSD1306_DEACTIVATE_SCROLL,
        SSD1306_DISPLAYON  // Main screen turn on
    };
    ssd1306_send_commands(ssd1306, init5, sizeof(init5) / sizeof(init5[0]));
}

void ssd1306_draw_pixel(SSD1306 *ssd1306, int16_t x, int16_t y, uint8_t color) {
    switch (color) {
        case SSD1306_COLOR_ON:
            ssd1306->buffer[x + (y / 8) * ssd1306->width] |= (1 << (y & 7));
            break;
        case SSD1306_COLOR_OFF:
            ssd1306->buffer[x + (y / 8) * ssd1306->width] &= ~(1 << (y & 7));
            break;
        case SSD1306_COLOR_INVERTED:
            ssd1306->buffer[x + (y / 8) * ssd1306->width] ^= (1 << (y & 7));
            break;
        default:
            break;
    }
}

void ssd1306_update(SSD1306 *ssd1306) {
    static const uint8_t dlist1[] = {
        SSD1306_PAGEADDR,
        0x0,                    // Page start address
        0xFF,                   // Page end (not really, but works here)
        SSD1306_COLUMNADDR,
        0x0
    };  // Column start address

    ssd1306_send_commands(ssd1306, dlist1, sizeof(dlist1) / sizeof(dlist1[0]));
    ssd1306_send_command(ssd1306, ssd1306->width - 1);  // Column end address

    gpio_put(ssd1306->dc_pin, 1);  // bring data/command high since we're sending data now
    spi_write_blocking(ssd1306->spi, ssd1306->buffer, ssd1306_buffer_size(ssd1306->width, ssd1306->height));
}

void ssd1306_clear(SSD1306 *ssd1306) {
	memset(ssd1306->buffer, 0, ssd1306_buffer_size(ssd1306->width, ssd1306->height) * sizeof(uint8_t));
}