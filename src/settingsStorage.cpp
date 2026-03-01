#include <my_main.h>
#define SETTINGS_HEAD 0x80000001
#define SETTINGS_TAIL 0x1715600D

typedef struct settingsStorage_v1_t
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
    uint32_t use4117Cursors;
    uint32_t __tail;
} settingsStorage_v1_t;

settingsStorage_t globalSettings;

void settings_default()
{
    globalSettings.__head = SETTINGS_HEAD;
    globalSettings.brightness = 170;
    globalSettings.colorPalette = IR_COLOR_PALETTE_DEFAULT;
    globalSettings.enableGraph = false;
    globalSettings.graphPos = 0;
    globalSettings.graphRefreshInterval = 0;
    globalSettings.graphSize = 0;
    globalSettings.enableMaxValueDisplay = false;
    globalSettings.enableMinValueDisplay = false;
    globalSettings.enableCenterValueDisplay = true;
    globalSettings.preserveUI = false;
    globalSettings.useBlackFlashBang = false;
    globalSettings.use4117Cursors = false;
    globalSettings.showBatteryPercent = false;
    globalSettings.__tail = SETTINGS_TAIL;
}

static long file_size_bytes(FILE *fp)
{
    if (!fp)
        return -1;
    long cur = ftell(fp);
    if (cur < 0)
        cur = 0;
    if (fseek(fp, 0, SEEK_END) != 0)
        return -1;
    long end = ftell(fp);
    fseek(fp, cur, SEEK_SET);
    return end;
}

void settings_load()
{
    FILE *fp = fopen(SETTINGS_PATH, "rb");
    if (fp == NULL)
    {
        printf("[Warning] No settings file\n");
        settings_default();
        settings_save();
        return;
    }

    const long sz = file_size_bytes(fp);
    rewind(fp);

    settings_default();

    bool ok = false;

    if (sz == (long)sizeof(settingsStorage_t))
    {
        settingsStorage_t tmp;
        memset(&tmp, 0, sizeof(tmp));
        if (fread(&tmp, sizeof(tmp), 1, fp) == 1)
        {
            if (tmp.__head == SETTINGS_HEAD && tmp.__tail == SETTINGS_TAIL)
            {
                globalSettings = tmp;
                ok = true;
            }
        }
    }
    else if (sz == (long)sizeof(settingsStorage_v1_t))
    {
        settingsStorage_v1_t tmp;
        memset(&tmp, 0, sizeof(tmp));
        if (fread(&tmp, sizeof(tmp), 1, fp) == 1)
        {
            if (tmp.__head == SETTINGS_HEAD && tmp.__tail == SETTINGS_TAIL)
            {
                globalSettings.__head = SETTINGS_HEAD;
                globalSettings.brightness = tmp.brightness;
                globalSettings.colorPalette = tmp.colorPalette;
                globalSettings.enableGraph = tmp.enableGraph;
                globalSettings.graphPos = tmp.graphPos;
                globalSettings.graphSize = tmp.graphSize;
                globalSettings.graphRefreshInterval = tmp.graphRefreshInterval;
                globalSettings.enableMaxValueDisplay = tmp.enableMaxValueDisplay;
                globalSettings.enableMinValueDisplay = tmp.enableMinValueDisplay;
                globalSettings.enableCenterValueDisplay = tmp.enableCenterValueDisplay;
                globalSettings.preserveUI = tmp.preserveUI;
                globalSettings.useBlackFlashBang = tmp.useBlackFlashBang;
                globalSettings.use4117Cursors = tmp.use4117Cursors;
                globalSettings.showBatteryPercent = false;
                globalSettings.__tail = SETTINGS_TAIL;
                ok = true;
            }
        }
    }

    fclose(fp);

    if (!ok)
    {
        printf("[Warning] Corrupted settings storage\n");
        settings_default();
        settings_save();
    }
}

void settings_save()
{
    FILE *fp = fopen(SETTINGS_PATH, "wb");
    if (fp == NULL)
    {
        printf("[Error] Unable to save settings\n");
        return;
    }
    fwrite(&globalSettings, sizeof(globalSettings), 1, fp);
    fclose(fp);
}
