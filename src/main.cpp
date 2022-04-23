#include <Arduino.h>

// the TFT IO bus library
// used by the drivers below
#include <tft_io.hpp>

// the ILI9341 driver
#include <ili9341.hpp>
// the SSD1306 driver
#include <ssd1306.hpp>

// GFX (for C++14)
#include <gfx_cpp14.hpp>

// our truetype font
#include "DEFTONE.hpp"
// color jpg image
#include "image.h"
// b&w jpg image
#include "image3.h"

// import driver namespace
using namespace arduino;
// import GFX namespace
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

#define LCD1_WRITE_SPEED 400 // 400% of 10MHz = 40MHz
#define LCD1_READ_SPEED 200 // 200% of 10MHz = 20MHz
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
// if your screen isn't working, change
// this to 400:
#define LCD2_WRITE_SPEED 800 // 800% of 100KHz = 800KHz
// change this to 1 if your screen is upside down
#define LCD2_ROTATION 3
#define LCD2_BIT_DEPTH 8

using ili9341_bus_t = tft_spi<VSPI, LCD1_CS>;
using ssd1306_bus_t = tft_i2c<>;

using screen1_t = ili9341<LCD1_DC, 
                          LCD1_RST, 
                          LCD1_BL, 
                          ili9341_bus_t, 
                          LCD1_ROTATION, 
                          LCD1_BL_HIGH, 
                          LCD1_WRITE_SPEED, 
                          LCD1_READ_SPEED>;

using screen2_t = ssd1306<LCD2_WIDTH, 
                          LCD2_HEIGHT, 
                          ssd1306_bus_t, 
                          LCD2_ROTATION, 
                          LCD2_BIT_DEPTH, 
                          LCD2_ADDRESS, 
                          LCD2_3_3v, 
                          LCD2_WRITE_SPEED>;

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

// draw a random alpha blended shape
template <typename Destination>
void draw_alpha(Destination& lcd) {
    // randomize
    randomSeed(millis());
    // declare a pixel with an alpha channel
    rgba_pixel<32> px;
    // points for a triangle
    spoint16 tpa[3];
    // maximum shape width
    const uint16_t sw =
        min(lcd.dimensions().width, lcd.dimensions().height) / 4;
    // set each channel to a random value
    // note that the alpha channel is ranged differently
    px.channel<channel_name::R>((rand() % 256));
    px.channel<channel_name::G>((rand() % 256));
    px.channel<channel_name::B>((rand() % 256));
    px.channel<channel_name::A>(50 + rand() % 156);
    // create a rectangle of a random size bounding the shape 
    srect16 sr(0, 0, rand() % sw + sw, rand() % sw + sw);
    // offset it to a random location
    sr.offset_inplace(rand() % (lcd.dimensions().width - sr.width()),
                      rand() % (lcd.dimensions().height - sr.height()));
    // choose a random shape to draw
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
            // create a triangle polygon
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
    // fill the screens just so we know they're alive
    // (not really necessary)
    screen1.fill(screen1.bounds(), color1_t::white);
    screen2.fill(screen2.bounds(), color2_t::white);
}

void loop() {
    if (!frame) { // first frame
        // prepare to draw the text for screen 1
        const float text_scale1 = DEFTONE_ttf.scale(80);
        // measure the text
        const ssize16 text_size1 = 
          DEFTONE_ttf.measure_text({32767, 32767}, 
                                    {0, 0}, 
                                    text, 
                                    text_scale1);
        // center it
        const srect16 text_rect1 = 
          text_size1.bounds().center((srect16)screen1.bounds());
        // prepare to draw the image for screen 1
        // ensure stream is at beginning since JPG loading doesn't seek
        image_jpg_stream.seek(0);
        size16 isz;
        if (gfx_result::success == 
              jpeg_image::dimensions(&image_jpg_stream, &isz)) {
            // start back at the beginning
            image_jpg_stream.seek(0);
            // draw them both
            draw::image(screen1, 
                        isz.bounds().center(screen1.bounds()), 
                        &image_jpg_stream);
            draw::text(screen1, 
                      text_rect1, 
                      {0, 0}, 
                      text, 
                      DEFTONE_ttf, 
                      text_scale1, 
                      color2_t::black, 
                      color2_t::white, 
                      true);
        }
        // prepare to draw the text for screen 2
        const float text_scale2 = DEFTONE_ttf.scale(30);
        // measure the text
        const ssize16 text_size2 = 
          DEFTONE_ttf.measure_text({32767, 32767}, 
                                    {0, 0}, 
                                    text, 
                                    text_scale2);
        // center it
        const srect16 text_rect2 = 
          text_size2.bounds().center((srect16)screen2.bounds());
        // prepare to draw the image for screen 1
        // ensure stream is at beginning since JPG loading doesn't seek
        image3_jpg_stream.seek(0);
        if (gfx_result::success == 
              jpeg_image::dimensions(&image3_jpg_stream, &isz)) {
            // start back at the beginning
            image3_jpg_stream.seek(0);
            // suspend so we don't see the JPG being painted
            draw::suspend(screen2);
            // draw them both
            draw::image(screen2, 
                        isz.bounds().center(screen2.bounds()), 
                        &image3_jpg_stream);
            draw::text(screen2, 
                        text_rect2, 
                        {0, 0}, 
                        text, 
                        DEFTONE_ttf, 
                        text_scale2, 
                        color2_t::black, 
                        color2_t::white, 
                        true, 
                        true);
            draw::resume(screen2);
        }
        delay(2000);
    }
    // on even frames we draw to screen 1
    // on odd frames we draw to screen 2
    if (frame & 1) {
        draw_alpha(screen2);
    } else {
        draw_alpha(screen1);
    }
    // once we have about 30 per screen start over
    if (frame < 60) {
        ++frame;
    } else {
        // before we start over copy the contents
        // of screen 2 to the center of screen 1
        rect16 sr = screen2.bounds().center(screen1.bounds());
        draw::bitmap(screen1,sr,screen2,screen2.bounds());
        delay(5000);
        frame = 0;
    }
}