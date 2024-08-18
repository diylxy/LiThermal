#include "httplib.h"
#include "cameraUtils.h"

httplib::Client cli(CAMERA_HOST_URL);
CameraUtils cameraUtils;

#define IF_CHECK_FILENAME "/proc/net/dev"
#define IF_CHECK_NAME "usb0"
#define IF_CHECK_MAX_SIZE 2048

bool CameraUtils::checkCameraConnection()
{
    static char if_check_buffer[IF_CHECK_MAX_SIZE];
    FILE *f = fopen(IF_CHECK_FILENAME, "r");
    if (f == NULL)
        return false;
    fread(if_check_buffer, IF_CHECK_MAX_SIZE, 1, f);
    fclose(f);
    if (strstr(if_check_buffer, IF_CHECK_NAME) != NULL)
    {
        return true;
    }
    return false;
}

void CameraUtils::setCameraIP()
{
    system("ifconfig usb0 up");
    system("ifconfig usb0 192.168.64.32");
}
