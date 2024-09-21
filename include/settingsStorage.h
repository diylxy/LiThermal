#pragma once
#include <stdint.h>

typedef struct settingsStorage_t
{
    uint32_t __head;

    uint32_t brightness;
    uint32_t colorPalette;
    uint32_t enableGraph;
    uint32_t graphPos;
    uint32_t graphSize;
    uint32_t graphRefreshInterval;
    uint32_t enableMaxValueDisplay;
    uint32_t enableMinValueDisplay;
    uint32_t enableCenterValueDisplay;
    uint32_t preserveUI;
    uint32_t useBlackFlashBang;
    uint32_t __tail;
} settingsStorage_t;
extern settingsStorage_t globalSettings;
#define GRAPH_DATA_SOURCE_MAX 0
#define GRAPH_DATA_SOURCE_MIN 1

void settings_default();
void settings_load();
void settings_save();
