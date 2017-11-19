/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org/>
*/

#include <stdint.h>
#include "SSD1306.h"

#ifdef SIMULATOR
#include "simulator/I2C.h"
#else
#include "I2C.h"
#endif

SSD1306::SSD1306() {
    i2c.init(SSD1306_DEFAULT_ADDRESS);

    // Turn display off
    sendCommand(SSD1306_DISPLAYOFF);

    sendCommand(SSD1306_SETDISPLAYCLOCKDIV);
    sendCommand(0x80);

    sendCommand(SSD1306_SETMULTIPLEX);
    sendCommand(0x3F);
    
    sendCommand(SSD1306_SETDISPLAYOFFSET);
    sendCommand(0x00);
    
    sendCommand(SSD1306_SETSTARTLINE | 0x00);
    
    // We use internal charge pump
    sendCommand(SSD1306_CHARGEPUMP);
    sendCommand(0x14);
    
    // Horizontal memory mode
    sendCommand(SSD1306_MEMORYMODE);
    sendCommand(0x00);
    
    sendCommand(SSD1306_SEGREMAP | 0x1);

    sendCommand(SSD1306_COMSCANDEC);

    sendCommand(SSD1306_SETCOMPINS);
    sendCommand(0x12);

    // Max contrast
    sendCommand(SSD1306_SETCONTRAST);
    sendCommand(0xCF);

    sendCommand(SSD1306_SETPRECHARGE);
    sendCommand(0xF1);

    sendCommand(SSD1306_SETVCOMDETECT);
    sendCommand(0x40);

    sendCommand(SSD1306_DISPLAYALLON_RESUME);

    // Non-inverted display
    sendCommand(SSD1306_NORMALDISPLAY);

    // Turn display back on
    sendCommand(SSD1306_DISPLAYON);
}

void SSD1306::sendCommand(uint8_t command) {
    i2c.start();
    i2c.write(0x00);
    i2c.write(command);
    i2c.stop();
}

void SSD1306::invert(uint8_t inverted) {
    if (inverted) {
        sendCommand(SSD1306_INVERTDISPLAY);
    } else {
        sendCommand(SSD1306_NORMALDISPLAY);
    }
}

void SSD1306::sendFramebuffer(uint8_t *buffer) {
    sendCommand(SSD1306_COLUMNADDR);
    sendCommand(0x00);
    sendCommand(0x7F);

    sendCommand(SSD1306_PAGEADDR);
    sendCommand(0x00);
    sendCommand(0x07);

    // We have to send the buffer as 16 bytes packets
    // Our buffer is 1024 bytes long, 1024/16 = 64
    // We have to send 64 packets
    for (uint8_t packet = 0; packet < 64; packet++) { //*** change to 32? was 64
        i2c.start();
        i2c.write(0x40);
        for (uint8_t packet_byte = 0; packet_byte < 16; ++packet_byte) {
            i2c.write(buffer[packet*16+packet_byte]);
        }
        i2c.stop();
    }
}

// Draw a character
// void SSD1306::drawChar(int16_t x, int16_t y, unsigned char c, uint16_t bg, uint8_t size) {
// 
// 	if(!gfxFont) { // 'Classic' built-in font
// 
// 		if((x >= _width)            || // Clip right
// 		(y >= _height)           || // Clip bottom
// 		((x + 6 * size - 1) < 0) || // Clip left
// 		((y + 8 * size - 1) < 0))   // Clip top
// 		return;
// 
// 		if(!_cp437 && (c >= 176)) c++; // Handle 'classic' charset behavior
// 
// 		startWrite();
// 		for(int8_t i=0; i<5; i++ ) { // Char bitmap = 5 columns
// 			uint8_t line = pgm_read_byte(&font[c * 5 + i]);
// 			for(int8_t j=0; j<8; j++, line >>= 1) {
// 				if(line & 1) {
// 					if(size == 1)
// 					writePixel(x+i, y+j, color);
// 					else
// 					writeFillRect(x+i*size, y+j*size, size, size, color);
// 					} else if(bg != color) {
// 					if(size == 1)
// 					writePixel(x+i, y+j, bg);
// 					else
// 					writeFillRect(x+i*size, y+j*size, size, size, bg);
// 				}
// 			}
// 		}
// 		if(bg != color) { // If opaque, draw vertical line for last column
// 			if(size == 1) writeFastVLine(x+5, y, 8, bg);
// 			else          writeFillRect(x+5*size, y, size, 8*size, bg);
// 		}
// 		endWrite();
// 
// 		} else { // Custom font
// 
// 		// Character is assumed previously filtered by write() to eliminate
// 		// newlines, returns, non-printable characters, etc.  Calling
// 		// drawChar() directly with 'bad' characters of font may cause mayhem!
// 
// 		c -= (uint8_t)pgm_read_byte(&gfxFont->first);
// 		GFXglyph *glyph  = &(((GFXglyph *)pgm_read_pointer(&gfxFont->glyph))[c]);
// 		uint8_t  *bitmap = (uint8_t *)pgm_read_pointer(&gfxFont->bitmap);
// 
// 		uint16_t bo = pgm_read_word(&glyph->bitmapOffset);
// 		uint8_t  w  = pgm_read_byte(&glyph->width),
// 		h  = pgm_read_byte(&glyph->height);
// 		int8_t   xo = pgm_read_byte(&glyph->xOffset),
// 		yo = pgm_read_byte(&glyph->yOffset);
// 		uint8_t  xx, yy, bits = 0, bit = 0;
// 		int16_t  xo16 = 0, yo16 = 0;
// 
// 		if(size > 1) {
// 			xo16 = xo;
// 			yo16 = yo;
// 		}
// 
// 		// Todo: Add character clipping here
// 
// 		// NOTE: THERE IS NO 'BACKGROUND' COLOR OPTION ON CUSTOM FONTS.
// 		// THIS IS ON PURPOSE AND BY DESIGN.  The background color feature
// 		// has typically been used with the 'classic' font to overwrite old
// 		// screen contents with new data.  This ONLY works because the
// 		// characters are a uniform size; it's not a sensible thing to do with
// 		// proportionally-spaced fonts with glyphs of varying sizes (and that
// 		// may overlap).  To replace previously-drawn text when using a custom
// 		// font, use the getTextBounds() function to determine the smallest
// 		// rectangle encompassing a string, erase the area with fillRect(),
// 		// then draw new text.  This WILL infortunately 'blink' the text, but
// 		// is unavoidable.  Drawing 'background' pixels will NOT fix this,
// 		// only creates a new set of problems.  Have an idea to work around
// 		// this (a canvas object type for MCUs that can afford the RAM and
// 		// displays supporting setAddrWindow() and pushColors()), but haven't
// 		// implemented this yet.
// 
// 		startWrite();
// 		for(yy=0; yy<h; yy++) {
// 			for(xx=0; xx<w; xx++) {
// 				if(!(bit++ & 7)) {
// 					bits = pgm_read_byte(&bitmap[bo++]);
// 				}
// 				if(bits & 0x80) {
// 					if(size == 1) {
// 						writePixel(x+xo+xx, y+yo+yy, color);
// 						} else {
// 						writeFillRect(x+(xo16+xx)*size, y+(yo16+yy)*size,
// 						size, size, color);
// 					}
// 				}
// 				bits <<= 1;
// 			}
// 		}
// 		endWrite();
// 
// 	} // End classic vs custom font
// }