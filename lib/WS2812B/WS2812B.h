#pragma once
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <buzzerMusic.h>
#include "fonts.h"
#include <Preferences.h>


#define ASCII_OFFSET 33

#define FONT_SMALL_WIDTH 4
#define FONT_SMALL_HEIGHT 5

#define FONT_NORMAL_WIDTH 5
#define FONT_NORMAL_HEIGHT 8

#define UNDERLINE          0b10000000
#define MONOSPACE          0b01000000
#define UPPERCASE          0b00100000
#define FONT_SIZE_SMALL    0b00010000
#define SPACING            0b00000111

#define DEFAULT_FONT_SETTINGS UPPERCASE + 1 // no underline, no monospace, no uppercase, 1 spacing

#define STORAGE_CHECK 11

static Preferences preferences;
static bool storageInitiated = false;
static int storageId = 0;

static void resetStorage() {
    preferences.begin("1", false);
    preferences.putInt("init", STORAGE_CHECK + 1);
}

static void beginStorage() {
  preferences.begin("1", false);
  storageInitiated = preferences.getInt("init", 0) == STORAGE_CHECK;
  if(!storageInitiated) {
      preferences.putInt("init", STORAGE_CHECK);
      Serial.println("first Storage init");
      char ssid[] = "hoelle";
      char pwd[] = "13pe07al19ti58";
      preferences.putBytes("ssid", ssid, sizeof(ssid) + 1);
      preferences.putBytes("pwd", pwd, sizeof(pwd) + 1);
  }
}

static int getStorageId() {
    storageId++;
    return storageId - 1;
}

static void putDoubleVal(int id, double val) {
    char str[5];
    sprintf(str, "%i", id);
    preferences.putDouble(str, val);
}

static void putBoolVal(int id, bool val) {
    char str[5];
    sprintf(str, "%i", id);
    preferences.putBool(str, val);
}

static double getDoubleVal(int id, double defaultVal) {
    char str[5];
    sprintf(str, "%i", id);
    if(!storageInitiated) {
        preferences.putDouble(str, defaultVal);
        return defaultVal;
    }
    return preferences.getDouble(str);
}

static bool getBoolVal(int id, bool defaultVal) {
    char str[5];
    sprintf(str, "%i", id);
    if(!storageInitiated) {
        preferences.putBool(str, defaultVal);
        return defaultVal;
    }
    return preferences.getBool(str);
}

/**
 * @brief Convert time to string. max length: 12
 */
static void timeToStr(uint32_t msTime, char* hStr, char* mStr, char* sStr, char* msStr, bool oneMsDigit) {
  uint16_t ms = msTime % 1000;
  uint32_t seconds = (msTime / 1000) % 60;
  uint16_t minutes = (msTime / 60000) % 60;
  uint16_t hours = msTime / 36000000;
  if(hours > 0) {
    sprintf(hStr, "%i", hours);
  }
  if(minutes > 0) {
    sprintf(mStr, "%i", minutes);
  }
  if(seconds < 10) {
    sprintf(sStr, "0%i", seconds);
  } else {
    sprintf(sStr, "%i", seconds);
  }
  if(ms == 0) {
    sprintf(msStr, "000");
  } else if(ms < 10) {
    sprintf(msStr, "00%i", ms);
  } else if(ms < 100) {
    sprintf(msStr, "0%i", ms);
  } else {
    sprintf(msStr, "%i", ms);
  }
  if(oneMsDigit) {
    msStr[1] = 0;
  }
}

typedef int (*xyToindex)(int x, int y);

class WS2812B : public Adafruit_NeoPixel {
public:
    WS2812B(uint8_t pin, uint16_t width, uint16_t height, xyToindex xyCallback) :
        Adafruit_NeoPixel(width * height, pin, NEO_GRB + NEO_KHZ800),
        width(width),
        height(height),
        xyCallback(xyCallback) {}

    void setPixel(int x, int y, uint32_t color, bool additive = false);
    int print(const char* str, int x, int y, uint32_t color, uint8_t settings = DEFAULT_FONT_SETTINGS, int maxWidth = 1000, int offset = 0);
    void printSlideOverflow(const char* str, int x, int y, int maxX, uint32_t color, uint8_t settings = DEFAULT_FONT_SETTINGS);
    void printSlideOverflow(char str, int x, int y, int maxX, uint32_t color, uint8_t settings);
    int print(char digit, int x, int y, uint32_t color, uint8_t settings = DEFAULT_FONT_SETTINGS, uint8_t xStart = 0, uint8_t maxWidth = 10);
    void line(double x1, double y1, double x2, double y2, uint32_t color);
    void line(int x1, int y1, int x2, int y2, uint32_t color);
    void rect(int x1, int y1, int x2, int y2, uint32_t color);
    void dot(int x, int y, uint32_t color);

    static int textWidth(const char* str, uint8_t settings = DEFAULT_FONT_SETTINGS);
    static int boundsFromChar(char digit, uint8_t settings);
    
    uint16_t getWidth();
    uint16_t getHeight();

    void setBlur(double blur);
    void setBrightness(double brightness);

    double brightness = 1; // multiplicator

private:
    double blur = 0; // pixels of spread
    uint16_t width;
    uint16_t height;
    constexpr static int charWidth = 4;
    constexpr static int charHeight = 5;
    xyToindex xyCallback;

    void renderPixel(int x, int y, uint32_t color);
    void applyBrightness(uint32_t& color, double brightness);
};

static void renderTime(int x, int y, uint32_t ms, bool oneMsDigit, WS2812B* display) {
  char hStr[10] = "\0";
  char mStr[3]  = "\0";
  char sStr[3];
  char msStr[4];
  x+=2;
  timeToStr(ms, hStr, mStr, sStr, msStr, oneMsDigit);
  if(*hStr) {
    x = display->print(hStr, x - 2, y + 1, 0xFFFF00, FONT_SIZE_SMALL + MONOSPACE + 1);
    x = display->print(":", x- 1, y, 0xAA0000);
  }
  if(*mStr) {
    x = display->print(mStr, x - 2, y + 1, 0x00FFFF, FONT_SIZE_SMALL + MONOSPACE + 1);
    x = display->print(":", x - 1, y, 0xAA0000);
  }
  x = display->print(sStr, x - 1, y, 0x00FF00, 1);
  x = display->print(".", x- 1, y, 0xAA0000, 0);
  x = display->print(msStr[0], x, y, 0xFF00FF, 1);
  x = display->print(msStr + 1, x, y + 3, 0xFF00FF, FONT_SIZE_SMALL + 1);
}


namespace Event {
    enum Event_t {
        PRESS_BUTTON,
        RELEASE_BUTTON,
        LONG_RELEASE_BUTTON,
        SCROLL_UP,
        SCROLL_DOWN,
        NONE
    };
}

struct MenuItem {
    bool focused = false;
    /**
     * @brief Renders this MenuItem
     * 
     * @param x position
     * @param y poyition
     * @param display display to render on
     * @return int next free y pos after this MenuItem
     */
    virtual int renderItem(int x, int y, WS2812B* display) = 0;
    /**
     * @brief Handles an event
     * 
     * @param event Event that occured
     * @return true if the this item requests focus
     * @return false if this item doesnt have focus now
     */
    virtual bool handleEvent(Event::Event_t &event) = 0;
};

struct MenuRenderer : public MenuItem {

    const char* name;
    int status = 0; // 0=active, -1=hidden on left, 1 = hidden to the right
    bool renderContent;
    bool showItem = true;
    bool showBackBtn = true;

    MenuRenderer() : name(nullptr), status(0), renderContent(true), showBackBtn(false) {}

    MenuRenderer(const char* name) : name(name), status(1), renderContent(false), showBackBtn(true) {}

    MenuItem* subMenus[50];
    int size = 0;
    int activeIndex = 0;

    bool exitActive = false;

    int itemHeight = 8;
    int subTargetY = 0;
    double subY = 0;
    double subX = 0;
    double lpf = 0.37;
    bool willGoRight = false;
    bool returnToTop = true;
    bool backOnLongPress = false;

    bool animStarted = false;

    uint32_t hideTime = 0;

    void addItem(MenuItem* item, uint16_t index = 10000) {
        if(size == 50) return;
        if(index >= size) index = size;
        size++;
        for (size_t i = size + 1; i > index; i--) {
            subMenus[i] = subMenus[i - 1];
        }
        subMenus[index] = item;
    }

    void empty() {
        // for (size_t i = 0; i < size; i++) {
        //     delete subMenus[i];
        // }
        size = 0;
    }

    void removeItem(uint16_t index) {
        if(index >= size) return;
        removeItem(subMenus[index]);
    }

    void removeItem(MenuItem* item) {
        bool found = false;
        for (size_t i = 0; i < size; i++) {
            if(subMenus[i] == item) {
                // delete subMenus[i];
                found = true;
                size--;
                if(activeIndex >= size) {
                    activeIndex = max(0, activeIndex - 1);
                    scrollTo(activeIndex);
                }
            }
            if(found && i < size) {
                subMenus[i] = subMenus[i + 1];
            }
        }
    }

    void scrollTo(size_t index) {
        if(index >= size) return;
        activeIndex = index;
        subTargetY = -activeIndex * itemHeight;
    }

    int renderItem(int x, int y, WS2812B* display) {
        if(showItem) {
            int x2 = display->print(">", x, y + 1, 0x009900, FONT_SIZE_SMALL);
            display->printSlideOverflow(name, x2, y + 1, x + 31, 0x999999, FONT_SIZE_SMALL + 1);
        }
        if(renderContent) {
            render(x + 32, y, display);
        }
        return y + 8;
    }

    int render(int x, int y, WS2812B* display) {
        subY = subY * (1.0 - lpf) + subTargetY * lpf;
        subX = subX * (1.0 - lpf) + (status * 32.0) * lpf;
        if((abs(subX - (status * 32.0)) < 1) && animStarted) {
            xAnimDone();
        }
        x += round(subX);
        y += round(subY);
        for (int i = 0; i < size; i++) {
            y = subMenus[i]->renderItem(x, y, display);
        }
        if(exitActive) { // Exit button
            int x2 = display->print("<", x + 0, y + 1, 0x990000, FONT_SIZE_SMALL + 5);
            display->print("BACK", x2, y + 1, 0x999999, FONT_SIZE_SMALL + 1);
        }
        return y;
    }

    void open(bool fromRight) {
        status = 0;
        if(fromRight && returnToTop) {
            subTargetY = 0;
            subY = 0;
            activeIndex = 0;
        }
        renderContent = true;
        if(activeIndex >= size) {
            activeIndex = 0;
        }
    }

    void goLeft() {
        if(status != 0) return;
        status = -1;
        animStarted = true;
        bgPlayer.playBeepOn();
    }

    void goRight() {
        if(status != 0) return;
        status = 1;
        showItem = true;
        animStarted = true;
        bgPlayer.playBeepOff();
    }

    void xAnimDone() {
        renderContent = status <= 0; // render if status is 0 or hidden to the left(still render children)
        animStarted = false;
        showItem = status >= 0;
    }

    bool handleEvent(Event::Event_t& event) {
        if(backOnLongPress && status == 0 && event == Event::LONG_RELEASE_BUTTON) {
            goRight();
            return false;
        }
        if(size == 0) {
            bgPlayer.playBeepBeep();
            return false;
        }
        if(status <= 0 && size > 0) {
            if(!exitActive && subMenus[activeIndex]->handleEvent(event)) {
                goLeft();
                return true;
            } else if(status == -1) {
                open(false);
                return true;
            }
            if(subMenus[activeIndex]->focused) {
                return true;
            }
        }

        switch(event) {
            case Event::LONG_RELEASE_BUTTON: {
                if(status == 0 && showBackBtn) {
                    goRight();
                    return false;
                }
                if(!showBackBtn) {
                    scrollTo(0);
                }
                break;
            }
            case Event::RELEASE_BUTTON: {
                if(exitActive) {
                    goRight();
                    exitActive = false;
                    return false;
                }
                if(status == 1) {
                    open(true);
                    return true;// hide parent
                }
                break;
            } case Event::PRESS_BUTTON: {
                break;
            } case Event::SCROLL_UP: {
                if(status == 0) {
                    if(exitActive) {
                        exitActive = false;
                    } else {
                        activeIndex--;
                    }
                    activeIndex = max(0, activeIndex);
                    subTargetY = -activeIndex * itemHeight;
                }
                break;
            } case Event::SCROLL_DOWN: {
                if(status == 0) {
                    activeIndex++;
                    exitActive = activeIndex == size;
                    if(!showBackBtn) {
                        activeIndex = min(size -1, activeIndex);
                        exitActive = false;
                    }
                    subTargetY = -activeIndex * itemHeight;
                    activeIndex = min(size -1, activeIndex);
                }
                break;
            }
        }
        if(willGoRight) {
            willGoRight = false;
            goRight();
        }
        return status == 0; // request focus as long as status = showed
    }
};

typedef void (*callback)();
typedef void (*deleteCallback) (uint16_t id);

struct Button : public MenuItem {
    const char* name;
    callback callbackFunc;
    bool confirmNeeded;
    bool confirm = false;

    bool isDelete = false;
    MenuRenderer* parent;
    uint16_t id;
    deleteCallback delectCallback;
    uint32_t color;

    Button(const char* name, callback callbackFunc, bool confirmNeeded = false, uint32_t color = 0x999999) : name(name),
    callbackFunc(callbackFunc),
    confirmNeeded(confirmNeeded),
    color(color) {}

    int renderItem(int x, int y, WS2812B* display) {
        if(confirm) {
            display->print("DELETE?", x + 1, y + 1, 0x993300, FONT_SIZE_SMALL + UNDERLINE + 1);
        } else {
            display->rect(x, y, x+1, y+7, 0x779900);
            display->printSlideOverflow(name, x + 3, y + 1, 31, color, FONT_SIZE_SMALL + 1);
        }
        return y + 8;
    }

    void action() {
        if(isDelete) {
            parent->removeItem(this);
            delectCallback(id);
        }
        if(callbackFunc) {
            callbackFunc();
        }
    }

    bool handleEvent(Event::Event_t& event) {
        if(event == Event::RELEASE_BUTTON) {
            if(!confirmNeeded) {
                action();
                return false;
            }
            if(confirm) {
                confirm = false;
                action();
                return false;
            }
            confirm = true;
            return false;
        }
        if(event != Event::PRESS_BUTTON) {
            confirm = false;
        }
        return false;
    }
};

struct DeleteButton : public Button {

    DeleteButton(const char* name, bool confirm, MenuRenderer* parent, uint16_t id, deleteCallback delectCallback) : Button(name, nullptr, confirm) {
        isDelete = true;
        this->parent = parent;
        this->id = id;
        this->delectCallback = delectCallback;
    }

    int renderItem(int x, int y, WS2812B* display) {
        if(confirm) {
            return Button::renderItem(x, y, display);
        }
        x = display->print("X", x + 1, y + 1, 0x990000, FONT_SIZE_SMALL + 2);
        display->printSlideOverflow(name, x, y + 1, 31, 0x999999, FONT_SIZE_SMALL + 1);
        return y + 8;
    }
};

struct InputItem : public MenuItem {
    const char* name;
    double minVal;
    double maxVal;
    double step;
    double value;
    bool integer;
    int storageId;
    double lastVal;

    InputItem(const char* name, double minVal, double maxVal, double step, double value = 0) :
        name(name),
        minVal(minVal),
        maxVal(maxVal),
        step(step),
        value(value),
        lastVal(value) {
            integer = floor(step) == ceil(step);
            storageId = getStorageId();
            readStorage();
        }
    
    void readStorage() {
        value = getDoubleVal(storageId, value);
    }

    void writeStorage() {
        putDoubleVal(storageId, value);
    }

    int renderItem(int x, int y, WS2812B* display) {
        char charNum[10];
        if(integer) {
            sprintf(charNum, "%i", (int) value);
        } else {
            sprintf(charNum, "%.1f", value);
        }
        uint32_t color = focused ? 0x0077FF : 0x0044AA;
        x = display->print(charNum, x, y + 1, color, FONT_SIZE_SMALL + focused * UNDERLINE + 1);
        display->printSlideOverflow(name, x, y + 1, 31, 0x999999, FONT_SIZE_SMALL + 1);
        return y + 8;
    }

    uint32_t lastScroll = 0;

    bool handleEvent(Event::Event_t& event) {
        switch(event) {
            case Event::LONG_RELEASE_BUTTON: {
                if(focused) {
                    value = lastVal;
                    focused = false;
                    bgPlayer.playBeepOff();
                    event = Event::NONE;
                }
                break;
            }
            case Event::RELEASE_BUTTON: {
                focused = !focused;
                if(focused) {
                    lastVal = value;
                    bgPlayer.playBeepOn();
                } else {
                    bgPlayer.playBeepOff();
                    writeStorage();
                }
                break;
            }
            case Event::PRESS_BUTTON: {
                break;
            } case Event::SCROLL_UP: {
                if(focused) {
                    value += (millis() - lastScroll < 75) ? step * 10 : step;
                    lastScroll = millis();
                    if(value >= maxVal) {
                        value = maxVal;
                        bgPlayer.playBeepBeep();
                    }
                }
                break;
            } case Event::SCROLL_DOWN: {
                if(focused) {
                    value -= (millis() - lastScroll < 75) ? step * 10 : step;
                    lastScroll = millis();
                    if(value <= minVal) {
                        value = minVal;
                        bgPlayer.playBeepBeep();
                    }
                }
                break;
            }
        }
        return false;
    }
};

struct CheckBox : public MenuItem {

    const char* name;
    bool checked;
    uint16_t storageId;

    CheckBox(const char name[], bool checked = false) : name(name), checked(checked) {
        storageId = getStorageId();
        readStorage();
    }
    
    void readStorage() {
        checked = getBoolVal(storageId, checked);
    }

    void writeStorage() {
        putBoolVal(storageId, checked);
    }

    int renderItem(int x, int y, WS2812B* display) {
        uint32_t checkColor = checked ? 0x009900 : 0x990000;
        display->rect(x, y + 1, x + 1, y + 5, checkColor);
        display->printSlideOverflow(name, x + 3, y + 1, 31, 0x999999, FONT_SIZE_SMALL + 1);
        return y + 8;
    }

    bool handleEvent(Event::Event_t &event) {
        switch(event) {
            case Event::RELEASE_BUTTON: {
                checked = !checked;
                if(checked) {
                    bgPlayer.playBeepOn();
                } else {
                    bgPlayer.playBeepOff();
                }
                writeStorage();
                break;
            }
            case Event::PRESS_BUTTON: {
                break;
            } case Event::SCROLL_UP: {
                break;
            } case Event::SCROLL_DOWN: {
                break;
            }
            default: {}
        }
        return false;
    }
};

struct TextItem : public MenuItem {

    const char* name;
    uint32_t color;

    TextItem(const char* name, uint32_t color = 0xCCCCCC) : name(name), color(color) {}

    int renderItem(int x, int y, WS2812B* display) {
        // display->rect(x, y + 1, x + 1, y + 5, checkColor);
        display->printSlideOverflow(name, x + 1, y + 1, 31, color, FONT_SIZE_SMALL + 1);
        return y + 8;
    }

    bool handleEvent(Event::Event_t &event) {
        return false;
    }
};

struct LapItem : public MenuItem {

    int lap;
    uint32_t ms;

    LapItem(int lap, uint32_t ms) : lap(lap), ms(ms) {}

    int renderItem(int x, int y, WS2812B* display) {
        char lapStr[5];
        sprintf(lapStr, "%i", lap);
        x = display->print(lapStr, x, y + 1, 0x999999, FONT_SIZE_SMALL + 1);
        display->line(x, y+1, x, y+6, 0x444444);
        x++;
        renderTime(x, y, ms, false, display);
        return y + 8;
    }

    bool handleEvent(Event::Event_t &event) {
        return false;
    }
};

template<class T>
struct Option;

template<class T>
struct Select : public MenuRenderer {
    T value;
    int selectedIndex = -1;

    Select(const char* name);

    void addOption(const char* name, T value);
    void deselectAll();
    int renderItem(int x, int y, WS2812B* display);
    void selectOption(Option<T>* option, bool initWillGoRight = true);
    void selectOption(uint16_t index);
    int render(int x, int y, WS2812B* display);
    void removeByValue(T value);
    bool isSelected();
};

template<class T>
struct Option : public MenuItem {
    const char* name;
    T value;
    Select<T>* selectGroup;
    bool selected = false;

    Option(const char* name, T value, Select<T>* selectGroup) : name(name), value(value), selectGroup(selectGroup) {}

    int renderItem(int x, int y, WS2812B* display) {
        if(selected) {
            display->rect(x, y + 1, x + 1, y + 5, 0x009900);
            display->rect(x + 2, y + 2, x + 2, y + 4, 0x009900);
        }
        display->printSlideOverflow(name, x + (selected ? 4 : 1), y + 1, 31, 0x999999, FONT_SIZE_SMALL + 1);
        return y + 8;
    }

    bool handleEvent(Event::Event_t &event) {
        if(event == Event::RELEASE_BUTTON) {
            selectGroup->selectOption(this);
        }
        return false;
    }
};

template<class T>
Select<T>::Select(const char* name) : MenuRenderer(name) {
    showBackBtn = false;
    returnToTop = false;
    backOnLongPress = true;
}

template<class T>
bool Select<T>::isSelected() {
    return selectedIndex >= 0;
}

template<class T>
int Select<T>::render(int x, int y, WS2812B* display) {
    MenuRenderer::render(x, y, display);
    if(selectedIndex >= 0) {
        if(activeIndex > selectedIndex) {
            display->line(x, y, x + 1, y, 0x009900);
        }
        if(activeIndex < selectedIndex) {
            display->line(x, y + 7, x + 1, y + 7, 0x009900);
        }
    }
    return y;
}

template<class T>
void Select<T>::removeByValue(T value) {
    for (size_t i = 0; i < size; i++) {
        Option<T>* o = (Option<T>*) subMenus[i];
        if(o->value == value) {
            removeItem(o);
            if(selectedIndex == i) {
                selectedIndex = -1;
            }
            if(selectedIndex > i) {
                selectedIndex--;
            }
            return;
        }
    }
}

template<class T>
void Select<T>::addOption(const char* name, T value) {
    Option<T>* option = new Option<T>(name, value, this);
    addItem(option);
}

template<class T>
void Select<T>::selectOption(uint16_t index) {
    if(index >= size) return;
    selectOption(((Option<T>*) subMenus[index]));
}

template<class T>
void Select<T>::selectOption(Option<T>* option, bool initWillGoRight) {
    for (size_t i = 0; i < size; i++) {
        ((Option<T>*) subMenus[i])->selected = false;
    }
    value = option->value;
    option->selected = true;
    willGoRight = initWillGoRight;
    for (size_t i = 0; i < size; i++) {
        if(subMenus[i] == option) {
            selectedIndex = i;
        }
    }
}

template<class T>
int Select<T>::renderItem(int x, int y, WS2812B* display) {
    int startx = x;
    display->line(x, y+1, x+1,y+1, 0x555555);
    display->rect(x, y+3, x+1,y+4, 0x009900);
    display->line(x, y+6, x+1,y+6, 0x555555);
    x += 3;
    for (size_t i = 0; i < size; i++) {
        Option<T>* option = ((Option<T>*) subMenus[i]);
        if(option->selected) {
            x = display->print(option->name[0], x, y + 1, 0x222299, FONT_SIZE_SMALL + 1);
            break;
        }
    }
    display->printSlideOverflow(name, x, y + 1, startx + 31, 0x999999, FONT_SIZE_SMALL + 1);
    if(renderContent) {
        render(startx + 32, y, display);
    }
    return y + 8;
}

typedef void (*TextCallback)(const char*);

struct TextInput : public MenuItem {
    const char* name;
    TextCallback callback;
    char input[15];
    char nextChar = '`'; // @ = empty char
    uint8_t inputSize = 0;

    bool focused = false;
    bool reset = true;
    uint32_t lastPress = 0;

    TextInput(const char* name, TextCallback callback) : name(name), callback(callback) {
        input[0] = '\0';
    }

    int renderItem(int x, int y, WS2812B* display) {
        int startX = x;
        if(focused) {
            render(x + 32, y, display);
        }
        uint8_t underline = millis() % 1500 < 650 ? UNDERLINE : 0;
        x = display->print("T", x + 1, y + 1, 0x999944, FONT_SIZE_SMALL + underline + 2);
        display->printSlideOverflow(name, x, y + 1, startX + 31, 0x999999, FONT_SIZE_SMALL + 1);
        return y + 8;
    }

    void render(int x, int y, WS2812B* display) {
        int width = WS2812B::textWidth(input, FONT_SIZE_SMALL + 1);
        int diff = min(0, (27 - x) - (width));
        x = display->print(input, x + diff, y + 1, 0x999999, FONT_SIZE_SMALL + 1);
        uint8_t underline = millis() % 1500 < 750 ? UNDERLINE : 0;
        display->print(nextChar, x, y + 1, 0x999999, FONT_SIZE_SMALL + underline);
    }

    void focus() {
        if(reset) {
            inputSize = 0;
            input[0] = 0;
        }
        nextChar = '`';
        focused = true;
    }

    void unfocus() {
        focused = false;
    }

    bool handleEvent(Event::Event_t &event) {
        switch(event) {
            case Event::LONG_RELEASE_BUTTON: {
                if(focused) {
                    if(inputSize > 0) {
                        uint8_t len = strlen(input) + 1;
                        char* valClone = new char[len];
                        for (size_t i = 0; i < len; i++) {
                            valClone[i] = input[i];
                        }
                        valClone[len] = 0;
                        callback(valClone);
                    }
                    unfocus();
                    return false;
                }
                break;
            }
            case Event::RELEASE_BUTTON: {
                if(!focused) {
                    focus();
                    return true;
                }
                if(millis() - lastPress < 400) {
                    inputSize = max(0, inputSize - 2);
                    input[inputSize] = 0;
                    bgPlayer.playBeepOff();
                    return true;
                }
                lastPress = millis();
                if(strlen(input) >= 14) {
                    bgPlayer.playBeepBeep();
                    return true;
                }
                if(nextChar == '`') {
                    if(inputSize == 0) {
                        unfocus();
                        return false;
                    }
                    input[inputSize - 1] = 0;
                    inputSize--;
                    bgPlayer.playBeepOff();
                    return true;
                }
                input[inputSize] = nextChar;
                inputSize++;
                input[inputSize] = '\0';
                // nextChar = '@';
                bgPlayer.playBeepOn();
                break;
            } case Event::PRESS_BUTTON: {
                break;
            } case Event::SCROLL_UP: {
                if(focused) {
                    nextChar = max((uint8_t)'!', (uint8_t) (nextChar));
                    nextChar = min((uint8_t)'~', (uint8_t) (nextChar + 1));
                }
                break;
            } case Event::SCROLL_DOWN: {
                if(focused) {
                    nextChar = max((uint8_t)'!', (uint8_t) (nextChar - 1));
                    nextChar = min((uint8_t)'~', (uint8_t) (nextChar));
                }
                break;
            }
        }
        return focused;
    }
};