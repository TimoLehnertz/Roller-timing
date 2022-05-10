#include "buzzerMusic.h"

int beepBeep[] = {
  NOTE_DS6,10, NOTE_DS7,7,NOTE_DS6,10
};

int beepLong[] = {
  NOTE_DS6,-5, NOTE_DS7,-5
};

int beepStart[] = {
  NOTE_DS6,-5, NOTE_DS7,-20, NOTE_DS6,5
};

int beepShortLow[] = {
  NOTE_E5,10, NOTE_E5,10
};

int beepShortHigh[] = {
  NOTE_D8,10, NOTE_D8,10
};

int beepOn[] = {
  NOTE_AS5,10, NOTE_AS5,20,NOTE_DS8,10
};

int beepOff[] = {
  NOTE_D8,10, NOTE_DS8,20,NOTE_F5,10
};

int marioBros[] = {

  // Super Mario Bros theme
  // Score available at https://musescore.com/user/2123/scores/2145
  // Theme by Koji Kondo
  
  
//   NOTE_E5,8, NOTE_E5,8, REST,8, NOTE_E5,8, REST,8, NOTE_C5,8, NOTE_E5,8, //1
//   NOTE_G5,4, REST,4, NOTE_G4,8, REST,4, 
//   NOTE_C5,-4, NOTE_G4,8, REST,4, NOTE_E4,-4, // 3
//   NOTE_A4,4, NOTE_B4,4, NOTE_AS4,8, NOTE_A4,4,
//   NOTE_G4,-8, NOTE_E5,-8, NOTE_G5,-8, NOTE_A5,4, NOTE_F5,8, NOTE_G5,8,
//   REST,8, NOTE_E5,4,NOTE_C5,8, NOTE_D5,8, NOTE_B4,-4,
//   NOTE_C5,-4, NOTE_G4,8, REST,4, NOTE_E4,-4, // repeats from 3
//   NOTE_A4,4, NOTE_B4,4, NOTE_AS4,8, NOTE_A4,4,
//   NOTE_G4,-8, NOTE_E5,-8, NOTE_G5,-8, NOTE_A5,4, NOTE_F5,8, NOTE_G5,8,
//   REST,8, NOTE_E5,4,NOTE_C5,8, NOTE_D5,8, NOTE_B4,-4,

  
//   REST,4, NOTE_G5,8, NOTE_FS5,8, NOTE_F5,8, NOTE_DS5,4, NOTE_E5,8,//7
//   REST,8, NOTE_GS4,8, NOTE_A4,8, NOTE_C4,8, REST,8, NOTE_A4,8, NOTE_C5,8, NOTE_D5,8,
//   REST,4, NOTE_DS5,4, REST,8, NOTE_D5,-4,
//   NOTE_C5,2, REST,2,

//   REST,4, NOTE_G5,8, NOTE_FS5,8, NOTE_F5,8, NOTE_DS5,4, NOTE_E5,8,//repeats from 7
//   REST,8, NOTE_GS4,8, NOTE_A4,8, NOTE_C4,8, REST,8, NOTE_A4,8, NOTE_C5,8, NOTE_D5,8,
//   REST,4, NOTE_DS5,4, REST,8, NOTE_D5,-4,
//   NOTE_C5,2, REST,2,

//   NOTE_C5,8, NOTE_C5,4, NOTE_C5,8, REST,8, NOTE_C5,8, NOTE_D5,4,//11
//   NOTE_E5,8, NOTE_C5,4, NOTE_A4,8, NOTE_G4,2,

//   NOTE_C5,8, NOTE_C5,4, NOTE_C5,8, REST,8, NOTE_C5,8, NOTE_D5,8, NOTE_E5,8,//13
//   REST,1, 
  NOTE_C5,8, NOTE_C5,4, NOTE_C5,8, REST,8, NOTE_C5,8, NOTE_D5,4,
  NOTE_E5,8, NOTE_C5,4, NOTE_A4,8, NOTE_G4,2,
  NOTE_E5,8, NOTE_E5,8, REST,8, NOTE_E5,8, REST,8, NOTE_C5,8, NOTE_E5,4,
  NOTE_G5,4, REST,4, NOTE_G4,4, REST,4, 
//   NOTE_C5,-4, NOTE_G4,8, REST,4, NOTE_E4,-4, // 19
  
//   NOTE_A4,4, NOTE_B4,4, NOTE_AS4,8, NOTE_A4,4,
//   NOTE_G4,-8, NOTE_E5,-8, NOTE_G5,-8, NOTE_A5,4, NOTE_F5,8, NOTE_G5,8,
//   REST,8, NOTE_E5,4, NOTE_C5,8, NOTE_D5,8, NOTE_B4,-4,

//   NOTE_C5,-4, NOTE_G4,8, REST,4, NOTE_E4,-4, // repeats from 19
//   NOTE_A4,4, NOTE_B4,4, NOTE_AS4,8, NOTE_A4,4,
//   NOTE_G4,-8, NOTE_E5,-8, NOTE_G5,-8, NOTE_A5,4, NOTE_F5,8, NOTE_G5,8,
//   REST,8, NOTE_E5,4, NOTE_C5,8, NOTE_D5,8, NOTE_B4,-4,

//   NOTE_E5,8, NOTE_C5,4, NOTE_G4,8, REST,4, NOTE_GS4,4,//23
//   NOTE_A4,8, NOTE_F5,4, NOTE_F5,8, NOTE_A4,2,
//   NOTE_D5,-8, NOTE_A5,-8, NOTE_A5,-8, NOTE_A5,-8, NOTE_G5,-8, NOTE_F5,-8,
  
//   NOTE_E5,8, NOTE_C5,4, NOTE_A4,8, NOTE_G4,2, //26
//   NOTE_E5,8, NOTE_C5,4, NOTE_G4,8, REST,4, NOTE_GS4,4,
//   NOTE_A4,8, NOTE_F5,4, NOTE_F5,8, NOTE_A4,2,
//   NOTE_B4,8, NOTE_F5,4, NOTE_F5,8, NOTE_F5,-8, NOTE_E5,-8, NOTE_D5,-8,
//   NOTE_C5,8, NOTE_E4,4, NOTE_E4,8, NOTE_C4,2,

//   NOTE_E5,8, NOTE_C5,4, NOTE_G4,8, REST,4, NOTE_GS4,4,//repeats from 23
//   NOTE_A4,8, NOTE_F5,4, NOTE_F5,8, NOTE_A4,2,
//   NOTE_D5,-8, NOTE_A5,-8, NOTE_A5,-8, NOTE_A5,-8, NOTE_G5,-8, NOTE_F5,-8,
  
//   NOTE_E5,8, NOTE_C5,4, NOTE_A4,8, NOTE_G4,2, //26
//   NOTE_E5,8, NOTE_C5,4, NOTE_G4,8, REST,4, NOTE_GS4,4,
//   NOTE_A4,8, NOTE_F5,4, NOTE_F5,8, NOTE_A4,2,
//   NOTE_B4,8, NOTE_F5,4, NOTE_F5,8, NOTE_F5,-8, NOTE_E5,-8, NOTE_D5,-8,
//   NOTE_C5,8, NOTE_E4,4, NOTE_E4,8, NOTE_C4,2,
//   NOTE_C5,8, NOTE_C5,4, NOTE_C5,8, REST,8, NOTE_C5,8, NOTE_D5,8, NOTE_E5,8,
//   REST,1,

//   NOTE_C5,8, NOTE_C5,4, NOTE_C5,8, REST,8, NOTE_C5,8, NOTE_D5,4, //33
//   NOTE_E5,8, NOTE_C5,4, NOTE_A4,8, NOTE_G4,2,
//   NOTE_E5,8, NOTE_E5,8, REST,8, NOTE_E5,8, REST,8, NOTE_C5,8, NOTE_E5,4,
//   NOTE_G5,4, REST,4, NOTE_G4,4, REST,4, 
//   NOTE_E5,8, NOTE_C5,4, NOTE_G4,8, REST,4, NOTE_GS4,4,
//   NOTE_A4,8, NOTE_F5,4, NOTE_F5,8, NOTE_A4,2,
//   NOTE_D5,-8, NOTE_A5,-8, NOTE_A5,-8, NOTE_A5,-8, NOTE_G5,-8, NOTE_F5,-8,
  
//   NOTE_E5,8, NOTE_C5,4, NOTE_A4,8, NOTE_G4,2, //40
//   NOTE_E5,8, NOTE_C5,4, NOTE_G4,8, REST,4, NOTE_GS4,4,
//   NOTE_A4,8, NOTE_F5,4, NOTE_F5,8, NOTE_A4,2,
//   NOTE_B4,8, NOTE_F5,4, NOTE_F5,8, NOTE_F5,-8, NOTE_E5,-8, NOTE_D5,-8,
//   NOTE_C5,8, NOTE_E4,4, NOTE_E4,8, NOTE_C4,2,
  
//   //game over sound
//   NOTE_C5,-4, NOTE_G4,-4, NOTE_E4,4, //45
//   NOTE_A4,-8, NOTE_B4,-8, NOTE_A4,-8, NOTE_GS4,-8, NOTE_AS4,-8, NOTE_GS4,-8,
//   NOTE_G4,8, NOTE_D4,8, NOTE_E4,-2,  

};

void BackgroudPlayer::begin() {
    ledcSetup(0, 2000, 11); // channel, max frequency, resolution
    ledcAttachPin(pin, 0);
}

void BackgroudPlayer::playBeepBeep() {
    playNotes(beepBeep, 6);
}

void BackgroudPlayer::playBeepLong() {
    playNotes(beepLong, 4);
}

void BackgroudPlayer::playBeepStart() {
    playNotes(beepStart, 6);
}

void BackgroudPlayer::playBeepOn() {
    playNotes(beepOn, 6);
}

void BackgroudPlayer::playBeepOff() {
    playNotes(beepOff, 6);
}

void BackgroudPlayer::playMarioBros() {
    playNotes(marioBros, sizeof(marioBros) / sizeof(int));
}

void BackgroudPlayer::playBeepShortHigh() {
    playNotes(beepShortHigh, 4);
}

void BackgroudPlayer::playBeepShortLow() {
    playNotes(beepShortLow, 4);
}

void BackgroudPlayer::setDisabled(bool disabled) {
  if(!this->disabled == disabled) {
    ledcWriteTone(0, 0);
  }
  this->disabled = disabled;
}

void BackgroudPlayer::handle() {
    if(!running || disabled) return;
    // sizeof gives the number of bytes, each int value is composed of two bytes (16 bits)
    // there are two values per note (pitch and duration), so for each note there are four bytes

    // this calculates the duration of a whole note in ms
    int wholenote = (60000 * 4) / tempo;

    int divider = 0, noteDuration = 0;

    divider = melody[current + 1];
    if (divider > 0) {
        // regular note, just proceed
        noteDuration = (wholenote) / divider;
    } else if (divider < 0) {
        // dotted notes are represented with negative durations!!
        noteDuration = (wholenote) / abs(divider);
        noteDuration *= 1.5; // increases the duration in half for dotted notes
    }

    uint32_t passed = millis() - start;

    if(passed > lastNoteStart + noteDuration * 0.9) {
        ledcWriteTone(0, 0);
    }

    if(passed > lastNoteStart + noteDuration) {
        lastNoteStart = passed;
        ledcWriteTone(0, melody[current + 2]);
        current += 2;
        if(current >= count) {
            ledcWriteTone(0, 0);
            running = false;
        }
    }
}

void BackgroudPlayer::playNotes(int* melody, size_t count) {
    this->melody = melody;
    this->count = count;
    start = millis();
    lastNoteStart = 0;
    running = true;
    current = 2;
    ledcWriteTone(0, melody[0]);
}

BackgroudPlayer bgPlayer(27);