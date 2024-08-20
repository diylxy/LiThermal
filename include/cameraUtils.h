#pragma once
#define CAMERA_HOST_URL "http://192.168.64.64"

typedef struct temperature_point_t
{
    float maxTemperature;
    float minTemperature;
    float averageTemperature;
    struct
    {
        float positionX;
        float positionY;
    } MaxTemperaturePoint, MinTemperaturePoint;
    bool isFreezedata;
} temperature_point_t;

// 摄像头调色板
#define IR_COLOR_PALETTE_WHITEHOT 0
#define IR_COLOR_PALETTE_BLACKHOT 1
#define IR_COLOR_PALETTE_FUSION1 2
#define IR_COLOR_PALETTE_FUSION2 3
#define IR_COLOR_PALETTE_RAINBOW 4
#define IR_COLOR_PALETTE_IRONBOW1 5
#define IR_COLOR_PALETTE_IRONBOW2 6
#define IR_COLOR_PALETTE_SEPIA 7
#define IR_COLOR_PALETTE_COLOR1 8
#define IR_COLOR_PALETTE_COLOR2 9
#define IR_COLOR_PALETTE_ICEFIRE 10
#define IR_COLOR_PALETTE_RAIN 11
#define IR_COLOR_PALETTE_REDHOT 12
#define IR_COLOR_PALETTE_GREENHOT 13
#define IR_COLOR_PALETTE_DEEPBLUE 14
#define IR_COLOR_PALETTE_MAX 15
#define IR_COLOR_PALETTE_DEFAULT IR_COLOR_PALETTE_FUSION1

// 数字降噪模式
#define IR_DNR_MODE_CLOSE 0
#define IR_DNR_MODE_GENERAL 1
#define IR_DNR_MODE_ADVANCED 2
class CameraUtils
{
public:
    bool connected = false;
    bool checkCameraConnection(); // 检查摄像头连接
    void setCameraIP();           // 设置摄像头固定IP
    void initHTTPClient();        // 初始化HTTP客户端（即设置HTTP认证）
    void getTemperature(temperature_point_t *result);        // 获取摄像头温度信息
    void setColorPalette(int palette);
    void setDigitalNoiceReduce(int mode, int frameLevel, int interFrameLevel);
    void setDigitalDetailEnhancement(bool en, int level);
    float readJpegWithExtra(const char *save_filename = NULL, int result_x = 160 / 2, int result_y = 120 / 2);
};

extern CameraUtils cameraUtils;