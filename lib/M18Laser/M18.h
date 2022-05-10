#include <Arduino.h>

class M18 {
public:
    M18(uint8_t pin, bool isNPN, bool inverted = false) : npn(isNPN), pin(pin), inverted(inverted) {}

    void begin() {
        pinMode(pin, INPUT_PULLUP);
    }

    bool isReflected() {
        bool result = false;
        if(npn) {
            result = analogRead(pin) < 4000;
        } else {
            result = false;
        }
        return (inverted) ? !result : result;
    }

    void setNPN(bool isNPN) {
        this->npn = npn;
    }

    uint8_t getPin() {
        return pin;
    }

    bool isNPN() {
        return npn;
    }

    void setInverted(bool inverted) {
        this->inverted = inverted;
    }

    bool isInverted() {
        return inverted;
    }

private:
    bool npn;
    uint8_t pin;
    bool inverted;
};