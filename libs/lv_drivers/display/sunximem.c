/**
 * @file sunximem.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "sunximem.h"

#if USE_SUNXIFB_G2D

#include <stdio.h>
#include <ion_mem_alloc.h>

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *      STRUCTURES
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static struct SunxiMemOpsS *memops;

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

bool sunxifb_mem_init() {
    if (memops)
        return true;

    memops = GetMemAdapterOpsS();
    if (SunxiMemOpen(memops) < 0) {
        perror("Error: cannot open ion device");
        return false;
    }

    return true;
}

void sunxifb_mem_deinit() {
    if (memops) {
        SunxiMemClose(memops);
        memops = NULL;
    }
}

void* sunxifb_mem_alloc(size_t size, char *label) {
    if (size == 0) {
        printf("size illegal.");
        return NULL;
    }

    void *alloc = SunxiMemPalloc(memops, size);
    if (alloc == NULL) {
        printf("couldn't allocate memory (%lu bytes).", (unsigned long) size);
        return NULL;
    }

#ifdef LV_USE_SUNXIFB_DEBUG
    printf("%s: sunxifb_mem_alloc=%p size=%lu bytes\n", label, alloc, (unsigned long) size);
#endif /* LV_USE_SUNXIFB_DEBUG */

    return alloc;
}

void sunxifb_mem_free(void **data, char *label) {
    if (*data != NULL) {
#ifdef LV_USE_SUNXIFB_DEBUG
        printf("%s: sunxifb_mem_free=%p\n", label, *data);
#endif /* LV_USE_SUNXIFB_DEBUG */
        SunxiMemPfree(memops, *data);
        *data = NULL;
    } else {
        printf("couldn't free memory.\n");
    }
}

void* sunxifb_mem_get_phyaddr(void *data) {
    if (data != NULL)
        return SunxiMemGetPhysicAddressCpu(memops, data);
    else
        return NULL;
}

void sunxifb_mem_flush_cache(void *data, size_t size) {
    SunxiMemFlushCache(memops, data, size);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

#endif
