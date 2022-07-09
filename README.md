[![teensy-gfx-arm](https://github.com/newdigate/teensy-gfx/workflows/teensy-gfx-arm/badge.svg)](https://github.com/newdigate/teensy-gfx/actions)
[![teensy-gfx-linux](https://github.com/newdigate/teensy-gfx/workflows/teensy-gfx-linux/badge.svg)](https://github.com/newdigate/teensy-gfx/actions)
[![basic-example-linux](https://github.com/newdigate/teensy-gfx/workflows/basic-example/badge.svg)](https://github.com/newdigate/teensy-gfx/actions)

# teensy gfx
A portable c++17 TFT display graphics abstractions library based on [ST7735_t3](https://github.com/PaulStoffregen/ST7735_t3) and [adafruit-GFX](https://github.com/adafruit/Adafruit-GFX-Library). 

Adds ```View``` class with adafruit gfx drawing routines used by other libraries of mine to reduce code duplication

## implement a custom RGB565 graphics device (or view)
* implement a class which inherits from ```View``` 
* override ```void Pixel(int16_t x, int16_t y, uint16_t color)```

## please note
  * this library is an **experimental** abstraction adaption and many functions may not be supported.   

## approximate interface
```c++
class View {
public:
    void        fillScreen(uint16_t color);
    void        drawPixel(int16_t x, int16_t y, uint16_t color);
    void        drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
    void        drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
    void        fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void        fillWindow(uint16_t color);
    void        setRotation(uint8_t r);
    void        invertDisplay(bool i);
    void        setRowColStart(uint16_t x, uint16_t y);
    uint16_t    rowStart();
    uint16_t    colStart();

    int16_t     width();
    int16_t     height();
    uint8_t     getRotation();

    void        drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
    void        drawCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, uint16_t color);
    void        fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
    void        fillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, int16_t delta, uint16_t color);
    void        drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
    void        fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
    void        drawRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color);
    void        fillRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color);
    void        drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color);
    void        drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
    void        drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void        drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size_x, uint8_t size_y);
    void        drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size);
    void        setCursor(int16_t x, int16_t y, bool autoCenter=false);
    void        getCursor(int16_t *x, int16_t *y);
    void        setTextColor(uint16_t c);
    void        setTextColor(uint16_t c, uint16_t bg);
    void        setTextSize(uint8_t sx, uint8_t sy);
    void        setTextSize(uint8_t s);
    uint8_t     getTextSizeX();
    uint8_t     getTextSizeY();
    uint8_t     getTextSize();
    void        setTextWrap(bool w);
    bool        getTextWrap();

    int16_t     getCursorX(void) ;
    int16_t     getCursorY(void) ;
    void        setFont(const ILI9341_t3_font_t &f);
    void        setFont(const GFXfont *f = NULL);
    void        setFontAdafruit(void);
    void        drawFontChar(unsigned int c);
    void        drawGFXFontChar(unsigned int c);

    void        getTextBounds(const uint8_t *buffer, uint16_t len, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h);
    void        getTextBounds(const char *string, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h);
    void        getTextBounds(const String &str, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h);
    int16_t     strPixelLen(const char * str);

    int16_t     drawNumber(long long_num,int poX, int poY);
    int16_t     drawFloat(float floatNumber,int decimal,int poX, int poY);
    int16_t     drawString(const String& string, int poX, int poY);
    int16_t     drawString1(char string[], int16_t len, int poX, int poY);

    void        setTextDatum(uint8_t datum);

    void        setScrollTextArea(int16_t x, int16_t y, int16_t w, int16_t h);
    void        setScrollBackgroundColor(uint16_t color);
    void        enableScroll(void);
    void        disableScroll(void);
    void        scrollTextArea(uint8_t scrollSize);
    void        resetScrollBackgroundColor(uint16_t color);
    uint16_t    readPixel(int16_t x, int16_t y);
    void        readRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t *pcolors);

    void        setOrigin(int16_t x = 0, int16_t y = 0) ;
    void        getOrigin(int16_t* x, int16_t* y);
    void        setClipRect(int16_t x1, int16_t y1, int16_t w, int16_t h);
    void        setClipRect();
    void        writeRect(int16_t x, int16_t y, int16_t w, int16_t h, const uint16_t *pcolors);

    void        setFrameBuffer(uint16_t *frame_buffer);
    uint8_t     useFrameBuffer(bool b);  
    void        freeFrameBuffer();                              
    void        updateScreen();                            
    bool        updateScreenAsync(bool update_cont = false);                  
    void        waitUpdateAsyncComplete() ;
    void        endUpdateAsync() ;
    void        dumpDMASettings();
    void        charBounds(char c, int16_t *x, int16_t *y, int16_t *minx, int16_t *miny, int16_t *maxx, int16_t *maxy);
    bool        gfxFontLastCharPosFG(int16_t x, int16_t y);
    void        drawFontBits(bool opaque, uint32_t bits, uint32_t numbits, int32_t x, int32_t y, uint32_t repeat);
    void        drawFontPixel( uint8_t alpha, uint32_t x, uint32_t y );
    uint32_t    fetchpixel(const uint8_t *p, uint32_t index, uint32_t x);
    void        setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
    void        pushColor(uint16_t color, bool last_pixel=false);
    void        setAddr(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
    uint16_t    Color565(uint8_t r, uint8_t g, uint8_t b);
    size_t      write(uint8_t c) ;
    size_t      write(const uint8_t *buffer, size_t size);
}

```
