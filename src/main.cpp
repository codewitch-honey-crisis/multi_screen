#include <Arduino.h>
#include <tft_io.hpp>
#include <ili9341.hpp>
#include <ssd1306.hpp>
#include <gfx_cpp14.hpp>
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

using ili9341_bus_t = tft_spi<VSPI,LCD1_CS>;
using ssd1306_bus_t = tft_i2c<>;

using screen1_t = ili9341<LCD1_DC,LCD1_RST,LCD1_BL,ili9341_bus_t,LCD1_ROTATION,LCD1_BL_HIGH,LCD1_WRITE_SPEED,LCD1_READ_SPEED>;
using screen2_t = ssd1306<LCD2_WIDTH,LCD2_HEIGHT,ssd1306_bus_t,LCD2_ROTATION,LCD2_BIT_DEPTH,LCD2_ADDRESS,LCD2_3_3v,LCD2_WRITE_SPEED>;

using color1_t = color<typename screen1_t::pixel_type>;
using color2_t = color<typename screen2_t::pixel_type>;

screen1_t screen1;
screen2_t screen2;
int frame;
const char* text = "ESP32";
// put your main code here, to run repeatedly:
auto fade_px = color<rgba_pixel<32>>::black;

void setup() {
  Serial.begin(115200);
  frame = 0;
  fade_px.template channelr<channel_name::A>(.25);
  screen1.fill(screen1.bounds(),color1_t::white);
  screen2.fill(screen2.bounds(),color2_t::white);

}

void loop() {
  if(!frame) {
    const float text_scale1 = DEFTONE_ttf.scale(80);
    const ssize16 text_size1 = DEFTONE_ttf.measure_text({32767,32767},{0,0},text,text_scale1);
    const srect16 text_rect1 = text_size1.bounds().center((srect16)screen1.bounds());
    
    image_jpg_stream.seek(0);
    size16 isz;
    if(gfx_result::success==jpeg_image::dimensions(&image_jpg_stream,&isz)) {
      image_jpg_stream.seek(0);
      draw::image(screen1,isz.bounds().center(screen1.bounds()),&image_jpg_stream);
      draw::text(screen1,text_rect1,{0,0},text,DEFTONE_ttf,text_scale1, color2_t::black,color2_t::white,true);
    }
    const float text_scale2 = DEFTONE_ttf.scale(30);
    const ssize16 text_size2 = DEFTONE_ttf.measure_text({32767,32767},{0,0},text,text_scale2);
    const srect16 text_rect2 = text_size2.bounds().center((srect16)screen2.bounds());

    image3_jpg_stream.seek(0);
    if(gfx_result::success==jpeg_image::dimensions(&image3_jpg_stream,&isz)) {
      image3_jpg_stream.seek(0);
      draw::suspend(screen2);
      draw::image(screen2,isz.bounds().center(screen2.bounds()),&image3_jpg_stream);
      draw::text(screen2,text_rect2,{0,0},text,DEFTONE_ttf,text_scale2, color2_t::black,color2_t::white,true,true);
      draw::resume(screen2);
    }    
  }
  if(frame<20) {
    draw::filled_rectangle(screen2,screen2.bounds(),fade_px);
  } else if(frame<35) {
    draw::filled_rectangle(screen1,screen1.bounds(),fade_px);
  
  }
  if(frame<35) {
    ++frame;
  } else {
    frame = 0;
  }
    
}