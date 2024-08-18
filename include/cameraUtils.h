#pragma once
#define CAMERA_HOST_URL "http://192.168.64.64"

class CameraUtils
{
public:
    bool checkCameraConnection(); // 检查摄像头连接
    void setCameraIP();           // 设置摄像头固定IP
};

extern CameraUtils cameraUtils;