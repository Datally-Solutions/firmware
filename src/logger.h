#pragma once
#include <Arduino.h>

#include "config.h"

extern String logBuffer;
extern int logLineCount;

void addLog(String message) {
    String timestamp = String(millis() / 1000) + "s — ";
    logBuffer += timestamp + message + "<br>";
    logLineCount++;
    if (logLineCount > MAX_LOG_LINES) {
        int firstBreak = logBuffer.indexOf("<br>");
        logBuffer = logBuffer.substring(firstBreak + 4);
        logLineCount--;
    }
    Serial.println(message);
}
