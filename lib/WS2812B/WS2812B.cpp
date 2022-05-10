#include <Arduino.h>
#include <WS2812B.h>
#include <Adafruit_NeoPixel.h>

void WS2812B::setPixel(int x, int y, uint32_t color, bool additive) {
    if(x >= 0 && x < width && y >= 0 && y < height) {
        uint16_t index = xyCallback(x, y);
        setPixelColor(index, (additive ? getPixelColor(index) : 0) + color);
    }
}

void WS2812B::renderPixel(int x, int y, uint32_t color) {
    int blurry = 1.0 + blur * 4.0;
    applyBrightness(color, brightness / blurry); // brightness
    
    setPixel(x, y, color, true);
    for (double i = 1; i < blur + 1; i++) {
        applyBrightness(color, 0.7);
        setPixel(x + i, y + 0, color, true);
        setPixel(x - i, y + 0, color, true);
        setPixel(x + 0, y + i, color, true);
        setPixel(x + 0, y - i, color, true);
    }
}

void WS2812B::applyBrightness(uint32_t& color, double brightness) {
    brightness = max(0.0, brightness);
    uint32_t red = (color >> 16) & 0xff;
    uint32_t green = (color >> 8) & 0xff;
    uint32_t blue = color & 0xff;
    red = red * brightness;
    green = green * brightness;
    blue = blue * brightness;
    color = red << 16 | green << 8 | blue;
}

uint16_t WS2812B::getHeight() {
    return height;
}

uint16_t WS2812B::getWidth() {
    return width;
}

void swap(int& a, int& b) {
    int tmp = b;
    b = a;
    a = tmp;
}

void WS2812B::line(double x1, double y1, double x2, double y2, uint32_t color) {
    line((int) round(x1), (int) round(y1), (int) round(x2), (int) round(y2), color);
}

void WS2812B::line(int x1, int y1, int x2, int y2, uint32_t color) {
    if(x1 == x2 && y1 == y2) { // prevent divide by 0
        renderPixel(x1, y1, color);
        return;
    }
    int dx = x2 - x1;
    int dy = y2 - y1;
    if(abs(dx) > abs(dy)) {
        if(x2 < x1) {
            swap(x1, x2);
            swap(y1, y2);
        }
        for (int x = x1; x <= x2; x++) {
            int y = y1 + dy * (x - x1) / dx;
            renderPixel(x, y, color);
        }
    } else {
        if(y2 < y1) {
            swap(x1, x2);
            swap(y1, y2);
        }
        for (int y = y1; y <= y2; y++) {
            int x = x1 + dx * (y - y1) / dy;
            renderPixel(x, y, color);
        }
    }
}

void WS2812B::dot(int x, int y, uint32_t color) {
    renderPixel(x, y, color);
}

void WS2812B::rect(int x1, int y1, int x2, int y2, uint32_t color) {
    line(x1, y1, x1, y2, color);
    line(x1, y1, x2, y1, color);
    line(x2, y2, x1, y2, color);
    line(x2, y2, x2, y1, color);
}

void WS2812B::printSlideOverflow(char str, int x, int y, int maxX, uint32_t color, uint8_t settings) {
    char arr[2] = {str, 0};
    printSlideOverflow(arr, x, y, maxX, color, settings);
}


void WS2812B::printSlideOverflow(const char* str, int x, int y, int maxX, uint32_t color, uint8_t settings) {
    int overflow = textWidth(str, settings) - (maxX - x + 1);
    double progress = (sin(millis() / 750.0) + 1.0) / 2.0;
    int offset = round(-progress * max(0, overflow));
    print(str, x, y, color, settings, (maxX - x + 1), offset);
}

/**
 * @brief Prints a char array to screen
 * 
 * @param str The string
 * @param x absolute x pos
 * @param y absolute y pos
 * @param color color of text and underline
 * @param settings font settings see WS2812B.h
 * @param maxWidth stop printing before x + maxWidth reached
 * @param offset offset text by x pixels. Doesnt affect any bounds
 * @return int xPos for next call to print() or -1 if maxWidth got exeeded
 */
int WS2812B::print(const char* str, int x, int y, uint32_t color, uint8_t settings, int maxWidth, int offset) {
    uint8_t len = strlen(str);
    int startX = x;
    x += offset;
    int prevX = x;
    for (size_t strpos = 0; strpos < len; strpos++) {
        int nextStartX = x + boundsFromChar(str[strpos], settings);
        if(prevX < startX && nextStartX >= startX) {
            x = print(str[strpos], x, y, color, settings, startX - x, maxWidth);
        } else if(nextStartX < startX) {
            x = nextStartX;
            continue;
        } else {
            x = print(str[strpos], x, y, color, settings, 0, maxWidth);
        }
        maxWidth -= x - max(startX, prevX);
        prevX = x;
        if(maxWidth < 0) return -1;
    }
    return x;
}

int WS2812B::boundsFromChar(char digit, uint8_t settings) {
    if(digit < ' ' && digit != '\t') {
        return 0;
    }
    if((settings & UPPERCASE) && digit >= 'a' && digit <= 'z') {
        digit -= 32;
    }
    uint8_t index = digit - ASCII_OFFSET;
    uint8_t width;
    const uint8_t* letter;
    if(settings & FONT_SIZE_SMALL) {
        letter = fontSmall[index];
        width = FONT_SMALL_WIDTH;
    } else {
        letter = fontNormal[index];
        width = FONT_NORMAL_WIDTH;
    }
    if(digit == '\t') {
        return 2 * width; // tab
    }
    if(digit == ' ') {
        return width / 2;
    }
    if(!(settings & MONOSPACE)) while(!letter[width - 1]) width--;
    // if(digit == '.' || digit == ',') {
    //     width--;
    // }
    return width + (settings & SPACING);
}

int WS2812B::textWidth(const char* str, uint8_t settings) {
    uint8_t len = strlen(str);
    int width = 0;
    for (size_t strpos = 0; strpos < len; strpos++) {
        width += boundsFromChar(str[strpos], settings);
    }
    return width;
}

int WS2812B::print(char digit, int xPos, int yPos, uint32_t color, uint8_t settings, uint8_t xStart, uint8_t maxWidth) {
    if(digit < ' ' && digit != '\t') {
        return xPos;
    }
    if((settings & UPPERCASE) && digit >= 'a' && digit <= 'z') {
        digit -= 32;
    }
    uint8_t index = digit - ASCII_OFFSET;
    uint8_t width, height;
    const uint8_t* letter;
    if((settings & FONT_SIZE_SMALL) > 0) {
        letter = fontSmall[index];
        width = FONT_SMALL_WIDTH;
        height = FONT_SMALL_HEIGHT;
    } else {
        letter = fontNormal[index];
        width = FONT_NORMAL_WIDTH;
        height = FONT_NORMAL_HEIGHT;
    }

    if(digit == '\t') {
        return xPos + 2 * width; // tab
    }
    if(digit == ' ') {
        return xPos + (width / 2);
    }

    if(!(settings & MONOSPACE)) while(!letter[width - 1]) width--;

    for (int x = xStart; x < min(width, maxWidth); x++) {
        uint8_t column = letter[width - x - 1];
        for (int y = 0; y < height; y++) {
            if(column & (0b00000001 << (height - y - 1))) {
                renderPixel(x + xPos, y + yPos, color);
            }
        }
    }
    if(settings & UNDERLINE) {
        for (int x = xStart; x < min(width + (settings & SPACING), (int) maxWidth); x++) {
            renderPixel(x + xPos, yPos + height + 1, color);
        }
    }
    // if(digit == '.' || digit == ',') {
    //     width--;
    // }
    return xPos + width + (settings & SPACING);
}

void WS2812B::setBlur(double blur) {
    this->blur = max(blur, 0.0);
}

void WS2812B::setBrightness(double brightness) {
    this->brightness = max(brightness, 0.0);
}