#pragma once
#include "my_main.h"

class VideoPlayer
{
private:
public:
    // cv::VideoCapture video;
    lv_obj_t *img_obj;
    void init();
    void connect();
    void play();
    void disconnect();
};

extern VideoPlayer videoPlayer;