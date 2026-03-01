#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

bool recorder_start(const char *filename, int width, int height, int fps);
void recorder_stop();
bool recorder_is_recording();
void recorder_submit_bgra(const uint8_t *bgra, int stride, int width, int height);

#ifdef __cplusplus
}
#endif
