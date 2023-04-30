#ifndef PARSE_BMP_H_
#define PARSE_BMP_H_

#include "imgProcCore.h"

typedef struct __attribute__ ((packed)) {
  uint16_t file_signature;
  uint32_t file_size;
  uint32_t res;
  uint32_t offset_pixelArray;
} parseBmp_header_t;

typedef struct __attribute__ ((packed)) {
  uint32_t DIB_header_size;
  uint32_t imgWidth_pxl;
  uint32_t imgHeight_pxl;
  uint16_t imgPlanes;
  uint16_t bitsPerPixel;
  uint32_t compression;
  uint32_t imgDataSize;
  uint32_t xPxlPerMeter;
  uint32_t yPxlPerMeter;
  uint32_t colorsInTable;
  uint32_t impColorCount;
} parseBmp_DID_header_t;

parseBmp_header_t parseBmp_getHeader(char *imgPath);
parseBmp_DID_header_t parseBmp_getDidHeader(char *imgPath);

imgProcCore_colorMap_t parseBmp_getColorMap(char *imgPath);
imgProcCore_monoMap_t parseBmp_getMonoMap(char *imgPath);

uint8_t parseBmp_createMonoImg(imgProcCore_monoMap_t *imgData, char *imgPath);
uint8_t parseBmp_createColorImg(imgProcCore_colorMap_t *imgData, char *imgPath);

#endif
