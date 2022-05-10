#include <Arduino.h>
#include <MusicBuzzer.h>
#include <buzzerMusic.h>
#include <WS2812B.h>
#include <ky40.h>
#include <WiFi.h>
#include <HTTPClient.h>

// #include "esp_netif.h"
#include <esp_wifi.h>
#include <sdkconfig.h>
#include <M18.h>
#include <esp_now.h>
                                                 
const uint8_t broadcastAddress[6] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};


HTTPClient http;

#define IS_LASER
#define LASER_IS_NPN true
// #define LASER_IS_NPN false

#define DISPLAY_PIN 16
#define LASER_PIN   34 // 120 Ohm pull down resistor Black Wire!
#define BAT_PIN     32 // voltage divider 1 / 1
#define BAT_LED_PIN 27
#define PIN_A       19
#define PIN_B       05
#define PIN_BUTTON  23

#define INTRO_LINES 100
#define INTRO_TIME 3000

#define GUI_FPS 120

#define BAT_LED_COUNT 4

M18 m18Laser(LASER_PIN, LASER_IS_NPN);
// #ifdef IS_LASER
Adafruit_NeoPixel batLed(10, BAT_LED_PIN);
// #else

// #endif

struct Athlete;
struct Result;
struct Trigger;

void renderIntro();
void handleGUI();
void handleEvent(Event::Event_t event);
void handleGUILogic();
void updateGUI();
void renderGUI();
void handleKY40Events();
int xyToIndex(int x, int y);
void focusLive();
void focusSettings();
void renderLive();
void handleStart();
void handleUp();
Athlete* addAthleteFull(const char* name);
void addAthlete(const char* name);
void saveResult();
void focusAskSaveResult();
void deleteAthlete(uint16_t id);
void delelteAllAthletes();
Athlete* getAthleteById(uint16_t id);
void delelteAllResults();
void addResult(Result* res);
void removeResult();
void printError(esp_err_t error);
void beginESPNow();
void handleESPNow();
void receiveCallback(const uint8_t *mac_addr, const uint8_t *data, int data_len);
void sendCallback(const uint8_t *mac_addr, esp_now_send_status_t status);
void sepNowSend(uint8_t* data, size_t size);
void sendTrigger(Trigger* trigger, bool response = false);
void send(const char* str);
void send(char c);
void send(uint8_t* data, size_t size);
void changeSSID(const char* ssid);
void changePwd(const char* pwd);
void deleteAFKDevices();
void removeDevice(uint8_t index);
void registerDevice(uint8_t deviceId);
void uploadNow();
void setupWIFI(bool atBootup = false);
void makeMaster();
void factoryReset1();

struct ListedDevice {
  uint8_t deviceId;
  uint32_t lastRegister;
  uint32_t lastTrigger;
  bool isMaster;
};
ListedDevice listedDevices[10];
uint8_t listedDevicesSize = 0;

bool laser = false; // if laser is triggered

bool isMaster = false;
uint32_t lastMasterRequest = 0;
bool masterFound = false;

struct Athlete {
  const char* name;
  uint16_t id; // only gui intern
};

struct Trigger {
  uint8_t triggerer;
  uint32_t ms;
};

struct Result {
  uint16_t meters;
  uint16_t triggerSize;
  Athlete* athlete;
  Trigger** triggers;
  bool saved; // not in storage

  uint16_t getSize() {
    return sizeof(uint16_t) +         //meters
      sizeof(uint16_t) +              //triggerSize
      strlen(athlete->name) + 1 +     //athlete (+1 for terminating char)
      sizeof(Trigger) * triggerSize;  //triggers
  }
};

uint32_t msFromResult(Result* res);

Athlete* athletes[100];
uint8_t athleteSize = 0;
uint16_t athleteId = 0; // used only on gui side

Trigger* triggers[3000];
uint16_t triggerSize = 0;

Result* results[100];
uint8_t resultsSize = 0;
Result* activeResult = nullptr;
int activeResultIndex = -1;

bool wifiInited = false;
bool postSend = false;

enum Mode {
  SIMPLE, DISTANCE, RACE, GHOST, PROGRAM, VELOCITY
};

Button* startBtn = new Button("START!", handleStart);
Button* uploadBtn = new Button("UPLOAD NOW", uploadNow, false, 0x003399);
Button* delResultsBtn = new Button("DEL. RESULTS", delelteAllResults, true, 0xFF0000);
Button* saveResultBtn = new Button("SAVE!", saveResult, false, 0x22FF22);
Button* removeResultBtn = new Button("REMOVE", removeResult, true, 0xFF0000);
Button* cancelSaveLapBtn = new Button("CANCEL", focusLive, false, 0x992211);
Button* delAllAthletesbtn = new Button("DEL ALL", delelteAllAthletes, true, 0xFF0000);
Button* factoryResetBtn = new Button("FACT. RESET", factoryReset1, true, 0x0055AA);
Button* makeMasterBtn = new Button("MAKE MASTER", makeMaster, false, 0x449944);
Select<Mode>* modeSelect = new Select<Mode>("MODE");
MenuRenderer* athleteSettings = new MenuRenderer("ATHLETES");
MenuRenderer* settings = new MenuRenderer("SETTINGS");
MenuRenderer* lapSettings = new MenuRenderer("LAPS");
MenuRenderer* wifiSettings = new MenuRenderer("WIFI");
TextInput* ssidInput = new TextInput("SSID", changeSSID);
TextItem* ssidText = new TextItem("-", 0x337788);
TextInput* pwdInput = new TextInput("PWD", changePwd);
MenuRenderer* showPwd = new MenuRenderer("SHOW PWD");
TextItem* pwdText = new TextItem("", 0x337788);
MenuRenderer* simpleSettings = new MenuRenderer("SIMPLE");
MenuRenderer* distanceSettings = new MenuRenderer("DISTANCE");
MenuRenderer* raceSettings = new MenuRenderer("RACE");
MenuRenderer* ghostSettings = new MenuRenderer("GHOST");
MenuRenderer* programSettings = new MenuRenderer("PROGRAM");
MenuRenderer* velocitySettings = new MenuRenderer("VELOCITY");
MenuRenderer* startSettings = new MenuRenderer("START");
TextInput* athleteAdder = new TextInput(" ADD", addAthlete);
MenuRenderer* systemSettings = new MenuRenderer("SYSTEM");
MenuRenderer* advancedSettings = new MenuRenderer("ADVANCED");
CheckBox* useStart;
CheckBox* showLaps;
CheckBox* useWIFI;
CheckBox* useSound;
CheckBox* useLaserSound;
CheckBox* invertLaser;
InputItem* resultDistance;
InputItem* lapDistanceInput;
InputItem* distanceInput;
InputItem* laserPerDistance;
InputItem* raceLapsInput;
InputItem* raceTargetInput;
InputItem* raceDistanceInput;
InputItem* brightness;
InputItem* dispTimeDur;
InputItem* triggerDelay;
InputItem* deviceId;
InputItem* startMinDelay;
InputItem* startMaxDelay;

WS2812B display(DISPLAY_PIN, 32, 8, &xyToIndex);
MenuRenderer menu;
MenuRenderer saveResultMenu;
Select<uint16_t>* athleteSelect = new Select<uint16_t>("ATHL.");

double x = 10 - 32;
double y = 10;
double targetX = 0;
double targetY = 0;

int focusedGUI = 0; // 0 = settings, 1 = Live view, 2 = saveLap

int error = 0; // millis
uint32_t errorExpTime = 0;

int laps = 0;

bool skipIntro = false;

struct ModeRenderer {
  
  bool wantsFocus = false;

  virtual void init();

  virtual void render(int x, int y, WS2812B* display) {
    int textX = x;
    bool showLast = false;
    uint16_t displayLaps = 0;
    uint32_t ms = getTime(&showLast, &displayLaps);
    if(showLaps->checked) {
      char lapStr[3];
      sprintf(lapStr, "%i", displayLaps % 10);
      textX = display->print(lapStr, textX, y + 3, 0xFFFFFF, FONT_SIZE_SMALL + 1);
      display->line(x+3, y+0, x+3, x+7, 0x0000FF);
      for (size_t i = 0; i < (laps / 10) % 10; i++) {
        display->dot(x+i % 3, y+i/3, 0xFF0000 >> i* 2);
      }
    }
    renderTime(textX, y, ms, !showLast, display);
    renderResults(x, y, display);
  }

  void renderResults(int x, int y, WS2812B* display) {
    if(focusedGUI == 0) return;
    y += 8;
    for (int i = resultsSize - 1; i >= 0; i--) {
      if(y > 8 || y < -8) {
        y += 8;
        continue;
      }
      Result* res = results[i];
      display->line(x+3, y+0, x+3, y+7, 0x0000FF);
      // char indexStr[5];
      // sprintf(indexStr, "%i", i % 10);
      // display->print(indexStr, x, y + 3, 0xFFF000, FONT_SIZE_SMALL + 0);
      if(res->saved) {
        display->line(x+0,y+1,x+0,y+2,0x00FF00);
        display->line(x+0,y+2,x+2,y+0,0x00FF00);
      } else {
        display->line(x+0,y+0,x+2,y+2,0xFF0000);
        display->line(x+2,y+0,x+0,y+2,0xFF0000);
      }
      renderTime(x + 4, y, msFromResult(res), false, display);
      y += 8;
    }
  }

  virtual void handleEvent(Event::Event_t& event) {
    switch(event) {
      case Event::SCROLL_UP: {
        activeResultIndex = max(-1, activeResultIndex - 1);
        // Serial.println(activeResultIndex);
        break;
      }
      case Event::SCROLL_DOWN: {
        activeResultIndex = min((int) resultsSize - 1, activeResultIndex + 1);
        // Serial.println(activeResultIndex);
        break;
      }
      case Event::LONG_RELEASE_BUTTON: {
        if(activeResultIndex == -1) {
          wantsFocus = false;
        } else {
          activeResultIndex = -1;
        }
        break;
      }
      case Event::RELEASE_BUTTON: {
        if(activeResultIndex == -1) {
          laps = 0;
        } else {
          event = Event::NONE;
          focusAskSaveResult();
          return; // dont run update for targetY
        }
        break;
      }
      default: {}
    }
    targetY = 2 - activeResultIndex * 8;
  }

  virtual void trigger(Trigger* triggerer) = 0;

  virtual uint32_t getTime(bool* showLast, uint16_t* laps1) {
    *laps1 = laps;
    *showLast = false;
    if(triggerSize == 0) {
      return 0;
    }
    if(triggerSize == 1) {
      return millis() - triggers[triggerSize - 1]->ms;
    }
    *showLast = millis() - triggers[triggerSize - 1]->ms < dispTimeDur->value * 1000;
    if(*showLast) { // show last Time
      return triggers[triggerSize - 1]->ms - triggers[triggerSize - 2]->ms;
    } else { // show running Time
      return millis() - triggers[triggerSize - 1]->ms;
    }
  };
};

ModeRenderer* getModeRenderer();

struct ModeS : public ModeRenderer {

  void render(int x, int y, WS2812B* display) {
    ModeRenderer::render(x, y, display);
  }

  void handleEvent(Event::Event_t& event) {
    ModeRenderer::handleEvent(event);
  }

  void trigger(Trigger* t) {
    if(triggerSize <= 1) return;

    Result* r = new Result{(uint16_t) lapDistanceInput->value, 2, (Athlete*) nullptr, &triggers[triggerSize - 2], false};
    addResult(r);
    
    laps++;
  }

  void init() {}
  
  uint32_t getTime(bool* showLast, uint16_t* displayLaps) {
    return ModeRenderer::getTime(showLast, displayLaps);
  }
};

struct ModeD : public ModeRenderer {
  
  bool finished = false;
  bool started = false;

  void render(int x, int y, WS2812B* display) {
    ModeRenderer::render(x, y, display);
  }

  void handleEvent(Event::Event_t& event) {
    ModeRenderer::handleEvent(event);
    if(activeResultIndex == -1 && event == Event::RELEASE_BUTTON) {
        laps = laserPerDistance->value;
    }
  }

  void trigger(Trigger* t) {
    started = true;
    laps--;
    if(triggerSize <= 2) return;
    if(finished) {
      finished = false;
    }
    if(laps <= -1) {
      bgPlayer.playBeepOn();
      finished = true;
      Serial.println(millis() - triggers[triggerSize - 1 - (int) laserPerDistance->value]->ms);
      Result* r = new Result{(uint16_t) distanceInput->value, (int) laserPerDistance->value + 1, (Athlete*) nullptr, &triggers[triggerSize - 1 - (int) laserPerDistance->value], false};
      addResult(r);
      laps = laserPerDistance->value;
    }
  }

  void init() {
    laps = laserPerDistance->value;
  }

  uint32_t getTime(bool* showLast, uint16_t* displayLaps) {
    *displayLaps = laps;
    int maxLaps = laserPerDistance->value;
    int lapsDone = maxLaps - laps;
    if(lapsDone == 0) {
      *displayLaps = laps;
      if(finished) {
        *showLast = true;
        return triggers[triggerSize - 1]->ms - triggers[triggerSize - maxLaps - 1]->ms;
      }
      *showLast = false;
      return 0;
    }
    uint32_t firstTriggerMs = triggers[triggerSize - (maxLaps - laps)]->ms;
    if((millis() - triggers[triggerSize - 1]->ms) < dispTimeDur->value * 1000 && lapsDone > 1) {
      *showLast = true;
      return triggers[triggerSize - 1]->ms - triggers[triggerSize - 2]->ms;
    }
    *showLast = false;
    return millis() - firstTriggerMs;
  }
};

struct ModeR : public ModeRenderer {
  double timeTaken = 0;
  bool initiated = false;
  uint64_t startUs = 0;
  double adjust = 0;

  void render(int x, int y, WS2812B* display) {
    int xStart = x;
    // LAPS
    char lapStr[3];
    sprintf(lapStr, "%i", laps % 10);
    x = display->print(lapStr, x, y + 3, 0xFFFFFF, FONT_SIZE_SMALL);
    display->line(x, y+0, x, 7, 0x0000FF);
    for (int i = (max(0,laps) / 10) - 1 % 10; i >= 0 ; i--) {
      display->dot(x-3+i % 3, y+i/3, 0xFF0000 >> i* 2);
    }
    x+=2;

    // ADJUST
    uint32_t color = adjust > 0 ? 0x00FF00 : 0xFF0000;
    if(abs(adjust) < 0.3) {
      color = 0xFFFFFF;
    }

    double timeRight = 0;
    if(triggerSize > 0) {
      if(millis() - triggers[triggerSize - 1]->ms < dispTimeDur->value * 1000 && triggerSize > 1 && laps != raceLapsInput->value) {
        // if(triggers[triggerSize - 2] && triggers[triggerSize - 1]) {
        //   timeRight = (triggers[triggerSize - 2]->ms - triggers[triggerSize - 1]->ms) / 1000.0;
        // }
        timeRight = (millis() - triggers[triggerSize - ((int) raceLapsInput->value - laps)]->ms) / 1000.0;
      } else if(laps < raceLapsInput->value) {
        timeRight = (millis() - triggers[triggerSize - ((int) raceLapsInput->value - laps)]->ms) / 1000.0;
      } else {
        timeRight = (triggers[triggerSize - 1]->ms - triggers[triggerSize - 1 - (int) raceLapsInput->value]->ms) / 1000.0;
      }
    }
    char adjStr[10];
    char timeStr[10];
    sprintf(adjStr, "%.1f", laps < raceLapsInput->value ? adjust : timeTaken);
    sprintf(timeStr, "%f", timeRight);
    x = display->print(adjStr, x , y, color, 1);
    x = display->print(timeStr, x + 1 , y + 3, 0x000099, FONT_SIZE_SMALL + 1);
    renderResults(xStart, y, display);
  }

  void handleEvent(Event::Event_t& event) {
    ModeRenderer::handleEvent(event);
    if(event == Event::RELEASE_BUTTON && targetY == 10) {
      laps = raceLapsInput->value;
      adjust = 0;
      timeTaken = 0;
    }
  }

  void trigger(Trigger* t) {
    if(laps == raceLapsInput->value) {
      startUs = t->ms;
    }
    if(triggerSize < 2 || laps == raceLapsInput->value) {
      laps--;
      return;
    }
    timeTaken = (t->ms - startUs) / 1000.0;
    if(laps < raceLapsInput->value && laps > 0) {
      double lastLap = (t->ms - triggers[triggerSize - 2]->ms) / 1000.0;
      double timeGoal = raceTargetInput->value;
      double lastLapSol = (timeGoal - (timeTaken - lastLap)) / (laps + 1);
      adjust = lastLapSol - lastLap;
    }
    laps--;
    if(laps < 0) {
      Result* r = new Result{(uint16_t) raceDistanceInput->value, (uint16_t) (raceLapsInput->value + 1), (Athlete*) nullptr, &triggers[(int) triggerSize - (int) raceLapsInput->value - 1], false};
      addResult(r);
      laps = raceLapsInput->value;
    }
  }

  void init() {
    if(!initiated) {
      laps = raceLapsInput->value;
      initiated = true;
      adjust = 0;
      timeTaken = 0;
    }
  }

  uint32_t getTime(bool* showLast, uint16_t* displayLaps) {
    return ModeRenderer::getTime(showLast, displayLaps);
  }
};

struct ModeG : public ModeRenderer {
  void render(int x, int y, WS2812B* display) {
    ModeRenderer::render(x, y, display);
  }
  void handleEvent(Event::Event_t& event) {
    ModeRenderer::handleEvent(event);
  }

  void trigger(Trigger* t) {
  }

  void init() {

  }

  uint32_t getTime(bool* showLast, uint16_t* displayLaps) {
    return ModeRenderer::getTime(showLast, displayLaps);
  }
};

struct ModeP : public ModeRenderer {
  void render(int x, int y, WS2812B* display) {
    ModeRenderer::render(x, y, display);
  }
  void handleEvent(Event::Event_t& event) {
    ModeRenderer::handleEvent(event);
  }

  void trigger(Trigger* t) {
  }

  void init() {
    
  }

  uint32_t getTime(bool* showLast, uint16_t* displayLaps) {
    return ModeRenderer::getTime(showLast, displayLaps);
  }
};

struct ModeV : public ModeRenderer {
  void render(int x, int y, WS2812B* display) {
    ModeRenderer::render(x, y, display);
  }
  void handleEvent(Event::Event_t& event) {
    ModeRenderer::handleEvent(event);
  }

  void trigger(Trigger* t) {
  }

  void init() {
    
  }

  uint32_t getTime(bool* showLast, uint16_t* displayLaps) {
    return ModeRenderer::getTime(showLast, displayLaps);
  }
};

ModeS modeS;
ModeD modeD;
ModeR modeR;
ModeG modeG;
ModeP modeP;
ModeV modeV;

// uint16_t meters;
// uint16_t triggerSize;
// Athlete* athlete;
// Trigger** triggers;

uint8_t triggerToBytes(Trigger* trigger, uint8_t* buff) {
  buff[0] = trigger->triggerer;
  *((uint32_t*) &buff[1]) = trigger->ms;
  return 5; // size
}

void saveResultsToStorage() {
  preferences.remove("results");
  if(resultsSize == 0) return;

  uint16_t size = 0;
  uint16_t saveResultsCount = 0;
  for (size_t i = 0; i < resultsSize; i++) {
    if(results[i]->saved) {
      size += results[i]->getSize();
      saveResultsCount++;
    }
  }

  uint8_t* buff = new uint8_t[size];
  uint16_t buffSize = 0;

  for (size_t i = 0; i < resultsSize; i++) {
    Result* res = results[i];
    if(!res->saved) continue;

    Serial.printf("saving result: triggersize: %i\n", res->triggerSize);

    // Meter
    *((uint16_t*) &buff[buffSize])= res->meters;
    buffSize+=sizeof(uint16_t);

    // triggerSize
    *((uint16_t*) &buff[buffSize])= res->triggerSize;
    buffSize+=sizeof(uint16_t);

    // athlete
    uint16_t nameLen = strlen(res->athlete->name);
    for (size_t strpos = 0; strpos <= nameLen; strpos++) {
      *((char*) &buff[buffSize])= res->athlete->name[strpos];
      buffSize+=sizeof(char);
    }


    Serial.printf("savinf this exact ms:%i\n", res->triggers[1]->ms - res->triggers[0]->ms);
    for (size_t triggerIndex = 0; triggerIndex < res->triggerSize; triggerIndex++) {
      *((Trigger*) &buff[buffSize]) = *res->triggers[triggerIndex];
      Serial.println(((Trigger*) &buff[buffSize])->ms);
      // Serial.println(buffSize);
      buffSize += sizeof(Trigger);
      // buffSize += triggerToBytes(res->triggers[triggerIndex], &buff[buffSize]);
    }
  }
  preferences.putBytes("results", buff, size);
  Serial.printf("Saved %i results to storage using %i bytes. (%i)\n", saveResultsCount, size, buffSize);
  // delete[] buff;
}

Athlete* getAthleteByName(const char* name) {
  for (size_t i = 0; i < athleteSize; i++) {
    uint16_t strpos = 0;
    while(true) {
      if(name[strpos] != athletes[i]->name[strpos]) {
        break;
      }
      if(name[strpos] == 0 && athletes[i]->name[strpos] == 0) {
        return athletes[i];
      }
      strpos++;
    }
  }
  return nullptr;
}


void loadResultsFromStorage() {
  if(!storageInitiated) return;
  Serial.println("loading results...");
  resultsSize = 0;
  size_t size = preferences.getBytesLength("results");
  uint8_t* buff = new uint8_t[size];
  size_t usedSize = 0;
  preferences.getBytes("results", buff, size);
  while (usedSize < size) {
    Result* res = new Result;
    res->saved = true;
    
    res->meters = *((uint16_t*) &buff[usedSize]);
    usedSize+=sizeof(uint16_t);

    res->triggerSize = *((uint16_t*) &buff[usedSize]);
    usedSize+=sizeof(uint16_t);
    Serial.printf("loading triggersize: %i", res->triggerSize);

    uint16_t nameLen = strlen((char*) &buff[usedSize]);
    char* name = new char[nameLen + 1];
    for (size_t strpos = 0; strpos <= nameLen; strpos++) {
      name[strpos] = buff[usedSize];
      usedSize+=sizeof(char);
    }
    Athlete* athlete = new Athlete{name, athleteId};
    athleteId++;
    // if(!athlete) {
    //   athlete = addAthleteFull(name);
    // }
    res->athlete = athlete;
    res->triggers = new Trigger*[res->triggerSize];
    for (size_t triggerIndex = 0; triggerIndex < res->triggerSize; triggerIndex++) {
      // Serial.println(res->triggerSize);
      // delay(1000);
      Trigger* trigger = new Trigger;
      *trigger = *((Trigger*) &buff[usedSize]);
      usedSize+=sizeof(Trigger);
      res->triggers[triggerIndex] = trigger;
    }
    addResult(res);
  }

  Serial.printf("Loaded %i results using %i bytes\n", resultsSize, size);
  // delete[] buff;
}

/**
 * @brief Saves the names of all athletes as byte arrays seperated by 0
 * 
 */
void saveAthletesToStorage() {
  preferences.remove("athletes");
  if(athleteSize == 0) {
    return;
  }
  uint16_t size = 0;
  for (size_t i = 0; i < athleteSize; i++) {
    size += strlen(athletes[i]->name) + 1;
  }
  uint8_t* buff = new uint8_t[size];
  uint16_t buffSize = 0;

  for (size_t index = 0; index < athleteSize; index++) {
    uint8_t len = strlen(athletes[index]->name);
    for (size_t strpos = 0; strpos <= len; strpos++) {
      buff[buffSize] = athletes[index]->name[strpos];
      buffSize++;
    }
  }
  preferences.putBytes("athletes", buff, size);
  delete[] buff;
  Serial.printf("Saved %i athletes to storage using %i bytes. (%i)\n", athleteSize, size, buffSize);
}

void loadAthletesFromStorage() {
  if(!storageInitiated) return;
  athleteSize = 0;
  Serial.println("loading athletes...");
  uint32_t size = preferences.getBytesLength("athletes");
  uint8_t* bytes = new uint8_t[size];
  preferences.getBytes("athletes", bytes, size);
  uint16_t index = 0;
  for (size_t i = 0; i < size; i++) {
    uint8_t len = strlen((char*) bytes);
    athletes[athleteSize] = new Athlete{(char*) bytes, athleteId};
    athleteSelect->addOption((char*) bytes, athleteId);
    athleteSettings->addItem(new DeleteButton((char*) bytes, true, athleteSettings, athleteId, deleteAthlete), 1);
    athleteId++;
    athleteSize++;
    bytes += len + 1;
    i += len + 1;
    index++;
  }
  Serial.printf("Loaded %i athletes using %i bytes\n", athleteSize, size);
}

void trigger(Trigger* t) {
  // bgPlayer.playBeepLong();
  uint32_t lastTriggerMs = triggerSize > 0 ? triggers[triggerSize - 1]->ms : 0;
  // if(millis() - lastTriggerMs < triggerDelay->value * 1000) {
  //   delete t;
  //   return;
  // }
  Serial.println("Trigger");
  // Serial.println(triggerDelay->value);
  triggers[triggerSize] = t;
  triggerSize++;
  if(!isMaster) { // only slaves send triggers
    sendTrigger(t);
  }
  getModeRenderer()->trigger(triggers[triggerSize - 1]);
  if(triggerSize >= 3000) {
    for (size_t i = 0; i < triggerSize; i++) {
      delete triggers[i];
    }
    
    triggerSize = 0;
  }
}

uint32_t msFromResult(Result* res) {
  if(res->triggerSize <= 1) {
    return 0;
  }
  return res->triggers[res->triggerSize - 1]->ms - res->triggers[0]->ms;
}

int xyToIndex(int x, int y) {
  bool odd = x % 2 & 0x01;
  return x * 8 + !odd * y + odd * (8 - y - 1);
}

void removeResult() {
  if(activeResultIndex == -1 || (resultsSize - 1 - activeResultIndex) >= resultsSize) return;
  uint16_t index = resultsSize - 1 - activeResultIndex;
  delete results[index];
  resultsSize--;
  for (size_t i = 0; i < resultsSize; i++) {
    results[i] = results[i+1];
  }
  focusLive();
}

void saveResult() {
  focusLive();
  Serial.println("Saving result...");
  if(!activeResult) {
    error = 2;
    errorExpTime = millis() + 500000000;
    return;
  }
  if(!athleteSelect->isSelected()) {
    error = 1;
    errorExpTime = millis() + 5000;
    return;
  }

  uint16_t idAthlete = athleteSelect->value;
  activeResult->athlete = getAthleteById(idAthlete);
  if(!activeResult->athlete) {
    error = 1;
    errorExpTime = millis() + 5000;
    return;
  }

  activeResult->meters = (uint16_t) resultDistance->value;
  activeResult->saved = true;

  Serial.print("saving ms:");
  Serial.println(activeResult->triggers[1]->ms - activeResult->triggers[0]->ms);

  saveResultsToStorage();
  error = -1; // succsess
  errorExpTime = millis() + 1000;
}

struct Line {
  uint8_t speed;
  uint8_t offset;
  uint8_t length;
  uint8_t y;
};

Line* lines[INTRO_LINES];

void initIntro() {
  for (size_t i = 0; i < INTRO_LINES; i++) {
    Line* l = new Line;
    l->length = random(5, 40);
    l->speed = random(200, 255);
    l->offset = random(0, 60);
    l->y = random(0, 8);
    lines[i] = l;
  }
}

void renderIntro() {
  double brightness = display.brightness;
  display.setBrightness(min(0.1, brightness));
  double progress = 1.0 - ((double) INTRO_TIME - (millis() - 500)) / (double) INTRO_TIME;
  display.setBlur(-progress);
  if(millis() > INTRO_TIME / 2) {

    // display.print("SPEED", (int) (cos(0.0 + progress * PI * 1.5) * 25.0) + 29, 1, 0xFF44CC, FONT_SIZE_SMALL);
    display.print("R.-R.", (int) (cos(-0.2 + progress * PI * 2.0) * 27.0) + 35, 1, 0x66FF44, FONT_SIZE_SMALL);
  }
  progress *= 1.5; // faster lines
  for (size_t i = 0; i < INTRO_LINES; i++) {
    Line* l = lines[i];
    display.line((double) -l->offset - l->length + progress * l->speed, (double) l->y, -l->offset + progress * l->speed, (double) l->y, 0x22FF44);
  }
  display.setBrightness(brightness);
}

void handleStart() {
  focusLive();
}

void handleUp() {
  menu.scrollTo(0);
}

Athlete* getAthleteById(uint16_t id) {
  for (size_t i = 0; i < athleteSize; i++) {
    if(athletes[i]->id == id) {
      return athletes[i];
    }
  }
  return nullptr;
}

void delelteAllResults() {
  resultsSize = 0;
  saveResultsToStorage();
}

void delelteAllAthletes() {
  for (size_t i = 0; i < athleteSize; i++) {
    athleteSelect->removeByValue(athletes[i]->id);
    athleteSettings->removeItem(1);
  }
  athleteSize = 0;
  saveAthletesToStorage();
}

void deleteAthlete(uint16_t id) {
  athleteSelect->removeByValue(id);
  for (size_t i = 0; i < athleteSize; i++) {
    if(athletes[i]->id == id) {
      // delete athletes[i];
      athleteSize--;
    }
    if(i < athleteSize) {
      athletes[i] = athletes[i + 1];
    }
  }
  saveAthletesToStorage();
}

void addResult(Result* res) {
  results[resultsSize] = res;
  resultsSize++;
  if(resultsSize >= 100) {
    for (size_t i = 0; i < resultsSize; i++) {
      delete results[i];
    }
    resultsSize = 0;
  }
  if(activeResultIndex >= 0) {
    activeResultIndex++;
  }
}

void addAthlete(const char* name) {
  addAthleteFull(name);
}

Athlete* addAthleteFull(const char* name) {
  athletes[athleteSize] = new Athlete{name, athleteId};
  athleteSelect->addOption(name, athleteId);
  athleteSettings->addItem(new DeleteButton(name, true, athleteSettings, athleteId, deleteAthlete), 1);
  athleteId++;
  athleteSize++;
  saveAthletesToStorage();
  return athletes[athleteSize - 1];
}

void factoryReset1() {
  resetStorage();
  delelteAllAthletes();
  delelteAllResults();
  error = 4;
  errorExpTime = millis() + 10000000;
}

void uploadNow() {
  uint16_t savedResults = 0;
  for (size_t i = 0; i < resultsSize; i++) {
    if(results[i]->saved) {
      savedResults++;
    }
  }
  if(savedResults == 0) {
    error = 5;
    errorExpTime = millis() + 10000;
    return;
  }
  setupWIFI();
  useWIFI->checked = true;
  postSend = false;
  error = 6;
  errorExpTime = millis() + 1000000;
}

void handleWIFI() {
  if(!useWIFI->checked) return;

  if(millis() > 15000 && listedDevicesSize == 0 && !wifiInited && !postSend) {
    setupWIFI();
  }

  if(!wifiInited || postSend) return;
  if(resultsSize == 0) {
    postSend = true;
    return;
  }
  if(WiFi.status() == WL_CONNECTED && !postSend) {
    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    for (size_t i = 0; i < resultsSize; i++) {
      Result* res = results[i];
      if(!res->saved || !res->athlete) continue;

      http.begin("https://www.roller-results.com/api/index.php?uploadResults=1&user=1136&lname=1.0");
      http.addHeader("Content-Type", "text/plain");

      char json[1500] = "";
      char lapStr[1400] = "";
      char del = ' ';
      for (size_t i = 0; i < res->triggerSize; i++) {
        Trigger* trigger = res->triggers[i];
        sprintf(lapStr, "%s%c{\"t\":%i,\"ms\":%i}", lapStr, del, trigger->triggerer, trigger->ms);
        del = ',';
      }
      sprintf(json, "{\"d\":%i,\"a\":\"%s\",\"l\":[%s]}", res->meters, res->athlete->name, lapStr);
      int httpResponseCode = http.POST((uint8_t*) json, strlen(json)); //Send the actual POST request
      http.end();
      Serial.println(httpResponseCode);
      if(httpResponseCode != 200) {
        i--;
      }
      if(i == resultsSize - 1) {
        error = 100;
        errorExpTime = millis() + 10000;
        resultsSize = 0;
        saveResultsToStorage();
        wifiInited = false;
        postSend = true;
      }
    }
  } else {
    if(millis() % 1000 == 0) {
      Serial.println("Trying to connect to WIFI");
    }
  }
}

void changeSSID(const char* ssid) {
  preferences.remove("ssid");
  preferences.putBytes("ssid", (uint8_t*) ssid, strlen(ssid) + 1);
  ssidText->name = ssid;
}

void changePwd(const char* pwd) {
  preferences.remove("pwd");
  preferences.putBytes("pwd", (uint8_t*) pwd, strlen(pwd) + 1);
  pwdText->name = pwd;
}

void setupWIFI(bool atBootup) {
  if(atBootup) {
    size_t ssidLen = preferences.getBytesLength("ssid");
    size_t pwdLen = preferences.getBytesLength("pwd");
    char* ssid = new char[ssidLen];
    char* pwd  = new char[pwdLen];
    preferences.getBytes("ssid", ssid, ssidLen);
    preferences.getBytes("pwd", pwd, pwdLen);
    ssidText->name = ssid;
    pwdText->name = pwd;
  }
  WiFi.begin(ssidText->name, pwdText->name);
  wifiInited = true;
}

void checkResetButton() {
  bool reset = false;
  bgPlayer.begin();
  while(!digitalRead(PIN_BUTTON) && !reset) {
    bgPlayer.handle();
    if(millis() % ((7000 - millis()) / 5) == 0) {
      bgPlayer.playBeepOn();
    }
    if(millis() > 5000) {
      bgPlayer.playBeepLong();
      delay(1000);
      if(!digitalRead(PIN_BUTTON)) {
        Serial.println("reset");
        factoryReset1();
        reset = true;
      } else {
        bgPlayer.playBeepOff();
      }
    }
  }
}

void initDisplayGUI() {
  display.begin();
  athleteSettings->addItem(athleteAdder);
  athleteSettings->addItem(delAllAthletesbtn);

  // Menu
  showLaps             = new CheckBox("LAPS", true);
  useWIFI              = new CheckBox("WIFI ON", false);
  useSound             = new CheckBox("SOUND", true);
  useLaserSound        = new CheckBox("L SOUND", true);
  invertLaser          = new CheckBox("INVERT", false);
  useStart             = new CheckBox("USE", false);
  resultDistance       = new InputItem("DIST.", 10, 1000000, 10, 200);
  lapDistanceInput     = new InputItem("DIST.", 10, 1000000, 10, 200);
  distanceInput        = new InputItem("DIST.", 10, 1000000, 10, 200);
  laserPerDistance     = new InputItem("LAPS", 1, 1000000, 1, 5);
  raceLapsInput        = new InputItem("LAPS", 2, 1000000, 1, 3);
  raceTargetInput      = new InputItem("TARGET", 1, 1000000, 1, 300);
  raceDistanceInput    = new InputItem("DIST.", 10, 1000000, 100, 3000);
  brightness           = new InputItem("LED", 1, 75, 1, 5);
  dispTimeDur          = new InputItem("DISPLAY", -1, 100, 0.1, 3);
  triggerDelay         = new InputItem("DELAY", 0, 1000, 0.1, 3);
  deviceId             = new InputItem("D. ID", 0, 255, 1, 0);
  startMinDelay        = new InputItem("MIN", 0.5, 100, 0.1, 6);
  startMaxDelay        = new InputItem("MAX", 0.5, 100, 0.1, 9);

  modeSelect->addOption("SIMPLE", SIMPLE);
  modeSelect->addOption("DISATANCE", DISTANCE);
  modeSelect->addOption("RACE", RACE);
  // modeSelect->addOption("GHOST", SIMPLE);
  // modeSelect->addOption("Program", PROGRAM);
  // modeSelect->addOption("Velocity", VELOCITY);

  modeSelect->selectOption(SIMPLE);

  simpleSettings->addItem(showLaps);
  simpleSettings->addItem(lapDistanceInput);

  distanceSettings->addItem(distanceInput);
  distanceSettings->addItem(laserPerDistance);

  raceSettings->addItem(raceLapsInput);
  raceSettings->addItem(raceTargetInput);
  raceSettings->addItem(raceDistanceInput);

  wifiSettings->addItem(useWIFI);
  wifiSettings->addItem(uploadBtn);
  wifiSettings->addItem(ssidInput);
  wifiSettings->addItem(ssidText);
  wifiSettings->addItem(pwdInput);
  wifiSettings->addItem(showPwd);
  showPwd->addItem(pwdText);

  advancedSettings->addItem(invertLaser);
  advancedSettings->addItem(delResultsBtn);
  advancedSettings->addItem(factoryResetBtn);

  systemSettings->addItem(wifiSettings);
  systemSettings->addItem(deviceId);
  systemSettings->addItem(useSound);
  systemSettings->addItem(useLaserSound);
  systemSettings->addItem(brightness);
  systemSettings->addItem(dispTimeDur);
  systemSettings->addItem(triggerDelay);
  systemSettings->addItem(advancedSettings);

  startSettings->addItem(useStart);
  startSettings->addItem(startMinDelay);
  startSettings->addItem(startMaxDelay);

  settings->addItem(startSettings);
  settings->addItem(simpleSettings);
  settings->addItem(distanceSettings);
  settings->addItem(raceSettings);
  // settings->addItem(ghostSettings);
  // settings->addItem(programSettings);
  // settings->addItem(velocitySettings);

  menu.addItem(startBtn);
  menu.addItem(modeSelect);
  menu.addItem(settings);
  menu.addItem(athleteSettings);
  menu.addItem(makeMasterBtn);
  menu.addItem(systemSettings);

  saveResultMenu.addItem(athleteSelect);
  saveResultMenu.addItem(lapSettings);
  saveResultMenu.addItem(resultDistance);
  saveResultMenu.addItem(athleteSettings);
  saveResultMenu.addItem(cancelSaveLapBtn);
  focusLive();
  initIntro();
}

void setup() {
  // Setup for both Laser station and display
  Serial.begin(115200);
  Serial.println("Serial up");
  pinMode(LED_BUILTIN, OUTPUT);
  beginESPNow();
  setupWIFI();

  #ifdef IS_LASER // Setup only for laser station
    m18Laser.begin();
    batLed.begin();
  #else // Setup only for display
    beginKY40(PIN_A, PIN_B, PIN_BUTTON); // has to be done before call to checkResetButton()
    checkResetButton();
    beginStorage();
    loadAthletesFromStorage();
    loadResultsFromStorage();
    // delelteAllResults();
    initDisplayGUI();
    // bgPlayer.playMarioBros();
  #endif

  Serial.println("Booted :>");
}

/**
 * GUI
 */
uint32_t lastGUIRender = 0;

void handleGUI() {
  handleKY40Events();
  handleGUILogic();
  renderGUI();
}

bool animDone = true;

void focusAskSaveResult() {
  if(activeResultIndex == -1 || (resultsSize - 1 - activeResultIndex) >= resultsSize) return;
  activeResult = results[resultsSize - 1 - activeResultIndex];
  focusedGUI = 2;
  targetY = 10;
  targetX = 40;
  animDone = false;
  resultDistance->value = (double) activeResult->meters;
  // save unsave btn
  if(activeResult->saved) {
    saveResultMenu.removeItem(saveResultBtn);
    saveResultMenu.removeItem(removeResultBtn);
    saveResultMenu.addItem(removeResultBtn, 0);
  } else {
    saveResultMenu.removeItem(saveResultBtn);
    saveResultMenu.removeItem(removeResultBtn);
    saveResultMenu.addItem(saveResultBtn, 0);
  }
  // laps
  lapSettings->empty();
  for (size_t i = 1; i < activeResult->triggerSize; i++) {
    LapItem* lap = new LapItem(i, activeResult->triggers[i]->ms - activeResult->triggers[i - 1]->ms);
    lapSettings->addItem(lap);
  }
  
  saveResultMenu.scrollTo(0);
}

Mode lastMode = SIMPLE;

void focusLive() {
  focusedGUI = 1;
  targetY = 10;
  targetX = 10;
  animDone = false;
  getModeRenderer()->wantsFocus = true;
  if(lastMode != modeSelect->value) {
    lastMode = modeSelect->value;
    getModeRenderer()->init();
  }
}

void focusSettings() {
  focusedGUI = 0;
  targetY = 0;
  targetX = 0;
  animDone = false;
}

void renderLive(int x, int y) {
  getModeRenderer()->render(x, y, &display);
}

uint32_t startAt = 0xFFFFFFFF;

void handleEvent(Event::Event_t event) {
  if(error && event == Event::RELEASE_BUTTON) {
    error = 0;
    return;
  }
  if(!skipIntro && event != Event::PRESS_BUTTON) {
    skipIntro = true;
    return;
  }
  if(focusedGUI == 0) {
    menu.handleEvent(event);
    return;
  }
  if(focusedGUI == 1) {
    getModeRenderer()->handleEvent(event); // can change event to Event::NONE
    if(event == Event::RELEASE_BUTTON) {
      if(useStart->checked) {
        startAt = millis() + random(startMinDelay->value, startMaxDelay->value) * 1000;
        bgPlayer.playBeepOn();
      }
    }
    if(!getModeRenderer()->wantsFocus) {
      focusSettings();
    }
    return;
  }
  if(focusedGUI == 2) {
    saveResultMenu.handleEvent(event);
    return;
  }
}

void handleGUILogic() {
  display.setBrightness(brightness->value / 100.0);
  bgPlayer.setDisabled(!useSound->checked);
}

void drawOverlay() {
  if(laser) {
    display.dot(31, 0, 0x009900);
  }
  if(wifiInited) {

    display.dot(30, 0, 0x001155 * (millis() % 1000 < 500));
  } else {

    uint32_t meColor = isMaster ? 0xFF0000 : 0x999999;
    if(triggerSize > 0 && millis() - triggers[triggerSize - 1]->ms < 500) {
      meColor = 0x0000FF;
    }
    display.dot(29, 0, meColor);
    for (size_t i = 0; i < listedDevicesSize; i++) {
      if(millis() - listedDevices[i].lastTrigger < 500) {
        display.dot(28-i, 0, 0x0000FF);
      } else {
        // display.dot(28-i, 0, listedDevices[i].isMaster ? 0x990000 : 0x555555);
        display.dot(28-i, 0, 0x555555);
      }
    }
  }
}

void updateGUI() {
  double lpf = 0.2;
  x = x * (1.0 - lpf) + targetX * lpf;
  y = y * (1.0 - lpf) + targetY * lpf;
  if(abs(x - targetX) + abs(y - targetY) < 1) {
    x = targetX;
    y = targetY;
    animDone = true;
  }
}

ModeRenderer* getModeRenderer() {
  switch(modeSelect->value) {
    case SIMPLE:    return &modeS;
    case DISTANCE:  return &modeD;
    case RACE:      return &modeR;
    case GHOST:     return &modeG;
    case PROGRAM:   return &modeP;
    case VELOCITY:  return &modeV;
    default:        return &modeS;
  }
}

bool loadedFirst = false;

void renderError() {
  if(millis() > errorExpTime) {
    error = false;
    return;
  }
  switch(error) {
      case -1: {
        display.printSlideOverflow("SAVED!", 1, 1, 31, 0x33FF33, FONT_SIZE_SMALL + 1);
        break;
      }
      case 1: {
        display.printSlideOverflow("NO ATHL.!", 1, 1, 31, 0xff1144, FONT_SIZE_SMALL + 1);
        break;
      }
      case 2: {
        display.printSlideOverflow("RES NULL!", 1, 1, 31, 0x33FF33, FONT_SIZE_SMALL + 1);
        break;
      }
      case 4: {
        display.printSlideOverflow("FACT.RESET DONE", 0, 1, 31, 0xff1144, FONT_SIZE_SMALL + 1);
        break;
      }
      case 5: {
        display.printSlideOverflow("NO RESULTS", 0, 1, 31, 0xff1144, FONT_SIZE_SMALL + 1);
        break;
      }
      case 6: {
        display.printSlideOverflow(" UPLOADING...", 0, 1, 31, 0x1122CC, FONT_SIZE_SMALL + 1);
        break;
      }
      case 100: {
        display.printSlideOverflow("UPLOADED :)", 1, 1, 31, 0x0044FF, FONT_SIZE_SMALL + 1);
        break;
      }
    }
}

void renderGUI() {
  if(millis() - lastGUIRender < (1000 / GUI_FPS)) return;
  display.clear();
  if(millis() > INTRO_TIME * 3/4 || skipIntro) {
    updateGUI();
    if(!loadedFirst) {
      loadedFirst = true;
    }
    if(error) {
      renderError();
    } else {
      if(focusedGUI == 0 || !animDone) {
        menu.render(x, y, &display);
      }
      if(focusedGUI == 1 || !animDone) {
        renderLive(x - 10, y - 10);
      }
      if(focusedGUI == 2 || !animDone) {
        saveResultMenu.render(x - 40, y - 10, &display);
      }
    }
    if(millis() > INTRO_TIME) {
      drawOverlay();
    }
  }
  if(((long) millis() - 500) < INTRO_TIME && !skipIntro) {
    renderIntro();
  }
  if(((long) millis() - 500) > INTRO_TIME) {
    skipIntro = true;
  }
  display.show();
  lastGUIRender = millis();
}

/**
 * ky40
 */
bool holdingActive = false;
int kyLastPosition = 0;
bool kyLastPressed = false;
void handleKY40Events() {
if(kyPressed && (millis() - kyPressTime) > 500) {
    handleEvent(Event::LONG_RELEASE_BUTTON);
    bgPlayer.playBeepLong();
    kyPressTime = millis();
    holdingActive = true;
  }
  if(!kyPressed && kyLastPressed && holdingActive) {
    holdingActive = false;
    kyLastPressed = false;
  }
  if(kyPosition > kyLastPosition) {
    bgPlayer.playBeepShortHigh();
    handleEvent(Event::SCROLL_UP);
    kyLastPosition = kyPosition;
  }
  if(kyPosition < kyLastPosition) {
    bgPlayer.playBeepShortLow();
    handleEvent(Event::SCROLL_DOWN);
    kyLastPosition = kyPosition;
  }
  if(!kyLastPressed && kyPressed) {
    handleEvent(Event::PRESS_BUTTON);
    kyLastPressed = kyPressed;
  }
  if(kyLastPressed && !kyPressed) {
    handleEvent(Event::RELEASE_BUTTON);
    kyLastPressed = kyPressed;
  }
}

void handleBat() {
  double vBat = (analogReadMilliVolts(BAT_PIN) / 1000.0) * 2.0;
  // Serial.println(vBat);
  double batEmpty = 3.0;
  double batFull = 4.2;
  batLed.clear();
  // int i = 0;
  // for (; vBat > batEmpty; vBat -= (batFull - batEmpty) / (double) BAT_LED_COUNT) {
    // batLed.setPixelColor(0, 0x555555);
    batLed.setPixelColor(1, 0xFFFFFF);
    // batLed.setPixelColor(2, 0x555555);
  //   // Serial.println(i);
  //   i++;
  // }
  // delay(100);
  // if(i < BAT_LED_COUNT - 1) {
  //   batLed.setPixelColor(i, 0x555555 * (millis() % 2000 < 1000));
  // }
  batLed.show();
}

uint32_t lastTriggerSend = 0;
bool lastLaser = false;
void loop() {
  // Loop for all
  // handleBat();
  // handleWIFI();
  // handleESPNow();

  #ifdef IS_LASER
  laser = m18Laser.isReflected();
  digitalWrite(LED_BUILTIN, laser);
  return;
  Serial.println(analogRead(LASER_PIN));
  if(!laser) {
    bgPlayer.handle();
  }
  if(laser && !lastLaser) {
    if(useLaserSound->checked) {
      ledcWriteTone(0, 2489);
    }
    trigger(new Trigger{(uint8_t) deviceId->value, millis()});
  }
  if(millis() > startAt) {
    trigger(new Trigger{(uint8_t) deviceId->value, millis()});
    bgPlayer.playBeepLong();
    startAt = 0xFFFFFFFF;
  }
  if(!laser && lastLaser) {
    ledcWriteTone(0, 0);
  }
  lastLaser = laser;
  #else // loop for display
  handleGUI();
  #endif
}

/**
 * ESP NOW
 * 
 * PROTOCOL 
 * ? = who is there?
 * M<uint8> = Master here + device id
 * S<uint8> = Slave here + device id
 * T<uint8><uint32><uint8> = Trigger, idSender, time, idTrigger(used for response))
 * R<uint8> = Received, idTrigger
 *  
 * Device starts:
 * repeat for 5s
 *    Sends out who is there?
 *    waits 500ms Second
 *    if (response == Master here) isMaster = false
 * 
 * Master
 *    Receives Trigger and respondes with Received
 *    SEND ? every 2 Seconds to list devices
 * 
 * Slave
 * 
 * BOTH
 *    Sends trigger and resends if no Received came
 *    Respond to who is there 
 *    List Devices that respond to ?
 *    Remove devices that didnt repond for 5 seconds
 */
uint32_t lastDeviceSearch = 0;

bool lastWifiInited = false;

void handleESPNow() {
  if(lastWifiInited && !wifiInited) {
    beginESPNow();
  }
  lastWifiInited = wifiInited;
  deleteAFKDevices();
  if(wifiInited) return;
  if(millis() > 5000 && !masterFound) {
    isMaster = true;
    masterFound = true; // me :)
    Serial.println("I'm Master =)");
  }
  if(isMaster || !masterFound) {
    if(millis() - lastDeviceSearch > 2000) {
      send('?');
      uint8_t arr[2];
      arr[0] = 'M';
      arr[1] = (uint8_t) deviceId->value;
      send(arr, 2);
      lastDeviceSearch = millis();
    }
  }
}

#define CONFIG_ESPNOW_ENABLE_LONG_RANGE

void ESP_ERROR_CHECK1(esp_err_t err) {
  if(err != ESP_OK) {
    Serial.printf("ESP ERROR: %i\n", error);
  }
}

void beginESPNow() {
  
  // ESP_ERROR_CHECK(esp_netif_init());
  // ESP_ERROR_CHECK(esp_event_loop_create_default());
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));
  ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_start());

  #ifdef CONFIG_ESPNOW_ENABLE_LONG_RANGE
  ESP_ERROR_CHECK(
      esp_wifi_set_protocol(
          ESP_IF_WIFI_STA,
          WIFI_PROTOCOL_LR));
  #endif

  WiFi.setTxPower(WIFI_POWER_19_5dBm);
  // WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  if (esp_now_init() == ESP_OK) {
    Serial.println("ESPNow Init Success");
  }
  else {
    Serial.println("ESPNow Init Failed");
    return;
  }
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.ifidx = ESP_IF_WIFI_STA;
  peerInfo.encrypt = false;

  printError(esp_now_register_recv_cb(receiveCallback));
  printError(esp_now_register_send_cb(sendCallback));
  printError(esp_now_add_peer(&peerInfo));

  Serial.print("STA MAC: "); Serial.println(WiFi.macAddress());

  send('?');
}

uint32_t makeMasterTime = 0;

void makeMaster() {
  isMaster = true;
  makeMasterTime = millis();
}

void sendTrigger(Trigger* trigger, bool response) {
  uint8_t data[sizeof(Trigger) + 1];
  data[0] = response ? 'R' : 'T';
  *((Trigger*) &data[1]) = *trigger;
  send(data, sizeof(Trigger) + 1);
}

void send(const char* str) {
  send((uint8_t*) str, strlen(str));
}

void send(char c) {
  send((uint8_t*) &c, 1);
}

void send(uint8_t* data, size_t size) {
  Serial.print(">>");
  Serial.write(data, size);
  Serial.println();
  printError(esp_now_send(broadcastAddress, data, size));
}

void printError(esp_err_t error) {
  if(error != ESP_OK) {
    Serial.print("ESP NOW Error code: 0x");
    Serial.println(error, 16);
  }
}

void deleteAFKDevices() {
  for (size_t i = 0; i < listedDevicesSize; i++) {
    if(millis() - listedDevices[i].lastRegister > 5000) {
      removeDevice(i);
    }
  }
}

void removeDevice(uint8_t index) {
  listedDevicesSize--;
  for (size_t i = index; i < listedDevicesSize; i++) {
    listedDevices[i] = listedDevices[i+1];
  }
}

void registerDevice(uint8_t id, bool deviceIsMaster) {
  if(deviceIsMaster && millis() - makeMasterTime < 3000) {
    isMaster = false;
    masterFound = true;
  }
  if(id == (uint8_t) deviceId->value) return;
  for (size_t i = 0; i < listedDevicesSize; i++) {
    if(listedDevices[i].deviceId == id) {
      listedDevices[i].lastRegister = millis();
      listedDevices[i].isMaster = deviceIsMaster;
      return;
    }
  }
  if(listedDevicesSize < 10) {
    listedDevices[listedDevicesSize] = ListedDevice{id, millis(), 0, deviceIsMaster};
    listedDevicesSize++;
  }
}

void triggerListedDevice(uint8_t deviceId) {
  for (size_t i = 0; i < listedDevicesSize; i++) {
    if(listedDevices[i].deviceId == deviceId) {
      listedDevices[i].lastTrigger = millis();
      return;
    }
  }
}

void receiveCallback(const uint8_t *mac, const uint8_t *data, int len) {
  Serial.print("<<");
  Serial.write(data, len);
  Serial.println();
  if(len == 0) return;
  if(len == 1) {
    if(data[0] == '?') {
      uint8_t arr[2];
      arr[0] = isMaster ? 'M' : 'S';
      arr[1] = (uint8_t) deviceId->value;
      send(arr, 2);
    }
  }
  if(len == 2) {
    if(data[0] == 'M' || data[0] == 'S') {
      registerDevice(data[1], data[0] == 'M');
    }
  }
  if(data[0] == 'T' && len == sizeof(Trigger) + 1) {
  // if(isMaster && data[0] == 'T' && len == sizeof(Trigger) + 1) {
    Trigger* t = new Trigger;
    *t = *((Trigger*) &data[1]);
    triggerListedDevice(t->triggerer);
    t->ms = millis();
    // sendTrigger(t, true);
    trigger(t);
  }
}

void sendCallback(const uint8_t *mac_addr, esp_now_send_status_t status) {
  if(status != ESP_NOW_SEND_SUCCESS) {
    Serial.println("Sending failed");
  }
}