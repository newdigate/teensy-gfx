/***************************************************
  This is a library for the Adafruit 1.8" SPI display.
  This library works with the Adafruit 1.8" TFT Breakout w/SD card
  ----> http://www.adafruit.com/products/358
  as well as Adafruit raw 1.8" TFT display
  ----> http://www.adafruit.com/products/618

  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!
  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/

#include "View.h"
#include <limits.h>
#include "pins_arduino.h"
#include <cstdint>
#include "cstdlib"
#include <cstdlib>
#include <cstdio>
#include <cmath>

void View::setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    _windowx0 = x0;
    _windowy0 = y0;
    _windowx1 = x1;
    _windowy1 = y1;
}


void View::pushColor(uint16_t color, bool last_pixel)
{

}

//#include "glcdfont.c"
extern "C" const unsigned char glcdfont[];


void View::drawPixel(int16_t x, int16_t y, uint16_t color)
{
    if((x < _displayclipx1) ||(x >= _displayclipx2) || (y < _displayclipy1) || (y >= _displayclipy2)) return;

    x += _originx;
    y += _originy;

    Pixel(x,y, color);
}


void View::drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color)
{
    // Rectangular clipping
    if((x < _displayclipx1) || (x >= _displayclipx2) || (y >= _displayclipy2)) return;
    if(y < _displayclipy1) { h = h - (_displayclipy1 - y); y = _displayclipy1;}
    if((y+h-1) >= _displayclipy2) h = _displayclipy2-y;
    if(h<1) return;

    x+=_originx;
    y+=_originy;

    VLine(x,y, h, color);
}


void View::drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color)
{
    // Rectangular clipping
    if((y < _displayclipy1) || (x >= _displayclipx2) || (y >= _displayclipy2)) return;
    if(x<_displayclipx1) { w = w - (_displayclipx1 - x); x = _displayclipx1; }
    if((x+w-1) >= _displayclipx2)  w = _displayclipx2-x;
    if (w<1) return;

    x+=_originx;
    y+=_originy;

    HLine(x,y,w, color);
}

void View::fillScreen(uint16_t color)
{
    fillRect(0, 0,  _width, _height, color);
}

// fill a rectangle
void View::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{

    // Rectangular clipping (drawChar w/big text requires this)
    if((x >= _displayclipx2) || (y >= _displayclipy2)) return;
    if (((x+w) <= _displayclipx1) || ((y+h) <= _displayclipy1)) return;
    if(x < _displayclipx1) {	w -= (_displayclipx1-x); x = _displayclipx1; 	}
    if(y < _displayclipy1) {	h -= (_displayclipy1 - y); y = _displayclipy1; 	}
    if((x + w - 1) >= _displayclipx2)  w = _displayclipx2  - x;
    if((y + h - 1) >= _displayclipy2) h = _displayclipy2 - y;

    x+=_originx;
    y+=_originy;

    for (int i=x; i < x+w; i++)
        for (int j = y; j < y+h; j++) {
            Pixel(i,j, color);
        }
}

void View::setRotation(uint8_t m)
{
    rotation = m % 4; // can't be higher than 3
    switch (rotation) {
        case 0:
            _width  = _screenWidth;
            _height = _screenHeight;
            _xstart = _colstart;
            _ystart = _rowstart;
            break;
        case 1:
            _height = _screenWidth;
            _width  = _screenHeight;
            _ystart = _colstart;
            _xstart = _rowstart;
            break;
        case 2:
            _width  = _screenWidth;
            _height = _screenHeight;
            _xstart = _colstart;
            // hack to make work on a couple different displays
            _ystart = (_rowstart==0 || _rowstart==32)? 0 : 1;//_rowstart;
            break;
        case 3:
            _width = _screenHeight;
            _height = _screenWidth;
            _ystart = _colstart;
            // hack to make work on a couple different displays
            _xstart = (_rowstart==0 || _rowstart==32)? 0 : 1;//_rowstart;
            break;
    }
    _rot = rotation;	// remember the rotation...

    //Serial.printf("SetRotation(%d) _xstart=%d _ystart=%d _width=%d, _height=%d\n", _rot, _xstart, _ystart, _width, _height);


    setClipRect();
    setOrigin();

    cursor_x = 0;
    cursor_y = 0;
}

void View::setRowColStart(uint16_t x, uint16_t y) {
    _rowstart = x;
    _colstart = y;
    if (_rot != 0xff) setRotation(_rot);
}


void View::invertDisplay(bool i)
{
}

uint16_t View::readPixel(int16_t x, int16_t y)
{
    uint16_t colors = 0;
    readRect(x, y, 1, 1, &colors);
    return colors;
}


// Now lets see if we can read in multiple pixels
void View::readRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t *pcolors)
{
    /*
    // Use our Origin.
    x+=_originx;
    y+=_originy;
    //BUGBUG:: Should add some validation of X and Y

    if (_useFramebuffer) {
        uint16_t * _pfbtft = getFrameBufferPtr();
		uint16_t * pfbPixel_row = &_pfbtft[ y*_width + x];
		for (;h>0; h--) {
			uint16_t * pfbPixel = pfbPixel_row;
			for (int i = 0 ;i < w; i++) {
				*pcolors++ = *pfbPixel++;
			}
			pfbPixel_row += _width;
		}
		return;
	} */
}

// Now lets see if we can writemultiple pixels
void View::writeRect(int16_t x, int16_t y, int16_t w, int16_t h, const uint16_t *pcolors)
{
    x+=_originx;
    y+=_originy;

    uint16_t x_clip_left = 0;  // How many entries at start of colors to skip at start of row
    uint16_t x_clip_right = 0;    // how many color entries to skip at end of row for clipping
    // Rectangular clipping

    // See if the whole thing out of bounds...
    if((x >= _displayclipx2) || (y >= _displayclipy2)) return;
    if (((x+w) <= _displayclipx1) || ((y+h) <= _displayclipy1)) return;

    // In these cases you can not do simple clipping, as we need to synchronize the colors array with the
    // We can clip the height as when we get to the last visible we don't have to go any farther.
    // also maybe starting y as we will advance the color array.
    if(y < _displayclipy1) {
        int dy = (_displayclipy1 - y);
        h -= dy;
        pcolors += (dy*w); // Advance color array to
        y = _displayclipy1;
    }
    if((y + h - 1) >= _displayclipy2) h = _displayclipy2 - y;
    // For X see how many items in color array to skip at start of row and likewise end of row
    if(x < _displayclipx1) {
        x_clip_left = _displayclipx1-x;
        w -= x_clip_left;
        x = _displayclipx1;
    }

    if((x + w - 1) >= _displayclipx2) {
        x_clip_right = w;
        w = _displayclipx2  - x;
        x_clip_right -= w;
    }
/*
    //if (_useFramebuffer) {
        uint16_t * _pfbtft = getFrameBufferPtr();
		uint16_t * pfbPixel_row = &_pfbtft[ y*_width + x];
		for (;h>0; h--) {
			uint16_t * pfbPixel = pfbPixel_row;
			pcolors += x_clip_left;
			for (int i = 0 ;i < w; i++) {
				*pfbPixel++ = *pcolors++;
			}
			pfbPixel_row += _width;
			pcolors += x_clip_right;

		}
		return;
	//}
*/
}

///
///
///
// Draw a rectangle
void View::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{

    drawFastHLine(x, y, w, color);
    drawFastHLine(x, y+h-1, w, color);
    drawFastVLine(x, y, h, color);
    drawFastVLine(x+w-1, y, h, color);
}

// Draw a rounded rectangle
void View::drawRoundRect(int16_t x, int16_t y, int16_t w,
                         int16_t h, int16_t r, uint16_t color) {
    // smarter version
    drawFastHLine(x+r  , y    , w-2*r, color); // Top
    drawFastHLine(x+r  , y+h-1, w-2*r, color); // Bottom
    drawFastVLine(x    , y+r  , h-2*r, color); // Left
    drawFastVLine(x+w-1, y+r  , h-2*r, color); // Right
    // draw four corners
    drawCircleHelper(x+r    , y+r    , r, 1, color);
    drawCircleHelper(x+w-r-1, y+r    , r, 2, color);
    drawCircleHelper(x+w-r-1, y+h-r-1, r, 4, color);
    drawCircleHelper(x+r    , y+h-r-1, r, 8, color);
}

// Fill a rounded rectangle
void View::fillRoundRect(int16_t x, int16_t y, int16_t w,
                         int16_t h, int16_t r, uint16_t color) {
    // smarter version
    fillRect(x+r, y, w-2*r, h, color);

    // draw four corners
    fillCircleHelper(x+w-r-1, y+r, r, 1, h-2*r-1, color);
    fillCircleHelper(x+r    , y+r, r, 2, h-2*r-1, color);
}

// Draw a triangle
void View::drawTriangle(int16_t x0, int16_t y0,
                        int16_t x1, int16_t y1,
                        int16_t x2, int16_t y2, uint16_t color) {
    drawLine(x0, y0, x1, y1, color);
    drawLine(x1, y1, x2, y2, color);
    drawLine(x2, y2, x0, y0, color);
}

// Fill a triangle
void View::fillTriangle (int16_t x0, int16_t y0,
                         int16_t x1, int16_t y1,
                         int16_t x2, int16_t y2, uint16_t color) {

    int16_t a, b, y, last;

    // Sort coordinates by Y order (y2 >= y1 >= y0)
    if (y0 > y1) {
        tgfx_swap(y0, y1); tgfx_swap(x0, x1);
    }
    if (y1 > y2) {
        tgfx_swap(y2, y1); tgfx_swap(x2, x1);
    }
    if (y0 > y1) {
        tgfx_swap(y0, y1); tgfx_swap(x0, x1);
    }

    if(y0 == y2) { // Handle awkward all-on-same-line case as its own thing
        a = b = x0;
        if(x1 < a)      a = x1;
        else if(x1 > b) b = x1;
        if(x2 < a)      a = x2;
        else if(x2 > b) b = x2;
        drawFastHLine(a, y0, b-a+1, color);
        return;
    }

    int32_t
            dx01 = x1 - x0,
            dy01 = y1 - y0,
            dx02 = x2 - x0,
            dy02 = y2 - y0,
            dx12 = x2 - x1,
            dy12 = y2 - y1,
            sa   = 0,
            sb   = 0;

    // For upper part of triangle, find scanline crossings for segments
    // 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
    // is included here (and second loop will be skipped, avoiding a /0
    // error there), otherwise scanline y1 is skipped here and handled
    // in the second loop...which also avoids a /0 error here if y0=y1
    // (flat-topped triangle).
    if(y1 == y2) last = y1;   // Include y1 scanline
    else         last = y1-1; // Skip it

    for(y=y0; y<=last; y++) {
        a   = x0 + sa / dy01;
        b   = x0 + sb / dy02;
        sa += dx01;
        sb += dx02;
        /* longhand:
        a = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
        b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
        */
        if(a > b) tgfx_swap(a, b);
        drawFastHLine(a, y, b-a+1, color);
    }

    // For lower part of triangle, find scanline crossings for segments
    // 0-2 and 1-2.  This loop is skipped if y1=y2.
    sa = dx12 * (y - y1);
    sb = dx02 * (y - y0);
    for(; y<=y2; y++) {
        a   = x1 + sa / dy12;
        b   = x0 + sb / dy02;
        sa += dx12;
        sb += dx02;
        /* longhand:
        a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
        b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
        */
        if(a > b) tgfx_swap(a, b);
        drawFastHLine(a, y, b-a+1, color);
    }
}

// Draw a circle outline
void View::drawCircle(int16_t x0, int16_t y0, int16_t r,
                      uint16_t color) {
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    drawPixel(x0  , y0+r, color);
    drawPixel(x0  , y0-r, color);
    drawPixel(x0+r, y0  , color);
    drawPixel(x0-r, y0  , color);

    while (x<y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        drawPixel(x0 + x, y0 + y, color);
        drawPixel(x0 - x, y0 + y, color);
        drawPixel(x0 + x, y0 - y, color);
        drawPixel(x0 - x, y0 - y, color);
        drawPixel(x0 + y, y0 + x, color);
        drawPixel(x0 - y, y0 + x, color);
        drawPixel(x0 + y, y0 - x, color);
        drawPixel(x0 - y, y0 - x, color);
    }
}

void View::drawCircleHelper(int16_t x0, int16_t y0,
                            int16_t r, uint8_t cornername, uint16_t color) {
    int16_t f     = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x     = 0;
    int16_t y     = r;

    while (x<y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f     += ddF_y;
        }
        x++;
        ddF_x += 2;
        f     += ddF_x;
        if (cornername & 0x4) {
            drawPixel(x0 + x, y0 + y, color);
            drawPixel(x0 + y, y0 + x, color);
        }
        if (cornername & 0x2) {
            drawPixel(x0 + x, y0 - y, color);
            drawPixel(x0 + y, y0 - x, color);
        }
        if (cornername & 0x8) {
            drawPixel(x0 - y, y0 + x, color);
            drawPixel(x0 - x, y0 + y, color);
        }
        if (cornername & 0x1) {
            drawPixel(x0 - y, y0 - x, color);
            drawPixel(x0 - x, y0 - y, color);
        }
    }
}

void View::fillCircle(int16_t x0, int16_t y0, int16_t r,
                      uint16_t color) {
    drawFastVLine(x0, y0-r, 2*r+1, color);
    fillCircleHelper(x0, y0, r, 3, 0, color);
}

// Used to do circles and roundrects
void View::fillCircleHelper(int16_t x0, int16_t y0, int16_t r,
                            uint8_t cornername, int16_t delta, uint16_t color) {

    int16_t f     = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x     = 0;
    int16_t y     = r;

    while (x<y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f     += ddF_y;
        }
        x++;
        ddF_x += 2;
        f     += ddF_x;

        if (cornername & 0x1) {
            drawFastVLine(x0+x, y0-y, 2*y+1+delta, color);
            drawFastVLine(x0+y, y0-x, 2*x+1+delta, color);
        }
        if (cornername & 0x2) {
            drawFastVLine(x0-x, y0-y, 2*y+1+delta, color);
            drawFastVLine(x0-y, y0-x, 2*x+1+delta, color);
        }
    }
}

// Bresenham's algorithm - thx wikpedia
void View::drawLine(int16_t x0, int16_t y0,
                    int16_t x1, int16_t y1, uint16_t color)
{
    bool steep = abs(y1 - y0) > abs(x1 - x0);
    if (steep) {
        tgfx_swap(x0, y0);
        tgfx_swap(x1, y1);
    }
    if (x0 > x1) {
        tgfx_swap(x0, x1);
        tgfx_swap(y0, y1);
    }

    int16_t dx, dy;
    dx = x1 - x0;
    dy = abs(y1 - y0);

    int16_t err = dx / 2;
    int16_t ystep;

    if (y0 < y1) {
        ystep = 1;
    } else {
        ystep = -1;
    }

    int16_t xbegin = x0;
    if (steep) {
        for (; x0<=x1; x0++) {
            err -= dy;
            if (err < 0) {
                int16_t len = x0 - xbegin;
                if (len) {
                    VLine(y0, xbegin, len + 1, color);
                } else {
                    Pixel(y0, x0, color);
                }
                xbegin = x0 + 1;
                y0 += ystep;
                err += dx;
            }
        }
        if (x0 > xbegin + 1) {
            VLine(y0, xbegin, x0 - xbegin, color);
        }

    } else {
        for (; x0<=x1; x0++) {
            err -= dy;
            if (err < 0) {
                int16_t len = x0 - xbegin;
                if (len) {
                    HLine(xbegin, y0, len + 1, color);
                } else {
                    Pixel(x0, y0, color);
                }
                xbegin = x0 + 1;
                y0 += ystep;
                err += dx;
            }
        }
        if (x0 > xbegin + 1) {
            HLine(xbegin, y0, x0 - xbegin, color);
        }
    }
}

void View::drawBitmap(int16_t x, int16_t y,
                      const uint8_t *bitmap, int16_t w, int16_t h,
                      uint16_t color) {

    int16_t i, j, byteWidth = (w + 7) / 8;

    for(j=0; j<h; j++) {
        for(i=0; i<w; i++ ) {
            if(bitmap[j * byteWidth + i / 8] & (128 >> (i & 7))) {
                drawPixel(x+i, y+j, color);
            }
        }
    }
}

void View::setCursor(int16_t x, int16_t y, bool autoCenter) {
    _center_x_text = autoCenter;	// remember the state.
    _center_y_text = autoCenter;	// remember the state.
    if (x == View::CENTER) {
        _center_x_text = true;
        x = _width/2;
    }
    if (y == View::CENTER) {
        _center_y_text = true;
        y = _height/2;
    }
    if (x < 0) x = 0;
    else if (x >= _width) x = _width - 1;
    cursor_x = x;
    if (y < 0) y = 0;
    else if (y >= _height) y = _height - 1;
    cursor_y = y;

    if(x>=scroll_x && x<=(scroll_x+scroll_width) && y>=scroll_y && y<=(scroll_y+scroll_height)){
        isWritingScrollArea	= true;
    } else {
        isWritingScrollArea = false;
    }
    _gfx_last_char_x_write = 0;	// Don't use cached data here

}

void View::getCursor(int16_t *x, int16_t *y) {
    *x = cursor_x;
    *y = cursor_y;
}

void View::setTextSize(uint8_t s_x, uint8_t s_y) {
    textsize_x = (s_x > 0) ? s_x : 1;
    textsize_y = (s_y > 0) ? s_y : 1;
}

uint8_t View::getTextSize() {
    return textsize_x;	// bug bug 2 values now
}

uint8_t View::getTextSizeX() {
    return textsize_x;
}
uint8_t View::getTextSizeY() {
    return textsize_y;
}

void View::setTextColor(uint16_t c) {
    // For 'transparent' background, we'll set the bg
    // to the same as fg instead of using a flag
    textcolor = textbgcolor = c;
}

void View::setTextColor(uint16_t c, uint16_t b) {
    textcolor   = c;
    textbgcolor = b;
    // pre-expand colors for fast alpha-blending later
    textcolorPrexpanded = (textcolor | (textcolor << 16)) & 0b00000111111000001111100000011111;
    textbgcolorPrexpanded = (textbgcolor | (textbgcolor << 16)) & 0b00000111111000001111100000011111;
}

void View::setTextWrap(bool w) {
    wrap = w;
}

bool View::getTextWrap()
{
    return wrap;
}

uint8_t View::getRotation(void) {
    return _rot;
}


/***************************************************************************************
** Function name:           setTextDatum
** Description:             Set the text position reference datum
***************************************************************************************/
void View::setTextDatum(uint8_t d)
{
    textdatum = d;
}


/***************************************************************************************
** Function name:           drawNumber
** Description:             draw a long integer
***************************************************************************************/
int16_t View::drawNumber(long long_num, int poX, int poY)
{
    char str[14];
    sprintf(str, "%ld", long_num);
    return drawString1(str, strlen(str), poX, poY);
}


int16_t View::drawFloat(float floatNumber, int dp, int poX, int poY)
{
    char str[14];               // Array to contain decimal string
    uint8_t ptr = 0;            // Initialise pointer for array
    int8_t  digits = 1;         // Count the digits to avoid array overflow
    float rounding = 0.5;       // Round up down delta
    char *format = "%%.%df";
    char *format2 = "%%.%df";
    sprintf(format2, format, dp);
    sprintf(str, format2, floatNumber);
    return 0;
}

/***************************************************************************************
** Function name:           drawString (with or without user defined font)
** Description :            draw string with padding if it is defined
***************************************************************************************/
// Without font number, uses font set by setTextFont()
int16_t View::drawString(const String& string, int poX, int poY)
{
    return drawString1((char*)string.c_str(), string.length(), poX, poY);
}

int16_t View::drawString1(char string[], int16_t len, int poX, int poY)
{
    int16_t sumX = 0, currentX = 0;
    uint8_t padding = 1/*, baseline = 0*/;

    uint16_t cwidth = strPixelLen(string); // Find the pixel width of the string in the font
    uint16_t cheight = textsize_y*8;


    if (textdatum || padX)
    {
        switch(textdatum) {
            case TC_DATUM:
                poX -= cwidth/2;
                padding += 1;
                break;
            case TR_DATUM:
                poX -= cwidth;
                padding += 2;
                break;
            case ML_DATUM:
                poY -= cheight/2;
                //padding += 0;
                break;
            case MC_DATUM:
                poX -= cwidth/2;
                poY -= cheight/2;
                padding += 1;
                break;
            case MR_DATUM:
                poX -= cwidth;
                poY -= cheight/2;
                padding += 2;
                break;
            case BL_DATUM:
                poY -= cheight;
                //padding += 0;
                break;
            case BC_DATUM:
                poX -= cwidth/2;
                poY -= cheight;
                padding += 1;
                break;
            case BR_DATUM:
                poX -= cwidth;
                poY -= cheight;
                padding += 2;
                break;
                /*
                 case L_BASELINE:
                   poY -= baseline;
                   //padding += 0;
                   break;
                 case C_BASELINE:
                   poX -= cwidth/2;
                   poY -= baseline;
                   //padding += 1;
                   break;
                 case R_BASELINE:
                   poX -= cwidth;
                   poY -= baseline;
                   padding += 2;
                   break;
               */
        }
        // Check coordinates are OK, adjust if not
        if (poX < 0) poX = 0;
        if (poX+cwidth > width())   poX = width() - cwidth;
        if (poY < 0) poY = 0;
        //if (poY+cheight-baseline >_height) poY = _height - cheight;
    }
    if(font == NULL){
        for(uint8_t i = 0; i < len; i++){
            if (wrap && ((currentX + textsize_x) >= _width-1)) { // Off right?
                currentX = 0;                                       // Reset x to zero,
                poY += textsize_y * 8; // advance y one line
            }
            drawChar((int16_t) (poX+currentX), (int16_t) poY, string[i], textcolor, textbgcolor, textsize_x, textsize_y);
            sumX += cwidth/(len) + padding;
            currentX += textsize_y * 6;
        }
    } else {
        setCursor(poX, poY);
        for(uint8_t i = 0; i < len; i++){
            drawFontChar(string[i]);
            setCursor(cursor_x, cursor_y);
        }
    }
    return sumX;
}


void View::scrollTextArea(uint8_t scrollSize){
    uint16_t awColors[scroll_width];
    for (int y=scroll_y+scrollSize; y < (scroll_y+scroll_height); y++) {
        readRect(scroll_x, y, scroll_width, 1, awColors);
        writeRect(scroll_x, y-scrollSize, scroll_width, 1, awColors);
    }
    fillRect(scroll_x, (scroll_y+scroll_height)-scrollSize, scroll_width, scrollSize, scrollbgcolor);
}

void View::setScrollTextArea(int16_t x, int16_t y, int16_t w, int16_t h){
    scroll_x = x;
    scroll_y = y;
    scroll_width = w;
    scroll_height = h;
}

void View::setScrollBackgroundColor(uint16_t color){
    scrollbgcolor=color;
    fillRect(scroll_x,scroll_y,scroll_width,scroll_height,scrollbgcolor);
}

void View::enableScroll(void){
    scrollEnable = true;
}

void View::disableScroll(void){
    scrollEnable = false;
}

void View::resetScrollBackgroundColor(uint16_t color){
    scrollbgcolor=color;
}

// Draw a character
void View::drawChar(int16_t x, int16_t y, unsigned char c,
                    uint16_t fgcolor, uint16_t bgcolor, uint8_t size_x, uint8_t size_y)
{
    if((x >= _width)            || // Clip right
       (y >= _height)           || // Clip bottom
       ((x + 6 * size_x - 1) < 0) || // Clip left  TODO: is this correct?
       ((y + 8 * size_y - 1) < 0))   // Clip top   TODO: is this correct?
        return;
    if (fgcolor == bgcolor) {
        // This transparent approach is only about 20% faster
        if ((size_x == 1) && (size_y == 1)) {
            uint8_t mask = 0x01;
            int16_t xoff, yoff;
            for (yoff=0; yoff < 8; yoff++) {
                uint8_t line = 0;
                for (xoff=0; xoff < 5; xoff++) {
                    if (glcdfont[c * 5 + xoff] & mask) line |= 1;
                    line <<= 1;
                }
                line >>= 1;
                xoff = 0;
                while (line) {
                    if (line == 0x1F) {
                        drawFastHLine(x + xoff, y + yoff, 5, fgcolor);
                        break;
                    } else if (line == 0x1E) {
                        drawFastHLine(x + xoff, y + yoff, 4, fgcolor);
                        break;
                    } else if ((line & 0x1C) == 0x1C) {
                        drawFastHLine(x + xoff, y + yoff, 3, fgcolor);
                        line <<= 4;
                        xoff += 4;
                    } else if ((line & 0x18) == 0x18) {
                        drawFastHLine(x + xoff, y + yoff, 2, fgcolor);
                        line <<= 3;
                        xoff += 3;
                    } else if ((line & 0x10) == 0x10) {
                        drawPixel(x + xoff, y + yoff, fgcolor);
                        line <<= 2;
                        xoff += 2;
                    } else {
                        line <<= 1;
                        xoff += 1;
                    }
                }
                mask = mask << 1;
            }
        } else {
            uint8_t mask = 0x01;
            int16_t xoff, yoff;
            for (yoff=0; yoff < 8; yoff++) {
                uint8_t line = 0;
                for (xoff=0; xoff < 5; xoff++) {
                    if (glcdfont[c * 5 + xoff] & mask) line |= 1;
                    line <<= 1;
                }
                line >>= 1;
                xoff = 0;
                while (line) {
                    if (line == 0x1F) {
                        fillRect(x + xoff * size_x, y + yoff * size_y,
                                 5 * size_x, size_y, fgcolor);
                        break;
                    } else if (line == 0x1E) {
                        fillRect(x + xoff * size_x, y + yoff * size_y,
                                 4 * size_x, size_y, fgcolor);
                        break;
                    } else if ((line & 0x1C) == 0x1C) {
                        fillRect(x + xoff * size_x, y + yoff * size_y,
                                 3 * size_x, size_y, fgcolor);
                        line <<= 4;
                        xoff += 4;
                    } else if ((line & 0x18) == 0x18) {
                        fillRect(x + xoff * size_x, y + yoff * size_y,
                                 2 * size_x, size_y, fgcolor);
                        line <<= 3;
                        xoff += 3;
                    } else if ((line & 0x10) == 0x10) {
                        fillRect(x + xoff * size_x, y + yoff * size_y,
                                 size_x, size_y, fgcolor);
                        line <<= 2;
                        xoff += 2;
                    } else {
                        line <<= 1;
                        xoff += 1;
                    }
                }
                mask = mask << 1;
            }
        }
    } else {
        // This solid background approach is about 5 time faster
        uint8_t xc, yc;
        uint8_t xr, yr;
        uint8_t mask = 0x01;
        uint16_t color;

        // We need to offset by the origin.
        x+=_originx;
        y+=_originy;
        int16_t x_char_start = x;  // remember our X where we start outputting...

        if((x >= _displayclipx2)            || // Clip right
           (y >= _displayclipy2)           || // Clip bottom
           ((x + 6 * size_x - 1) < _displayclipx1) || // Clip left  TODO: this is not correct
           ((y + 8 * size_y - 1) < _displayclipy1))   // Clip top   TODO: this is not correct
            return;

        // need to build actual pixel rectangle we will output into.
        int16_t y_char_top = y;	// remember the y
        int16_t w =  6 * size_x;
        int16_t h = 8 * size_y;

        if(x < _displayclipx1) {	w -= (_displayclipx1-x); x = _displayclipx1; 	}
        if((x + w - 1) >= _displayclipx2)  w = _displayclipx2  - x;
        if(y < _displayclipy1) {	h -= (_displayclipy1 - y); y = _displayclipy1; 	}
        if((y + h - 1) >= _displayclipy2) h = _displayclipy2 - y;
    }

}

void View::setFont(const ILI9341_t3_font_t &f) {
    font = &f;
    _gfx_last_char_x_write = 0;	// Don't use cached data here
    if (gfxFont) {
        cursor_y -= 6;
        gfxFont = NULL;
    }
    fontbpp = 1;
    // Calculate additional metrics for Anti-Aliased font support (BDF extn v2.3)
    if (font && font->version==23){
        fontbpp = (font->reserved & 0b000011)+1;
        fontbppindex = (fontbpp >> 2)+1;
        fontbppmask = (1 << (fontbppindex+1))-1;
        fontppb = 8/fontbpp;
        fontalphamx = 31/((1<<fontbpp)-1);
        // Ensure text and bg color are different. Note: use setTextColor to set actual bg color
        if (textcolor == textbgcolor) textbgcolor = (textcolor==0x0000)?0xFFFF:0x0000;
    }
}

// Maybe support GFX Fonts as well?
void View::setFont(const GFXfont *f) {
    font = NULL;	// turn off the other font...
    _gfx_last_char_x_write = 0;	// Don't use cached data here
    if (f == gfxFont) return;	// same font or lack of so can bail.

    if(f) {            // Font struct pointer passed in?
        if(!gfxFont) { // And no current font struct?
            // Switching from classic to new font behavior.
            // Move cursor pos down 6 pixels so it's on baseline.
            cursor_y += 6;
        }

        // Test wondering high and low of Ys here...
        int8_t miny_offset = 0;
#if 1
        for (uint8_t i=0; i <= (f->last - f->first); i++) {
            if (f->glyph[i].yOffset < miny_offset) {
                miny_offset = f->glyph[i].yOffset;
            }
        }
#else
        int max_delta = 0;
        uint8_t index_min = 0;
        uint8_t index_max = 0;
        int8_t minx_offset = 127;
        int8_t maxx_overlap = 0;
        uint8_t indexx_min = 0;
        uint8_t indexx_max = 0;
        for (uint8_t i=0; i <= (f->last - f->first); i++) {
        	if (f->glyph[i].yOffset < miny_offset) {
        		miny_offset = f->glyph[i].yOffset;
        		index_min = i;
        	}
        	if (f->glyph[i].xOffset < minx_offset) {
        		minx_offset = f->glyph[i].xOffset;
        		indexx_min = i;
        	}
        	if ( (f->glyph[i].yOffset + f->glyph[i].height) > max_delta) {
        		max_delta = (f->glyph[i].yOffset + f->glyph[i].height);
        		index_max = i;
        	}
        	int8_t x_overlap = f->glyph[i].xOffset + f->glyph[i].width - f->glyph[i].xAdvance;
        	if (x_overlap > maxx_overlap) {
        		maxx_overlap = x_overlap;
        		indexx_max = i;
        	}
        }
        Serial.printf("Set GFX Font(%x): Y: %d %d(%c) %d(%c) X: %d(%c) %d(%c)\n", (uint32_t)f, f->yAdvance,
        	miny_offset, index_min + f->first, max_delta, index_max + f->first,
        	minx_offset, indexx_min + f->first, maxx_overlap, indexx_max + f->first);
#endif
        _gfxFont_min_yOffset = miny_offset;	// Probably only thing we need... May cache?

    } else if(gfxFont) { // NULL passed.  Current font struct defined?
        // Switching from new to classic font behavior.
        // Move cursor pos up 6 pixels so it's at top-left of char.
        cursor_y -= 6;
    }
    gfxFont = f;
}

static uint32_t fetchbit(const uint8_t *p, uint32_t index)
{
    if (p[index >> 3] & (1 << (7 - (index & 7)))) return 1;
    return 0;
}

static uint32_t fetchbits_unsigned(const uint8_t *p, uint32_t index, uint32_t required)
{
    uint32_t val = 0;
    do {
        uint8_t b = p[index >> 3];
        uint32_t avail = 8 - (index & 7);
        if (avail <= required) {
            val <<= avail;
            val |= b & ((1 << avail) - 1);
            index += avail;
            required -= avail;
        } else {
            b >>= avail - required;
            val <<= required;
            val |= b & ((1 << required) - 1);
            break;
        }
    } while (required);
    return val;
}

static uint32_t fetchbits_signed(const uint8_t *p, uint32_t index, uint32_t required)
{
    uint32_t val = fetchbits_unsigned(p, index, required);
    if (val & (1 << (required - 1))) {
        return (int32_t)val - (1 << required);
    }
    return (int32_t)val;
}

uint32_t View::fetchpixel(const uint8_t *p, uint32_t index, uint32_t x)
{
    // The byte
    uint8_t b = p[index >> 3];
    // Shift to LSB position and mask to get value
    uint8_t s = ((fontppb-(x % fontppb)-1)*fontbpp);
    // Mask and return
    return (b >> s) & fontbppmask;
}

void View::drawFontChar(unsigned int c)
{
    uint32_t bitoffset;
    const uint8_t *data;

    //Serial.printf("drawFontChar(%c) %d\n", c, c);

    if (c >= font->index1_first && c <= font->index1_last) {
        bitoffset = c - font->index1_first;
        bitoffset *= font->bits_index;
    } else if (c >= font->index2_first && c <= font->index2_last) {
        bitoffset = c - font->index2_first + font->index1_last - font->index1_first + 1;
        bitoffset *= font->bits_index;
    } else if (font->unicode) {
        return; // TODO: implement sparse unicode
    } else {
        return;
    }
    //Serial.printf("  index =  %d\n", fetchbits_unsigned(font->index, bitoffset, font->bits_index));
    data = font->data + fetchbits_unsigned(font->index, bitoffset, font->bits_index);

    uint32_t encoding = fetchbits_unsigned(data, 0, 3);
    if (encoding != 0) return;
    uint32_t width = fetchbits_unsigned(data, 3, font->bits_width);
    bitoffset = font->bits_width + 3;
    uint32_t height = fetchbits_unsigned(data, bitoffset, font->bits_height);
    bitoffset += font->bits_height;
    //Serial.printf("  size =   %d,%d\n", width, height);
    //Serial.printf("  line space = %d\n", font->line_space);


    int32_t xoffset = fetchbits_signed(data, bitoffset, font->bits_xoffset);
    bitoffset += font->bits_xoffset;
    int32_t yoffset = fetchbits_signed(data, bitoffset, font->bits_yoffset);
    bitoffset += font->bits_yoffset;
    //Serial.printf("  offset = %d,%d\n", xoffset, yoffset);
    //Serial.printf("DChar: %c %u, %u, wh:%d %d o:%d %d\n", c, cursor_x, cursor_y, width, height, xoffset, yoffset);

    uint32_t delta = fetchbits_unsigned(data, bitoffset, font->bits_delta);
    bitoffset += font->bits_delta;
    //Serial.printf("  delta =  %d\n", delta);

    //Serial.printf("  cursor = %d,%d\n", cursor_x, cursor_y);

    //horizontally, we draw every pixel, or none at all
    if (cursor_x < 0) cursor_x = 0;
    int32_t origin_x = cursor_x + xoffset;
    if (origin_x < 0) {
        cursor_x -= xoffset;
        origin_x = 0;
    }
    if (origin_x + (int)width > _width) {
        if (!wrap) return;
        origin_x = 0;
        if (xoffset >= 0) {
            cursor_x = 0;
        } else {
            cursor_x = -xoffset;
        }
        cursor_y += font->line_space;
    }
    if(wrap && scrollEnable && isWritingScrollArea && ((origin_x + (int)width) > (scroll_x+scroll_width))){
        origin_x = 0;
        if (xoffset >= 0) {
            cursor_x = scroll_x;
        } else {
            cursor_x = -xoffset;
        }
        cursor_y += font->line_space;
    }

    if(scrollEnable && isWritingScrollArea && (cursor_y > (scroll_y+scroll_height - font->cap_height))){
        scrollTextArea(font->line_space);
        cursor_y -= font->line_space;
        cursor_x = scroll_x;
    }
    if (cursor_y >= _height) return;

    // vertically, the top and/or bottom can be clipped
    int32_t origin_y = cursor_y + font->cap_height - height - yoffset;
    //Serial.printf("  origin = %d,%d\n", origin_x, origin_y);

    // TODO: compute top skip and number of lines
    int32_t linecount = height;
    //uint32_t loopcount = 0;
    int32_t y = origin_y;
    bool opaque = (textbgcolor != textcolor);

    // Going to try a fast Opaque method which works similar to drawChar, which is near the speed of writerect
    if (!opaque) {

        // Anti-alias support
        if (fontbpp>1){
            // This branch should, in most cases, never happen. This is because if an
            // anti-aliased font is being used, textcolor and textbgcolor should always
            // be different. Even though an anti-alised font is being used, pixels in this
            // case will all be solid because pixels are rendered on same colour as themselves!
            // This won't look very good.
            bitoffset = ((bitoffset + 7) & (-8)); // byte-boundary
            uint32_t xp = 0;
            uint8_t halfalpha = 1<<(fontbpp-1);
            while (linecount) {
                uint32_t x = 0;
                while(x<width) {
                    // One pixel at a time, either on (if alpha > 0.5) or off
                    if (fetchpixel(data, bitoffset, xp)>=halfalpha){
                        Pixel(origin_x + x,y,textcolor);
                    }
                    bitoffset += fontbpp;
                    x++;
                    xp++;
                }
                y++;
                linecount--;
            }

        }
            // Soild pixels
        else{

            while (linecount > 0) {
                //Serial.printf("    linecount = %d\n", linecount);
                uint32_t n = 1;
                if (fetchbit(data, bitoffset++) != 0) {
                    n = fetchbits_unsigned(data, bitoffset, 3) + 2;
                    bitoffset += 3;
                }
                uint32_t x = 0;
                do {
                    int32_t xsize = width - x;
                    if (xsize > 32) xsize = 32;
                    uint32_t bits = fetchbits_unsigned(data, bitoffset, xsize);
                    //Serial.printf("    multi line %d %d %x\n", n, x, bits);
                    drawFontBits(opaque, bits, xsize, origin_x + x, y, n);
                    bitoffset += xsize;
                    x += xsize;
                } while (x < width);


                y += n;
                linecount -= n;
                //if (++loopcount > 100) {
                //Serial.println("     abort draw loop");
                //break;
                //}
            }
        } // 1bpp
    }

        // opaque
    else {
        // Now opaque mode...
        // Now write out background color for the number of rows above the above the character
        // figure out bounding rectangle...
        // In this mode we need to update to use the offset and bounding rectangles as we are doing it it direct.
        // also update the Origin
        int cursor_x_origin = cursor_x + _originx;
        int cursor_y_origin = cursor_y + _originy;
        origin_x += _originx;
        origin_y += _originy;

        int start_x = (origin_x < cursor_x_origin) ? origin_x : cursor_x_origin;
        if (start_x < 0) start_x = 0;

        int start_y = (origin_y < cursor_y_origin) ? origin_y : cursor_y_origin;
        if (start_y < 0) start_y = 0;
        int end_x = cursor_x_origin + delta;
        if ((origin_x + (int)width) > end_x)
            end_x = origin_x + (int)width;
        if (end_x >= _displayclipx2)  end_x = _displayclipx2;
        int end_y = cursor_y_origin + font->line_space;
        if ((origin_y + (int)height) > end_y)
            end_y = origin_y + (int)height;
        if (end_y >= _displayclipy2) end_y = _displayclipy2;
        end_x--;	// setup to last one we draw
        end_y--;
        int start_x_min = (start_x >= _displayclipx1) ? start_x : _displayclipx1;
        int start_y_min = (start_y >= _displayclipy1) ? start_y : _displayclipy1;

        // See if anything is in the display area.
        if((end_x < _displayclipx1) ||(start_x >= _displayclipx2) || (end_y < _displayclipy1) || (start_y >= _displayclipy2)) {
            cursor_x += delta;	// could use goto or another indent level...
            return;
        }
/*
		Serial.printf("drawFontChar(%c) %d\n", c, c);
		Serial.printf("  size =   %d,%d\n", width, height);
		Serial.printf("  line space = %d\n", font->line_space);
		Serial.printf("  offset = %d,%d\n", xoffset, yoffset);
		Serial.printf("  delta =  %d\n", delta);
		Serial.printf("  cursor = %d,%d\n", cursor_x, cursor_y);
		Serial.printf("  origin = %d,%d\n", origin_x, origin_y);
		Serial.printf("  Bounding: (%d, %d)-(%d, %d)\n", start_x, start_y, end_x, end_y);
		Serial.printf("  mins (%d %d),\n", start_x_min, start_y_min);
*/
#ifdef ENABLE_ST77XX_FRAMEBUFFER
        if (_use_fbtft) {
			uint16_t * pfbPixel_row = &_pfbtft[ start_y*_width + start_x];
			uint16_t * pfbPixel;
			int screen_y = start_y;
			int screen_x;

			// Clear above character
			while (screen_y < origin_y) {
				pfbPixel = pfbPixel_row;
				// only output if this line is within the clipping region.
				if ((screen_y >= _displayclipy1) && (screen_y < _displayclipy2)) {
					for (screen_x = start_x; screen_x <= end_x; screen_x++) {
						if (screen_x >= _displayclipx1) {
							*pfbPixel = textbgcolor;
						}
						pfbPixel++;
					}
				}
				screen_y++;
				pfbPixel_row += _width;
			}

			// Anti-aliased font
			if (fontbpp>1){
				screen_y = origin_y;
				bitoffset = ((bitoffset + 7) & (-8)); // byte-boundary
				uint32_t xp = 0;
				int glyphend_x = origin_x+width;
				while (linecount) {
					pfbPixel = pfbPixel_row;
					screen_x = start_x;
					while(screen_x<=end_x) {
						// XXX: I'm sure clipping could be done way more efficiently than just chekcing every single pixel, but let's just get this going
						if ((screen_x >= _displayclipx1) && (screen_x < _displayclipx2) && (screen_y >= _displayclipy1) && (screen_y < _displayclipy2)) {
							// Clear before or after pixel
							if ((screen_x<origin_x) || (screen_x>=glyphend_x)){
								*pfbPixel = textbgcolor;
							}
							// Draw alpha-blended character
							else{
								uint8_t alpha = fetchpixel(data, bitoffset, xp);
								*pfbPixel = alphaBlendRGB565Premultiplied( textcolorPrexpanded, textbgcolorPrexpanded, (uint8_t)(alpha * fontalphamx) );
								bitoffset += fontbpp;
								xp++;
							}
						} // clip
						screen_x++;
						pfbPixel++;
					}
					pfbPixel_row += _width;
					screen_y++;
					linecount--;
				}

			} // anti-aliased

			// 1bpp solid font
			else{

				// Now lets process each of the data lines (draw character)
				screen_y = origin_y;
				while (linecount > 0) {
					//Serial.printf("    linecount = %d\n", linecount);
					uint32_t b = fetchbit(data, bitoffset++);
					uint32_t n;
					if (b == 0) {
						//Serial.println("Single");
						n = 1;
					} else {
						//Serial.println("Multi");
						n = fetchbits_unsigned(data, bitoffset, 3) + 2;
						bitoffset += 3;
					}
					uint32_t bitoffset_row_start = bitoffset;
					while (n--) {
						pfbPixel = pfbPixel_row;

						// Clear to left
						if ((screen_y >= _displayclipy1) && (screen_y < _displayclipy2)) {
							bitoffset = bitoffset_row_start;	// we will work through these bits maybe multiple times
							for (screen_x = start_x; screen_x < origin_x; screen_x++) {
								if (screen_x >= _displayclipx1) {
									*pfbPixel = textbgcolor;
								} // make sure not clipped
								pfbPixel++;
							}
						}

						// Pixel bits
						screen_x = origin_x;
						uint32_t x = 0;
						do {
							uint32_t xsize = width - x;
							if (xsize > 32) xsize = 32;
							uint32_t bits = fetchbits_unsigned(data, bitoffset, xsize);
							uint32_t bit_mask = 1 << (xsize-1);
							//Serial.printf(" %d %d %x %x\n", x, xsize, bits, bit_mask);
							if ((screen_y >= _displayclipy1) && (screen_y < _displayclipy2)) {
								while (bit_mask && (screen_x <= end_x)) {
									if ((screen_x >= _displayclipx1) && (screen_x < _displayclipx2)) {
										*pfbPixel = (bits & bit_mask) ? textcolor : textbgcolor;
									}
									pfbPixel++;
									bit_mask = bit_mask >> 1;
									screen_x++;	// increment our pixel position.
								}
							}
								bitoffset += xsize;
							x += xsize;
						} while (x < width);

						// Clear to right
						if ((screen_y >= _displayclipy1) && (screen_y < _displayclipy2)) {
							// output bg color and right hand side
							while (screen_x++ <= end_x) {
								*pfbPixel++ = textbgcolor;
							}
						}
			 			screen_y++;
						pfbPixel_row += _width;
						linecount--;
					}
				}

			} // 1bpp

			// clear below character
	 		while (screen_y++ <= end_y) {
				if ((screen_y >= _displayclipy1) && (screen_y < _displayclipy2)) {
					pfbPixel = pfbPixel_row;
					for (screen_x = start_x; screen_x <= end_x; screen_x++) {
						if (screen_x >= _displayclipx1) {
							*pfbPixel = textbgcolor;
						}
						pfbPixel++;
					}
				}
				pfbPixel_row += _width;
			}

		} else
#endif
        {


            int screen_y = start_y_min;
            int screen_x;

            // Clear above character
            while (screen_y < origin_y) {
                for (screen_x = start_x_min; screen_x <= end_x; screen_x++) {
                    // writedata16(textbgcolor);
                }
                screen_y++;
            }

            // Anti-aliased font
            if (fontbpp>1){
                screen_y = origin_y;
                bitoffset = ((bitoffset + 7) & (-8)); // byte-boundary
                int glyphend_x = origin_x+width;
                uint32_t xp = 0;
                while (linecount) {
                    screen_x = start_x;
                    while(screen_x<=end_x) {
                        // XXX: I'm sure clipping could be done way more efficiently than just chekcing every single pixel, but let's just get this going
                        if ((screen_x >= _displayclipx1) && (screen_x < _displayclipx2) && (screen_y >= _displayclipy1) && (screen_y < _displayclipy2)) {
                            // Clear before or after pixel
                            if ((screen_x<origin_x) || (screen_x>=glyphend_x)){
                                writedata16(textbgcolor);
                            }
                                // Draw alpha-blended character
                            else{
                                uint8_t alpha = fetchpixel(data, bitoffset, xp);
                                writedata16( alphaBlendRGB565Premultiplied( textcolorPrexpanded, textbgcolorPrexpanded, (uint8_t)(alpha * fontalphamx) ) );
                                bitoffset += fontbpp;
                                xp++;
                            }
                        } // clip
                        screen_x++;
                    }
                    screen_y++;
                    linecount--;
                }

            } // anti-aliased

                // 1bpp
            else{

                // Now lets process each of the data lines.
                screen_y = origin_y;
                while (linecount > 0) {
                    //Serial.printf("    linecount = %d\n", linecount);
                    uint32_t b = fetchbit(data, bitoffset++);
                    uint32_t n;
                    if (b == 0) {
                        //Serial.println("    Single");
                        n = 1;
                    } else {
                        //Serial.println("    Multi");
                        n = fetchbits_unsigned(data, bitoffset, 3) + 2;
                        bitoffset += 3;
                    }
                    uint32_t bitoffset_row_start = bitoffset;
                    while (n--) {
                        // do some clipping here.
                        bitoffset = bitoffset_row_start;	// we will work through these bits maybe multiple times
                        // We need to handle case where some of the bits may not be visible, but we still need to
                        // read through them
                        //Serial.printf("y:%d  %d %d %d %d\n", screen_y, start_x, origin_x, _displayclipx1, _displayclipx2);
                        if ((screen_y >= _displayclipy1) && (screen_y < _displayclipy2)) {
                            for (screen_x = start_x; screen_x < origin_x; screen_x++) {
                                if ((screen_x >= _displayclipx1) && (screen_x < _displayclipx2)) {
                                    //Serial.write('-');
                                    writedata16(textbgcolor);
                                }
                            }
                        }
                        uint32_t x = 0;
                        screen_x = origin_x;
                        do {
                            uint32_t xsize = width - x;
                            if (xsize > 32) xsize = 32;
                            uint32_t bits = fetchbits_unsigned(data, bitoffset, xsize);
                            uint32_t bit_mask = 1 << (xsize-1);
                            //Serial.printf("     %d %d %x %x - ", x, xsize, bits, bit_mask);
                            if ((screen_y >= _displayclipy1) && (screen_y < _displayclipy2)) {
                                while (bit_mask) {
                                    if ((screen_x >= _displayclipx1) && (screen_x < _displayclipx2)) {
                                        writedata16((bits & bit_mask) ? textcolor : textbgcolor);
                                        //Serial.write((bits & bit_mask) ? '*' : '.');
                                    }
                                    bit_mask = bit_mask >> 1;
                                    screen_x++ ; // Current actual screen X
                                }
                                //Serial.println();
                                bitoffset += xsize;
                            }
                            x += xsize;
                        } while (x < width) ;
                        if ((screen_y >= _displayclipy1) && (screen_y < _displayclipy2)) {
                            // output bg color and right hand side
                            while (screen_x++ <= end_x) {
                                writedata16(textbgcolor);
                                //Serial.write('+');
                            }
                            //Serial.println();
                        }
                        screen_y++;
                        linecount--;
                    }
                }
            } // 1bpp

            // clear below character - note reusing xcreen_x for this
            screen_x = (end_y + 1 - screen_y) * (end_x + 1 - start_x_min); // How many bytes we need to still output
            //Serial.printf("Clear Below: %d\n", screen_x);
            while (screen_x-- > 1) {
                writedata16(textbgcolor);
            }
            writedata16_last(textbgcolor);
        }

    }
    // Increment to setup for the next character.
    cursor_x += delta;

}

//strPixelLen			- gets pixel length of given ASCII string
int16_t View::strPixelLen(const char * str)
{
//	//Serial.printf("strPixelLen %s\n", str);
    if (!str) return(0);
    if (gfxFont)
    {
        // BUGBUG:: just use the other function for now... May do this for all of them...
        int16_t x, y;
        uint16_t w, h;
        getTextBounds(str, cursor_x, cursor_y, &x, &y, &w, &h);
        return w;
    }

    uint16_t len=0, maxlen=0;
    while (*str)
    {
        if (*str=='\n')
        {
            if ( len > maxlen )
            {
                maxlen=len;
                len=0;
            }
        }
        else
        {
            if (!font)
            {
                len+=textsize_x*6;
            }
            else
            {

                uint32_t bitoffset;
                const uint8_t *data;
                uint16_t c = *str;

//				//Serial.printf("char %c(%d)\n", c,c);

                if (c >= font->index1_first && c <= font->index1_last) {
                    bitoffset = c - font->index1_first;
                    bitoffset *= font->bits_index;
                } else if (c >= font->index2_first && c <= font->index2_last) {
                    bitoffset = c - font->index2_first + font->index1_last - font->index1_first + 1;
                    bitoffset *= font->bits_index;
                } else if (font->unicode) {
                    continue;
                } else {
                    continue;
                }
                //Serial.printf("  index =  %d\n", fetchbits_unsigned(font->index, bitoffset, font->bits_index));
                data = font->data + fetchbits_unsigned(font->index, bitoffset, font->bits_index);

                uint32_t encoding = fetchbits_unsigned(data, 0, 3);
                if (encoding != 0) continue;
//				uint32_t width = fetchbits_unsigned(data, 3, font->bits_screenWidth);
//				//Serial.printf("  width =  %d\n", width);
                bitoffset = font->bits_width + 3;
                bitoffset += font->bits_height;

//				int32_t xoffset = fetchbits_signed(data, bitoffset, font->bits_xoffset);
//				//Serial.printf("  xoffset =  %d\n", xoffset);
                bitoffset += font->bits_xoffset;
                bitoffset += font->bits_yoffset;

                uint32_t delta = fetchbits_unsigned(data, bitoffset, font->bits_delta);
                bitoffset += font->bits_delta;
//				//Serial.printf("  delta =  %d\n", delta);

                len += delta;//+width-xoffset;
//				//Serial.printf("  len =  %d\n", len);
            }

            if ( len > maxlen )
            {
                maxlen=len;
//					//Serial.printf("  maxlen =  %d\n", maxlen);
            }
        }
        str++;
    }
//	//Serial.printf("Return  maxlen =  %d\n", maxlen);
    return( maxlen );
}

void View::charBounds(char c, int16_t *x, int16_t *y,
                      int16_t *minx, int16_t *miny, int16_t *maxx, int16_t *maxy) {

    // BUGBUG:: Not handling offset/clip
    if (font) {
        if(c == '\n') { // Newline?
            *x  = 0;    // Reset x to zero, advance y by one line
            *y += font->line_space;
        } else if(c != '\r') { // Not a carriage return; is normal char
            uint32_t bitoffset;
            const uint8_t *data;
            if (c >= font->index1_first && c <= font->index1_last) {
                bitoffset = c - font->index1_first;
                bitoffset *= font->bits_index;
            } else if (c >= font->index2_first && c <= font->index2_last) {
                bitoffset = c - font->index2_first + font->index1_last - font->index1_first + 1;
                bitoffset *= font->bits_index;
            } else if (font->unicode) {
                return; // TODO: implement sparse unicode
            } else {
                return;
            }
            //Serial.printf("  index =  %d\n", fetchbits_unsigned(font->index, bitoffset, font->bits_index));
            data = font->data + fetchbits_unsigned(font->index, bitoffset, font->bits_index);

            uint32_t encoding = fetchbits_unsigned(data, 0, 3);
            if (encoding != 0) return;
            uint32_t width = fetchbits_unsigned(data, 3, font->bits_width);
            bitoffset = font->bits_width + 3;
            uint32_t height = fetchbits_unsigned(data, bitoffset, font->bits_height);
            bitoffset += font->bits_height;
            //Serial.printf("  size =   %d,%d\n", width, height);
            //Serial.printf("  line space = %d\n", font->line_space);

            int32_t xoffset = fetchbits_signed(data, bitoffset, font->bits_xoffset);
            bitoffset += font->bits_xoffset;
            int32_t yoffset = fetchbits_signed(data, bitoffset, font->bits_yoffset);
            bitoffset += font->bits_yoffset;

            uint32_t delta = fetchbits_unsigned(data, bitoffset, font->bits_delta);
            bitoffset += font->bits_delta;

            int16_t
                    x1 = *x + xoffset,
                    y1 = *y + yoffset,
                    x2 = x1 + width,
                    y2 = y1 + height;

            if(wrap && (x2 > _width)) {
                *x  = 0; // Reset x to zero, advance y by one line
                *y += font->line_space;
                x1 = *x + xoffset,
                y1 = *y + yoffset,
                x2 = x1 + width,
                y2 = y1 + height;
            }
            if(x1 < *minx) *minx = x1;
            if(y1 < *miny) *miny = y1;
            if(x2 > *maxx) *maxx = x2;
            if(y2 > *maxy) *maxy = y2;
            *x += delta;	// ? guessing here...
        }
    }

    else if(gfxFont) {

        if(c == '\n') { // Newline?
            *x  = 0;    // Reset x to zero, advance y by one line
            *y += textsize_y * gfxFont->yAdvance;
        } else if(c != '\r') { // Not a carriage return; is normal char
            uint8_t first = gfxFont->first,
                    last  = gfxFont->last;
            if((c >= first) && (c <= last)) { // Char present in this font?
                GFXglyph *glyph  = gfxFont->glyph + (c - first);
                uint8_t gw = glyph->width,
                        gh = glyph->height,
                        xa = glyph->xAdvance;
                int8_t  xo = glyph->xOffset,
                        yo = glyph->yOffset + gfxFont->yAdvance/2;
                if(wrap && ((*x+(((int16_t)xo+gw)*textsize_x)) > _width)) {
                    *x  = 0; // Reset x to zero, advance y by one line
                    *y += textsize_y * gfxFont->yAdvance;
                }
                int16_t tsx = (int16_t)textsize_x,
                        tsy = (int16_t)textsize_y,
                        x1 = *x + xo * tsx,
                        y1 = *y + yo * tsy,
                        x2 = x1 + gw * tsx - 1,
                        y2 = y1 + gh * tsy - 1;
                if(x1 < *minx) *minx = x1;
                if(y1 < *miny) *miny = y1;
                if(x2 > *maxx) *maxx = x2;
                if(y2 > *maxy) *maxy = y2;
                *x += xa * tsx;
            }
        }

    } else { // Default font

        if(c == '\n') {                     // Newline?
            *x  = 0;                        // Reset x to zero,
            *y += textsize_y * 8;           // advance y one line
            // min/max x/y unchaged -- that waits for next 'normal' character
        } else if(c != '\r') {  // Normal char; ignore carriage returns
            if(wrap && ((*x + textsize_x * 6) > _width)) { // Off right?
                *x  = 0;                    // Reset x to zero,
                *y += textsize_y * 8;       // advance y one line
            }
            int x2 = *x + textsize_x * 6 - 1, // Lower-right pixel of char
            y2 = *y + textsize_y * 8 - 1;
            if(x2 > *maxx) *maxx = x2;      // Track max x, y
            if(y2 > *maxy) *maxy = y2;
            if(*x < *minx) *minx = *x;      // Track min x, y
            if(*y < *miny) *miny = *y;
            *x += textsize_x * 6;             // Advance x one char
        }
    }
}

// Add in Adafruit versions of text bounds calculations.
void View::getTextBounds(const uint8_t *buffer, uint16_t len, int16_t x, int16_t y,
                         int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h) {
    *x1 = x;
    *y1 = y;
    *w  = *h = 0;

    int16_t minx = _width, miny = _height, maxx = -1, maxy = -1;

    while(len--)
        charBounds(*buffer++, &x, &y, &minx, &miny, &maxx, &maxy);

    if(maxx >= minx) {
        *x1 = minx;
        *w  = maxx - minx + 1;
    }
    if(maxy >= miny) {
        *y1 = miny;
        *h  = maxy - miny + 1;
    }
}
void View::getTextBounds(const char *str, int16_t x, int16_t y,
                         int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h) {
    uint8_t c; // Current character

    *x1 = x;
    *y1 = y;
    *w  = *h = 0;

    int16_t minx = _width, miny = _height, maxx = -1, maxy = -1;

    while((c = *str++))
        charBounds(c, &x, &y, &minx, &miny, &maxx, &maxy);

    if(maxx >= minx) {
        *x1 = minx;
        *w  = maxx - minx + 1;
    }
    if(maxy >= miny) {
        *y1 = miny;
        *h  = maxy - miny + 1;
    }
}

void View::getTextBounds(const String &str, int16_t x, int16_t y,
                         int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h) {
    if (str.length() != 0) {
        getTextBounds(const_cast<char*>(str.c_str()), x, y, x1, y1, w, h);
    }
}



void View::drawFontPixel(uint8_t alpha, uint32_t x, uint32_t y ){
    // Adjust alpha based on the number of alpha levels supported by the font (based on bpp)
    // Note: Implemented look-up table for alpha, but made absolutely no difference in speed (T3.6)
    alpha = (uint8_t)(alpha * fontalphamx);
    uint32_t result = ((((textcolorPrexpanded - textbgcolorPrexpanded) * alpha) >> 5) + textbgcolorPrexpanded) & 0b00000111111000001111100000011111;
    Pixel(x,y,(uint16_t)((result >> 16) | result));
}

void View::drawFontBits(bool opaque, uint32_t bits, uint32_t numbits, int32_t x, int32_t y, uint32_t repeat)
{
    if (bits == 0) {
        if (opaque) {
            fillRect(x, y, numbits, repeat, textbgcolor);
        }
    } else {
        int32_t x1 = x;
        uint32_t n = numbits;
        int w;
        int bgw;

        w = 0;
        bgw = 0;

        do {
            n--;
            if (bits & (1 << n)) {
                if (bgw>0) {
                    if (opaque) {
                        fillRect(x1 - bgw, y, bgw, repeat, textbgcolor);
                    }
                    bgw=0;
                }
                w++;
            } else {
                if (w>0) {
                    fillRect(x1 - w, y, w, repeat, textcolor);
                    w = 0;
                }
                bgw++;
            }
            x1++;
        } while (n > 0);

        if (w > 0) {
            fillRect(x1 - w, y, w, repeat, textcolor);
        }

        if (bgw > 0) {
            if (opaque) {
                fillRect(x1 - bgw, y, bgw, repeat, textbgcolor);
            }
        }
    }
}

void View::drawGFXFontChar(unsigned int c) {
    // Lets do Adafruit GFX character output here as well
    if(c == '\r') 	 return;

    // Some quick and dirty tests to see if we can
    uint8_t first = gfxFont->first;
    if((c < first) || (c > gfxFont->last)) return;

    GFXglyph *glyph  = gfxFont->glyph + (c - first);
    uint8_t   w     = glyph->width,
            h     = glyph->height;

    // wonder if we should look at xo, yo instead?
    if((w == 0 ||  h == 0)  && (c != 32))   return;  // Is there an associated bitmap?

    int16_t xo = glyph->xOffset; // sic
    int16_t yo = glyph->yOffset + gfxFont->yAdvance/2;

    if(wrap && ((cursor_x + textsize_x * (xo + w)) > _width)) {
        cursor_x  = 0;
        cursor_y += (int16_t)textsize_y * gfxFont->yAdvance;
    }

    // Lets do the work to output the font character
    uint8_t  *bitmap = gfxFont->bitmap;

    uint16_t bo = glyph->bitmapOffset;
    uint8_t  xx, yy, bits = 0, bit = 0;
    //Serial.printf("DGFX_char: %c (%d,%d) : %u %u %u %u %d %d %x %x %d\n", c, cursor_x, cursor_y, w, h,
    //			glyph->xAdvance, gfxFont->yAdvance, xo, yo, textcolor, textbgcolor, _use_fbtft);Serial.flush();

    if (textcolor == textbgcolor) {

        //Serial.printf("DGFXChar: %c %u, %u, wh:%d %d o:%d %d\n", c, cursor_x, cursor_y, w, h, xo, yo);

        // NOTE: Adafruit GFX does not support Opaque font output as there
        // are issues with proportionally spaced characters that may overlap
        // So the below is not perfect as we may overwrite a small portion
        // of a letter with the next one, when we blank out...
        // But: I prefer to let each of us decide if the limitations are
        // worth it or not.  If Not you still have the option to not
        // Do transparent mode and instead blank out and blink...
        for(yy=0; yy<h; yy++) {
            uint8_t w_left = w;
            xx = 0;
            while (w_left) {
                if(!(bit & 7)) {
                    bits = bitmap[bo++];
                }
                // Could try up to 8 bits at time, but start off trying up to 4
                uint8_t xCount;
                if ((w_left >= 8) && ((bits & 0xff) == 0xff)) {
                    xCount = 8;
                    //Serial.print("8");
                    fillRect(cursor_x+(xo+xx)*textsize_x, cursor_y+(yo+yy)*textsize_y,
                             xCount * textsize_x, textsize_y, textcolor);
                } else if ((w_left >= 4) && ((bits & 0xf0) == 0xf0)) {
                    xCount = 4;
                    //Serial.print("4");
                    fillRect(cursor_x+(xo+xx)*textsize_x, cursor_y+(yo+yy)*textsize_y,
                             xCount * textsize_x, textsize_y, textcolor);
                } else if ((w_left >= 3) && ((bits & 0xe0) == 0xe0)) {
                    //Serial.print("3");
                    xCount = 3;
                    fillRect(cursor_x+(xo+xx)*textsize_x, cursor_y+(yo+yy)*textsize_y,
                             xCount * textsize_x, textsize_y, textcolor);
                } else if ((w_left >= 2) && ((bits & 0xc0) == 0xc0)) {
                    //Serial.print("2");
                    xCount = 2;
                    fillRect(cursor_x+(xo+xx)*textsize_x, cursor_y+(yo+yy)*textsize_y,
                             xCount * textsize_x, textsize_y, textcolor);
                } else {
                    xCount = 1;
                    if(bits & 0x80) {
                        if((textsize_x == 1) && (textsize_y == 1)){
                            drawPixel(cursor_x+xo+xx, cursor_y+yo+yy, textcolor);
                        } else {
                            fillRect(cursor_x+(xo+xx)*textsize_x, cursor_y+(yo+yy)*textsize_y,
                                     textsize_x, textsize_y, textcolor);
                        }
                    }
                }
                xx += xCount;
                w_left -= xCount;
                bit += xCount;
                bits <<= xCount;
            }

        }
        _gfx_last_char_x_write = 0;
    } else {
        // To Do, properly clipping and offsetting...
        // This solid background approach is about 5 time faster
        // Lets calculate bounding rectangle that we will update
        // We need to offset by the origin.

        // We are going direct so do some offsets and clipping
        int16_t x_offset_cursor = cursor_x + _originx;	// This is where the offseted cursor is.
        int16_t x_start = x_offset_cursor;  // I am assuming no negative x offsets.
        int16_t x_end = x_offset_cursor + (glyph->xAdvance * textsize_x);
        if (glyph->xAdvance < (xo + w)) x_end = x_offset_cursor + ((xo + w)* textsize_x);  // BUGBUG Overlflows into next char position.
        int16_t x_left_fill = x_offset_cursor + xo * textsize_x;
        int16_t x;

        if (xo < 0) {
            // Unusual character that goes back into previous character
            //Serial.printf("GFX Font char XO < 0: %c %d %d %u %u %u\n", c, xo, yo, w, h, glyph->xAdvance );
            x_start += xo * textsize_x;
            x_left_fill = 0;	// Don't need to fill anything here...
        }

        int16_t y_start = cursor_y + _originy + (_gfxFont_min_yOffset * textsize_y)+ gfxFont->yAdvance*textsize_y/2;  // UP to most negative value.
        int16_t y_end = y_start +  gfxFont->yAdvance * textsize_y;  // how far we will update
        int16_t y = y_start;
        //int8_t y_top_fill = (yo - _gfxFont_min_yOffset) * textsize_y;	 // both negative like -10 - -16 = 6...
        int8_t y_top_fill = (yo - gfxFont->yAdvance/2 - _gfxFont_min_yOffset) * textsize_y;

        // See if anything is within clip rectangle, if not bail
        if((x_start >= _displayclipx2)   || // Clip right
           (y_start >= _displayclipy2) || // Clip bottom
           (x_end < _displayclipx1)    || // Clip left
           (y_end < _displayclipy1))  	// Clip top
        {
            // But remember to first update the cursor position
            cursor_x += glyph->xAdvance * (int16_t)textsize_x;
            return;
        }

        // If our y_end > _displayclipy2 set it to _displayclipy2 as to not have to test both  Likewise X
        if (y_end > _displayclipy2) y_end = _displayclipy2;
        if (x_end > _displayclipx2) x_end = _displayclipx2;

        // If we get here and
        if (_gfx_last_cursor_y != (cursor_y + _originy))  _gfx_last_char_x_write = 0;

#ifdef ENABLE_ST77XX_FRAMEBUFFER
        if (_use_fbtft) {
			// lets try to output the values directly...
			uint16_t * pfbPixel_row = &_pfbtft[ y_start *_width + x_start];
			uint16_t * pfbPixel;
			// First lets fill in the top parts above the actual rectangle...
			while (y_top_fill--) {
				pfbPixel = pfbPixel_row;
				if ( (y >= _displayclipy1) && (y < _displayclipy2)) {
					for (int16_t xx = x_start; xx < x_end; xx++) {
						if ((xx >= _displayclipx1) && (xx >= x_offset_cursor)) {
							if ((xx >= _gfx_last_char_x_write) || (*pfbPixel != _gfx_last_char_textcolor))
								*pfbPixel = textbgcolor;
						}
						pfbPixel++;
					}
				}
				pfbPixel_row += _width;
				y++;
			}
			// Now lets output all of the pixels for each of the rows..
			for(yy=0; (yy<h) && (y < _displayclipy2); yy++) {
				uint16_t bo_save = bo;
				uint8_t bit_save = bit;
				uint8_t bits_save = bits;
				for (uint8_t yts = 0; (yts < textsize_y) && (y < _displayclipy2); yts++) {
					pfbPixel = pfbPixel_row;
					// need to repeat the stuff for each row...
					bo = bo_save;
					bit = bit_save;
					bits = bits_save;
					x = x_start;
					if (y >= _displayclipy1) {
						while (x < x_left_fill) {
							if ( (x >= _displayclipx1) && (x < _displayclipx2)) {
								if ((x >= _gfx_last_char_x_write) || (*pfbPixel != _gfx_last_char_textcolor))
									*pfbPixel = textbgcolor;
							}
							pfbPixel++;
							x++;

						}
				        for(xx=0; xx<w; xx++) {
				            if(!(bit++ & 7)) {
				                bits = bitmap[bo++];
				            }
				            for (uint8_t xts = 0; xts < textsize_x; xts++) {
								if ( (x >= _displayclipx1) && (x < _displayclipx2)) {
				            		if (bits & 0x80)
				            			*pfbPixel = textcolor;
				            		else if (x >= x_offset_cursor) {
										if ((x >= _gfx_last_char_x_write) || (*pfbPixel != _gfx_last_char_textcolor))
											*pfbPixel = textbgcolor;
				            		}
				            	}
								pfbPixel++;
				            	x++;	// remember our logical position...
				            }
				            bits <<= 1;
				        }
				        // Fill in any additional bg colors to right of our output
				        while (x++ < x_end) {
							if (x >= _displayclipx1) {
				        		*pfbPixel = textbgcolor;
				        	}
							pfbPixel++;
				        }
				    }
			        y++;	// remember which row we just output
					pfbPixel_row += _width;
			    }
		    }
		    // And output any more rows below us...
			while (y < y_end) {
				if (y >= _displayclipy1) {
					pfbPixel = pfbPixel_row;
					for (int16_t xx = x_start; xx < x_end; xx++) {
						if ((xx >= _displayclipx1) && (xx >= x_offset_cursor)) {
							if ((xx >= _gfx_last_char_x_write) || (*pfbPixel != _gfx_last_char_textcolor))
			        			*pfbPixel = textbgcolor;
			        	}
						pfbPixel++;
					}
				}
				pfbPixel_row += _width;
				y++;
			}

		} else
#endif
        {
            // lets try to output text in one output rectangle
            //Serial.printf("    SPI (%d %d) (%d %d)\n", x_start, y_start, x_end, y_end);Serial.flush();
            // compute the actual region we will output given


            setAddr((x_start >= _displayclipx1) ? x_start : _displayclipx1,
                    (y_start >= _displayclipy1) ? y_start : _displayclipy1,
                    x_end  - 1,  y_end - 1);
            //writecommand(ST7735_RAMWR);
            //Serial.printf("SetAddr: %u %u %u %u\n", (x_start >= _displayclipx1) ? x_start : _displayclipx1,
            //		(y_start >= _displayclipy1) ? y_start : _displayclipy1,
            //		x_end  - 1,  y_end - 1);
            // First lets fill in the top parts above the actual rectangle...
            //Serial.printf("    y_top_fill %d x_left_fill %d\n", y_top_fill, x_left_fill);
            while (y_top_fill--) {
                if ( (y >= _displayclipy1) && (y < _displayclipy2)) {
                    for (int16_t xx = x_start; xx < x_end; xx++) {
                        if (xx >= _displayclipx1) {
                            writedata16(gfxFontLastCharPosFG(xx,y)? _gfx_last_char_textcolor : (xx < x_offset_cursor)? _gfx_last_char_textbgcolor : textbgcolor);
                        }
                    }
                }
                y++;
            }
            //Serial.println("    After top fill"); Serial.flush();
            // Now lets output all of the pixels for each of the rows..
            for(yy=0; (yy<h) && (y < _displayclipy2); yy++) {
                uint16_t bo_save = bo;
                uint8_t bit_save = bit;
                uint8_t bits_save = bits;
                for (uint8_t yts = 0; (yts < textsize_y) && (y < _displayclipy2); yts++) {
                    // need to repeat the stuff for each row...
                    bo = bo_save;
                    bit = bit_save;
                    bits = bits_save;
                    x = x_start;
                    if (y >= _displayclipy1) {
                        while (x < x_left_fill) {
                            if ( (x >= _displayclipx1) && (x < _displayclipx2) ) {
                                // Don't need to check if we are in previous char as in this case x_left_fill is set to 0...
                                writedata16(gfxFontLastCharPosFG(x,y)? _gfx_last_char_textcolor :  textbgcolor);
                            }
                            x++;
                        }
                        for(xx=0; xx<w; xx++) {
                            if(!(bit++ & 7)) {
                                bits = bitmap[bo++];
                            }
                            for (uint8_t xts = 0; xts < textsize_x; xts++) {
                                if ( (x >= _displayclipx1) && (x < _displayclipx2)) {
                                    if (bits & 0x80)
                                        writedata16(textcolor);
                                    else
                                        writedata16(gfxFontLastCharPosFG(x,y)? _gfx_last_char_textcolor : (x < x_offset_cursor)? _gfx_last_char_textbgcolor : textbgcolor);
                                }
                                x++;	// remember our logical position...
                            }
                            bits <<= 1;
                        }
                        // Fill in any additional bg colors to right of our output
                        while (x < x_end) {
                            if (x >= _displayclipx1) {
                                writedata16(gfxFontLastCharPosFG(x,y)? _gfx_last_char_textcolor : (x < x_offset_cursor)? _gfx_last_char_textbgcolor : textbgcolor);
                            }
                            x++;
                        }
                    }
                    y++;	// remember which row we just output
                }
            }
            // And output any more rows below us...
            //Serial.println("    Bottom fill"); Serial.flush();
            while (y < y_end) {
                if (y >= _displayclipy1) {
                    for (int16_t xx = x_start; xx < x_end; xx++) {
                        if (xx >= _displayclipx1 ) {
                            writedata16(gfxFontLastCharPosFG(xx,y)? _gfx_last_char_textcolor : (xx < x_offset_cursor)? _gfx_last_char_textbgcolor : textbgcolor);
                        }
                    }
                }
                y++;
            }
            //writecommand_last(ST7735_NOP);
            //endSPITransaction();
        }
        // Save away info about this last char
        _gfx_c_last = c;
        _gfx_last_cursor_x = cursor_x + _originx;
        _gfx_last_cursor_y = cursor_y + _originy;
        _gfx_last_char_x_write = x_end;
        _gfx_last_char_textcolor = textcolor;
        _gfx_last_char_textbgcolor = textbgcolor;
    }

    cursor_x += glyph->xAdvance * (int16_t)textsize_x;
}

// Some fonts overlap characters if we detect that the previous
// character wrote out more width than they advanced in X direction
// we may want to know if the last character output a FG or BG at a position.
// Opaque font chracter overlap?
//	unsigned int _gfx_c_last;
//	int16_t   _gfx_last_cursor_x, _gfx_last_cursor_y;
//	int16_t	 _gfx_last_x_overlap = 0;

bool View::gfxFontLastCharPosFG(int16_t x, int16_t y) {
    GFXglyph *glyph  = gfxFont->glyph + (_gfx_c_last -  gfxFont->first);

    uint8_t   w     = glyph->width,
            h     = glyph->height;


    int16_t xo = glyph->xOffset; // sic
    int16_t yo = glyph->yOffset + gfxFont->yAdvance/2;
    if (x >= _gfx_last_char_x_write) return false; 	// we did not update here...
    if (y < (_gfx_last_cursor_y + (yo*textsize_y)))  return false;  // above
    if (y >= (_gfx_last_cursor_y + (yo+h)*textsize_y)) return false; // below


    // Lets compute which Row this y is in the bitmap
    int16_t y_bitmap = (y - ((_gfx_last_cursor_y + (yo*textsize_y))) + textsize_y - 1) / textsize_y;
    int16_t x_bitmap = (x - ((_gfx_last_cursor_x + (xo*textsize_x))) + textsize_x - 1) / textsize_x;
    uint16_t  pixel_bit_offset = y_bitmap * w + x_bitmap;

    return ((gfxFont->bitmap[glyph->bitmapOffset + (pixel_bit_offset >> 3)]) & (0x80 >> (pixel_bit_offset & 0x7)));
}

    size_t View::write(uint8_t c) {
        return write(&c, 1);
    }

    size_t View::write(const uint8_t *buffer, size_t size)
    {
        // Lets try to handle some of the special font centering code that was done for default fonts.
        if (_center_x_text || _center_y_text ) {
            int16_t x, y;
            uint16_t strngWidth, strngHeight;
            getTextBounds((uint8_t*)buffer, size, 0, 0, &x, &y, &strngWidth, &strngHeight);
            //Serial.printf("_fontwrite bounds: %d %d %u %u\n", x, y, strngWidth, strngHeight);
            // Note we may want to play with the x ane y returned if they offset some
            if (_center_x_text && strngWidth > 0){//Avoid operations for strngWidth = 0
                cursor_x -= ((x + strngWidth) / 2);
            }
            if (_center_y_text && strngHeight > 0){//Avoid operations for strngWidth = 0
                cursor_y -= ((y + strngHeight) / 2);
            }
            _center_x_text = false;
            _center_y_text = false;
        }

        size_t cb = size;
        while (cb) {
            uint8_t c = *buffer++;
            cb--;

            if (font) {
                if (c == '\n') {
                    cursor_y += font->line_space;
                    if(scrollEnable && isWritingScrollArea){
                        cursor_x  = scroll_x;
                    }else{
                        cursor_x  = 0;
                    }
                } else {
                    drawFontChar(c);
                }
            } else if (gfxFont)  {
                if (c == '\n') {
                    cursor_y += (int16_t)textsize_y * gfxFont->yAdvance;
                    if(scrollEnable && isWritingScrollArea){
                        cursor_x  = scroll_x;
                    }else{
                        cursor_x  = 0;
                    }
                } else {
                    drawGFXFontChar(c);
                }
            } else {
                if (c == '\n') {
                    cursor_y += textsize_y*8;
                    if(scrollEnable && isWritingScrollArea){
                        cursor_x  = scroll_x;
                    }else{
                        cursor_x  = 0;
                    }
                } else if (c == '\r') {
                    // skip em
                } else {
                    if(scrollEnable && isWritingScrollArea && (cursor_y > (scroll_y+scroll_height - textsize_y*8))){
                        scrollTextArea(textsize_y*8);
                        cursor_y -= textsize_y*8;
                        cursor_x = scroll_x;
                    }
                    drawChar(cursor_x, cursor_y, c, textcolor, textbgcolor, textsize_x, textsize_y);
                    cursor_x += textsize_x*6;
                    if(wrap && scrollEnable && isWritingScrollArea && (cursor_x > (scroll_x+scroll_width - textsize_x*6))){
                        cursor_y += textsize_y*8;
                        cursor_x = scroll_x;
                    }
                    else if (wrap && (cursor_x > (_width - textsize_x*6))) {
                        cursor_y += textsize_y*8;
                        cursor_x = 0;
                    }
                }
            }
        }
        return size;
    }
