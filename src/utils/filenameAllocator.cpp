#include <my_main.h>
#include <stdio.h>
#include <string.h>
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <dirent.h>
#include <stdint.h>
#include <sys/types.h>
#include <utils/filenameAllocator.h>
#define GALLERY_MAX_IMAGES 100000 // 必须为8的倍数
static uint8_t image_hashmap[GALLERY_MAX_IMAGES / 8];
static const uint32_t countTable[256] = {
    0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8};

int getTotalImages()
{
    int total = 0;
    for (int i = 0; i < sizeof(image_hashmap); ++i)
    {
        total += countTable[image_hashmap[i]];
    }
    return total;
}

int getLastImageID()            // 获取可用的文件名
{
    int i = 0;
    int j = 0;
    for (i = sizeof(image_hashmap) - 1; i >= 0; --i)
    {
        if (image_hashmap[i] != 0)
            break;
    }
    if (i < 0)
        return 0;
    if (image_hashmap[i] == 0xff)
    {
        if (i == sizeof(image_hashmap) - 1)
            return GALLERY_MAX_IMAGES;
        return (i + 1) * 8;
    }
    for (j = 7; j >= 0; --j)
    {
        if ((1 << j) & image_hashmap[i])
        {
            return i * 8 + j + 1;
        }
    }
    return i * 8;
}

int getNextImage(int current)
{
    int result;
    int i, j;
    if (current < 0)
        current = -1;
    current += 1;
    i = current / 8;
    j = current % 8;
    for (; i < sizeof(image_hashmap); ++i)
    {
        for (; j < 8; ++j)
        {
            if ((1 << j) & image_hashmap[i])
            {
                return i * 8 + j;
            }
        }
        j = 0;
    }
    if (current == 0)
        return -1;
    else
        return getNextImage(-1);
}

int getPrevImage(int current)
{
    int result;
    int i, j;
    if (current <= 0)
        current = GALLERY_MAX_IMAGES;
    current -= 1;
    i = current / 8;
    j = current % 8;
    for (; i >= 0; --i)
    {
        for (; j >= 0; --j)
        {
            if ((1 << j) & image_hashmap[i])
            {
                return i * 8 + j;
            }
        }
        j = 7;
    }
    if (current == GALLERY_MAX_IMAGES - 1)
        return -1;
    else
        return getPrevImage(GALLERY_MAX_IMAGES);
}

void readFiles(const char *dirToOpen)
{
    DIR *dir;
    struct dirent *entry;
    int id;
    static char ext[10];
    memset(image_hashmap, 0, sizeof(image_hashmap));
    if (!(dir = opendir(dirToOpen)))
        return;
    if (!(entry = readdir(dir)))
        return;
    do
    {
        if (entry->d_type != DT_DIR)
        {
            // 避免缓冲区溢出
            int len = strlen(entry->d_name);
            int ext_pos = strchr(entry->d_name, '.') - entry->d_name;
            if (ext_pos - len > 6)
                continue;
            int param_result = sscanf(entry->d_name, "CAP%d.%s", &id, ext);
            if (param_result != 2)
                continue;
            image_hashmap[id / 8] |= 1 << (id % 8);
            // 注意不要在这里统计图片总数，因为可能有文件名相同的情况
        }
    } while (entry = readdir(dir));
    closedir(dir);
}
static char newFilename[64];
const char *allocateNewFilename()
{
    int id = getLastImageID();
    if (id >= GALLERY_MAX_IMAGES)
        return NULL;
    sprintf(newFilename, GALLERY_PATH "/CAP%05d", id);
    image_hashmap[id / 8] |= 1 << (id % 8);
    printf("Allocated filename: %s\n", newFilename);
    return newFilename;
}

void freeFileName(int id)
{
    if (id >= GALLERY_MAX_IMAGES || id < 0)
        return ;
    image_hashmap[id / 8] &= ~(1 << (id % 8));
}