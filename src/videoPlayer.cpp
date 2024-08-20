#include "videoPlayer.h"
#include <sys/time.h>
#include <semaphore.h>
#include "videoCodec.h"
VideoPlayer videoPlayer;
uint8_t IR_frame_buffer[320 * 240 * 4];
pthread_t thread_image_ref;
static sem_t sem_video;
#define CMD_NONE 0
#define CMD_CONNECT 1
#define CMD_PLAY 2
#define CMD_PAUSE 3
#define CMD_DISCONNECT 4

int thread_video_command = CMD_NONE;

static lv_img_dsc_t img_ir_frame;
static void createImage(bool isResume)
{
    videoPlayer.img_obj = lv_img_create(lv_scr_act());
    lv_obj_set_size(videoPlayer.img_obj, 320, 240);
    img_ir_frame.header.always_zero = 0;
    img_ir_frame.header.w = 320;
    img_ir_frame.header.h = 240;
    img_ir_frame.header.cf = LV_IMG_CF_TRUE_COLOR;
    img_ir_frame.data = IR_frame_buffer;
    img_ir_frame.data_size = 320 * 240 * 4;
    lv_img_set_src(videoPlayer.img_obj, &img_ir_frame);
    lv_obj_align(videoPlayer.img_obj, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_opa(videoPlayer.img_obj, 0, 0);
}
static void destroyImage()
{
    if (lv_obj_is_valid(videoPlayer.img_obj))
    {
        lv_my_anim_fall_down(videoPlayer.img_obj);
        videoPlayer.img_obj = NULL;
    }
}
uint64_t getTimeStampUS()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000L + tv.tv_usec / 1000L;
}

#define STATE_IDLE 1
#define STATE_PLAYING 2
#define STATE_PAUSED 3

static bool connected = false;
static int current_state = STATE_IDLE;

void *thread_refresh_image(void *)
{
    static int err_count = 0;
    sem_wait(&sem_video);
    while (1)
    {
        switch (thread_video_command)
        {
        case CMD_CONNECT:
        {
            printf("Connceting to RTSP...\n");
            if (codec_openStream(VIDEO_STREAM_URL))
            {
                current_state = STATE_PLAYING;
                connected = true;
                cameraUtils.connected = true;
                pthread_mutex_lock(&lv_mutex);
                createImage(false);
                pthread_mutex_unlock(&lv_mutex);
                printf("Stream open success\n");
            }
            else
            {
                printf("Stream open fail\n");
                codec_closeEverything();
                sleep(1);
            }
        }
        break;
        case CMD_PLAY:
        {
            if (connected)
            {
                current_state = STATE_PLAYING;
            }
        }
        break;
        case CMD_PAUSE:
        {
            if (connected)
            {
                current_state = STATE_PAUSED;
            }
        }
        break;
        case CMD_DISCONNECT:
        {
            if (connected)
            {
                current_state = STATE_IDLE;
                connected = false;
                pthread_mutex_lock(&lv_mutex);
                destroyImage();
                pthread_mutex_unlock(&lv_mutex);
                codec_closeEverything();
            }
        }
        break;
        default:
            break;
        }
        thread_video_command = CMD_NONE;
        switch (current_state)
        {
        case STATE_IDLE:
            sem_wait(&sem_video);
            break;
        case STATE_PLAYING:
        {
            auto frame = codec_getFrame();
            if (frame == NULL)
            {
                printf("empty\n");
                ++err_count;
                if (err_count > 10)
                {
                    current_state = STATE_IDLE;
                    connected = false;
                    codec_closeEverything();
                }
                continue;
            }
            err_count = 0;
            av_image_copy_to_buffer(IR_frame_buffer, sizeof(IR_frame_buffer),
                                    (const uint8_t *const *)frame->data, (const int *)frame->linesize,
                                    AV_PIX_FMT_RGBA, frame->width, frame->height, 1);
            av_frame_free(&frame);
            if (lv_obj_get_style_opa(videoPlayer.img_obj, 0) == 0)
            {
                lv_obj_fade_in(videoPlayer.img_obj, 500, 0);
            }
            lv_obj_invalidate(videoPlayer.img_obj);
            usleep(10000);
            sem_trywait(&sem_video);
        }
        break;
        case STATE_PAUSED:
            sem_wait(&sem_video);
            break;
        default:
            break;
        }
    }
    return NULL;
}

void VideoPlayer::init()
{
    thread_image_ref = CMD_NONE;
    sem_init(&sem_video, 0, 0);
    avformat_network_init();
    pthread_create(&thread_image_ref, NULL, thread_refresh_image, NULL);
}

void VideoPlayer::connect()
{
    thread_video_command = CMD_CONNECT;
    sem_post(&sem_video);
}

void VideoPlayer::play()
{
    thread_video_command = CMD_PLAY;
    sem_post(&sem_video);
}

void VideoPlayer::disconnect()
{
    thread_video_command = CMD_DISCONNECT;
    sem_post(&sem_video);
}
