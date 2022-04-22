#include <Arduino.h>

#include <gfx_cpp14.hpp>
#include <ili9341.hpp>
#include <ssd1306.hpp>
#include <tft_io.hpp>
// our truetype font
#include "DEFTONE.hpp"
#include "image.h"
#include "image3.h"

using namespace arduino;
using namespace gfx;

// wiring is as follows for the ILI9341 display
// MOSI 23
// MISO 19
// SCLK 18
// VCC 3.3v
// see below for additional pins:
#define LCD1_CS 5
#define LCD1_DC 2
#define LCD1_RST 4
#define LCD1_BL 14

#define LCD1_WRITE_SPEED 400
#define LCD1_READ_SPEED 200
// you may need to change this to 1 if your screen is upside down
#define LCD1_ROTATION 3
// if you don't see any backlight, or any display
// try changing this to false
#define LCD1_BL_HIGH true

// wiring is as follows for the SSD1306 display
// SCL 22
// SDA 21
// VCC 3.3v
#define LCD2_WIDTH 128
#define LCD2_HEIGHT 64
#define LCD2_3_3v true
#define LCD2_ADDRESS 0x3C
#define LCD2_WRITE_SPEED 800
#define LCD2_ROTATION 3
#define LCD2_BIT_DEPTH 8

using ili9341_bus_t = tft_spi<VSPI, LCD1_CS>;
using ssd1306_bus_t = tft_i2c<>;

using screen1_t = ili9341<LCD1_DC, LCD1_RST, LCD1_BL, ili9341_bus_t, LCD1_ROTATION, LCD1_BL_HIGH, LCD1_WRITE_SPEED, LCD1_READ_SPEED>;
using screen2_t = ssd1306<LCD2_WIDTH, LCD2_HEIGHT, ssd1306_bus_t, LCD2_ROTATION, LCD2_BIT_DEPTH, LCD2_ADDRESS, LCD2_3_3v, LCD2_WRITE_SPEED>;

// for easy access to x11 colors in the screen's native format
using color1_t = color<typename screen1_t::pixel_type>;
using color2_t = color<typename screen2_t::pixel_type>;

// declare the screens
screen1_t screen1;
screen2_t screen2;

// frame counter
int frame;

// title text
const char* text = "ESP32";

template <typename Destination>
void draw_alpha(Destination& lcd) {
    randomSeed(millis());

    rgba_pixel<32> px;
    spoint16 tpa[3];
    const uint16_t sw =
        min(lcd.dimensions().width, lcd.dimensions().height) / 4;
    px.channel<channel_name::R>((rand() % 256));
    px.channel<channel_name::G>((rand() % 256));
    px.channel<channel_name::B>((rand() % 256));
    px.channel<channel_name::A>(50 + rand() % 156);
    srect16 sr(0, 0, rand() % sw + sw, rand() % sw + sw);
    sr.offset_inplace(rand() % (lcd.dimensions().width - sr.width()),
                      rand() % (lcd.dimensions().height - sr.height()));
    switch (rand() % 4) {
        case 0:
            draw::filled_rectangle(lcd, sr, px);
            break;
        case 1:
            draw::filled_rounded_rectangle(lcd, sr, .1, px);
            break;
        case 2:
            draw::filled_ellipse(lcd, sr, px);
            break;
        case 3:
            tpa[0] = {int16_t(((sr.x2 - sr.x1) / 2) + sr.x1), sr.y1};
            tpa[1] = {sr.x2, sr.y2};
            tpa[2] = {sr.x1, sr.y2};
            spath16 path(3, tpa);
            draw::filled_polygon(lcd, path, px);
            break;
    }
}

void setup() {
    Serial.begin(115200);
    frame = 0;
    screen1.fill(screen1.bounds(), color1_t::white);
    screen2.fill(screen2.bounds(), color2_t::white);
}

void loop() {
    if (!frame) {
        const float text_scale1 = DEFTONE_ttf.scale(80);
        const ssize16 text_size1 = DEFTONE_ttf.measure_text({32767, 32767}, {0, 0}, text, text_scale1);
        const srect16 text_rect1 = text_size1.bounds().center((srect16)screen1.bounds());

        image_jpg_stream.seek(0);
        size16 isz;
        if (gfx_result::success == jpeg_image::dimensions(&image_jpg_stream, &isz)) {
            image_jpg_stream.seek(0);
            draw::image(screen1, isz.bounds().center(screen1.bounds()), &image_jpg_stream);
            draw::text(screen1, text_rect1, {0, 0}, text, DEFTONE_ttf, text_scale1, color2_t::black, color2_t::white, true);
        }
        const float text_scale2 = DEFTONE_ttf.scale(30);
        const ssize16 text_size2 = DEFTONE_ttf.measure_text({32767, 32767}, {0, 0}, text, text_scale2);
        const srect16 text_rect2 = text_size2.bounds().center((srect16)screen2.bounds());

        image3_jpg_stream.seek(0);
        if (gfx_result::success == jpeg_image::dimensions(&image3_jpg_stream, &isz)) {
            image3_jpg_stream.seek(0);
            draw::suspend(screen2);
            draw::image(screen2, isz.bounds().center(screen2.bounds()), &image3_jpg_stream);
            draw::text(screen2, text_rect2, {0, 0}, text, DEFTONE_ttf, text_scale2, color2_t::black, color2_t::white, true, true);
            draw::resume(screen2);
        }
    }
    if (frame & 1) {
        draw_alpha(screen2);
    } else {
        draw_alpha(screen1);
    }
    if (frame < 60) {
        ++frame;
    } else {
        frame = 0;
    }
}