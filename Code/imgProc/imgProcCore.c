#include "imgProcCore.h"

imgProcCore_monoMap_t imgProcCore_covertToGray(imgProcCore_colorMap_t* clrMap) {
  imgProcCore_monoMap_t* monoMap = (imgProcCore_monoMap_t*)malloc(sizeof(imgProcCore_monoMap_t));
  monoMap->width = clrMap->width;
  monoMap->height = clrMap->height;
  if((monoMap->width > 0) && (monoMap->height > 0)) {
    uint32_t imagePxls = monoMap->width * monoMap->height;
    uint8_t* grayValues = (uint8_t*)malloc(imagePxls * sizeof(uint8_t));
    for (uint32_t i = 0; i < imagePxls; i++) {
      float grayFloat = (0.299 * clrMap->rData[i]) + (0.587 * clrMap->gData[i]) + (0.114 * clrMap->bData[i]);
      grayValues[i] = (uint8_t)(grayFloat + 0.5);
    }
    monoMap->yData = grayValues;
  }
  return monoMap[0];
}
