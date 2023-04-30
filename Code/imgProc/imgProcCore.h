#ifndef IMAGE_PROCESSING_CORE_H_
#define IMAGE_PROCESSING_CORE_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct {
  uint8_t *rData;
  uint8_t *gData;
  uint8_t *bData;
  uint32_t width;
  uint32_t height;
} imgProcCore_colorMap_t;

typedef struct {
  uint8_t *yData;
  uint32_t width;
  uint32_t height;
} imgProcCore_monoMap_t;

imgProcCore_monoMap_t imgProcCore_covertToGray(imgProcCore_colorMap_t* clrMap);

#endif
