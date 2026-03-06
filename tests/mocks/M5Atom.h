#pragma once
#include "Arduino.h"

struct Display {
    void fillpix(uint32_t color) {}
};

struct M5AtomClass {
    Display dis;
};

extern M5AtomClass M5;
M5AtomClass M5;
