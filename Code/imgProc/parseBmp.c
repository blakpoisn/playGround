#include "parseBmp.h"

#define HEADER_SIZE (14)
#define COLOR_TABLE_SIZE (0x100)

parseBmp_header_t parseBmp_getHeader(char *imgPath) {
  parseBmp_header_t imgHeader;
  FILE *imgFile;
  
  imgFile = fopen(imgPath, "rb");
  if (imgFile != NULL) {
    fread(&imgHeader, sizeof(parseBmp_header_t), 1, imgFile);
    fclose(imgFile);
  }
  return imgHeader;
}

parseBmp_DID_header_t parseBmp_getDidHeader(char *imgPath) {
  parseBmp_DID_header_t imgDidHeader;
  FILE *imgFile;
  
  imgFile = fopen(imgPath, "rb");
  if (imgFile != NULL) {
    fseek(imgFile, 0x0E, SEEK_SET);
    fread(&imgDidHeader, sizeof(parseBmp_DID_header_t), 1, imgFile);
    fclose(imgFile);
  }
  return imgDidHeader;
}

imgProcCore_colorMap_t parseBmp_getColorMap(char *imgPath) {
  FILE *imgFile;
  imgProcCore_colorMap_t *colorMap = (imgProcCore_colorMap_t*)malloc(sizeof(imgProcCore_colorMap_t));
  colorMap->width = 0;
  colorMap->height = 0;
  parseBmp_header_t *imgHeader = (parseBmp_header_t*)malloc(sizeof(parseBmp_header_t));
  parseBmp_DID_header_t *imgDidHeader = (parseBmp_DID_header_t*)malloc(sizeof(parseBmp_DID_header_t));
  
  imgFile = fopen(imgPath, "rb");
  if (imgFile != NULL) {
    fread(imgHeader, sizeof(parseBmp_header_t), 1, imgFile);
    fread(imgDidHeader, sizeof(parseBmp_DID_header_t), 1, imgFile);
    
    if(imgDidHeader->bitsPerPixel == 24) {
      colorMap->width = imgDidHeader->imgWidth_pxl;
      colorMap->height = imgDidHeader-> imgHeight_pxl;
      uint32_t imgSize = colorMap->width*colorMap->height;
      uint32_t imgDataSize = (imgSize*(imgDidHeader->bitsPerPixel/8));
      uint8_t *imgData = (uint8_t*)malloc(sizeof(uint8_t)*imgDataSize);
      
      fseek(imgFile, imgHeader->offset_pixelArray, SEEK_SET);
      fread(imgData, sizeof(uint8_t), imgDataSize, imgFile);
      
      uint8_t *rData = (uint8_t*)malloc(sizeof(uint8_t)*imgSize);
      uint8_t *gData = (uint8_t*)malloc(sizeof(uint8_t)*imgSize);
      uint8_t *bData = (uint8_t*)malloc(sizeof(uint8_t)*imgSize);
      for (uint32_t i = 0; i < imgSize; i++) {
        rData[i] = imgData[(i*3 + 2)];
        gData[i] = imgData[(i*3 + 1)];
        bData[i] = imgData[(i*3 + 0)];
      }
      colorMap->rData = rData;
      colorMap->gData = gData;
      colorMap->bData = bData;
    }
    fclose(imgFile);
  }
  return colorMap[0];
}

imgProcCore_monoMap_t parseBmp_getMonoMap(char *imgPath) {
  FILE *imgFile;
  imgProcCore_monoMap_t *imgMap = (imgProcCore_monoMap_t*)malloc(sizeof(imgProcCore_monoMap_t));
  imgMap->width = 0;
  imgMap->height = 0;
  parseBmp_header_t *imgHeader = (parseBmp_header_t*)malloc(sizeof(parseBmp_header_t));
  parseBmp_DID_header_t *imgDidHeader = (parseBmp_DID_header_t*)malloc(sizeof(parseBmp_DID_header_t));
  
  imgFile = fopen(imgPath, "rb");
  if (imgFile != NULL) {
    fread(imgHeader, sizeof(parseBmp_header_t), 1, imgFile);
    fread(imgDidHeader, sizeof(parseBmp_DID_header_t), 1, imgFile);
    
    if(imgDidHeader->bitsPerPixel == 8) {
      imgMap->width = imgDidHeader->imgWidth_pxl;
      imgMap->height = imgDidHeader->imgHeight_pxl;
      uint32_t imgSize = imgMap->width * imgMap->height;
      uint8_t *imgData = (uint8_t*)malloc(sizeof(uint8_t) * imgSize);
      
      fseek(imgFile, imgHeader->offset_pixelArray, SEEK_SET);
      fread(imgData, sizeof(uint8_t), imgSize, imgFile);
      imgMap->yData = imgData;
    }
    fclose(imgFile);
  }
  return imgMap[0];
}

uint8_t parseBmp_createMonoImg(imgProcCore_monoMap_t *imgData, char *imgPath) {
  FILE *imgFile;
  imgFile = fopen(imgPath, "wb");
  if (imgFile != NULL) {
    parseBmp_header_t *header = (parseBmp_header_t*)malloc(sizeof(parseBmp_header_t));
    uint8_t ch[2] = {'B', 'M'};
    header->file_signature = *((uint16_t*)ch);
    header->res = 0;
    header->offset_pixelArray = sizeof(parseBmp_header_t) + sizeof(parseBmp_DID_header_t) + (COLOR_TABLE_SIZE * 4);
    header->file_size = header->offset_pixelArray + (imgData->width * imgData->height);
    fwrite(header, sizeof(parseBmp_header_t), 1, imgFile);
    
    parseBmp_DID_header_t* did_header = (parseBmp_DID_header_t*)malloc(sizeof(parseBmp_DID_header_t));
    did_header->DIB_header_size = sizeof(parseBmp_DID_header_t);
    did_header->imgWidth_pxl = imgData->width;
    did_header->imgHeight_pxl = imgData->height;
    did_header->imgPlanes = 1;
    did_header->bitsPerPixel = 8;
    did_header->compression = 0;
    did_header->imgDataSize = imgData->width * imgData->height;
    did_header->xPxlPerMeter = 0;
    did_header->yPxlPerMeter = 0;
    did_header->colorsInTable = COLOR_TABLE_SIZE;
    did_header->impColorCount = 0;
    fwrite(did_header, sizeof(parseBmp_DID_header_t), 1, imgFile);
    
    uint32_t colors[COLOR_TABLE_SIZE] = { 0 };
    for (uint32_t idx = 0; idx < COLOR_TABLE_SIZE; idx++) {
      colors[idx] = 0x0000000 | (idx << 16) | (idx << 8) | (idx);
    }
    fwrite(colors, (COLOR_TABLE_SIZE * 4), 1, imgFile);
    fwrite(imgData->yData, did_header->imgDataSize, 1, imgFile);
    
    fclose(imgFile);
  }
  else {
    return 0; // -ve result
  }
  return 1; // +ve result
}

uint8_t parseBmp_createColorImg(imgProcCore_colorMap_t *imgData, char *imgPath) {
  FILE *imgFile;
  imgFile = fopen(imgPath, "wb");
  if (imgFile != NULL) {
    parseBmp_header_t *header = (parseBmp_header_t*)malloc(sizeof(parseBmp_header_t));
    uint8_t ch[2] = {'B', 'M'};
    header->file_signature = *((uint16_t*)ch);
    header->offset_pixelArray = sizeof(parseBmp_header_t) + sizeof(parseBmp_DID_header_t);
    header->res = 0;
    header->file_size = header->offset_pixelArray + (imgData->width * imgData->height);
    fwrite(header, sizeof(parseBmp_header_t), 1, imgFile);
    
    parseBmp_DID_header_t* did_header = (parseBmp_DID_header_t*)malloc(sizeof(parseBmp_DID_header_t));
    did_header->DIB_header_size = sizeof(parseBmp_DID_header_t);
    did_header->imgWidth_pxl = imgData->width;
    did_header->imgHeight_pxl = imgData->height;
    did_header->imgPlanes = 1;
    did_header->bitsPerPixel = 24;
    did_header->compression = 0;
    did_header->imgDataSize = imgData->width * imgData->height * 3;
    did_header->xPxlPerMeter = 0;
    did_header->yPxlPerMeter = 0;
    did_header->colorsInTable = 0;
    did_header->impColorCount = 0;
    fwrite(did_header, sizeof(parseBmp_DID_header_t), 1, imgFile);
    
    uint8_t* rgbData = (uint8_t*)malloc(did_header->imgDataSize * sizeof(uint8_t));
    for (uint32_t i = 0; i < (imgData->width * imgData->height); i++) {
      rgbData[i*3 + 2] = imgData->rData[i];
      rgbData[i*3 + 1] = imgData->gData[i];
      rgbData[i*3 + 0] = imgData->bData[i];
    }
    fwrite(rgbData, did_header->imgDataSize, 1, imgFile);
    
    fclose(imgFile);
  }
  return 1; // +ve result
}
