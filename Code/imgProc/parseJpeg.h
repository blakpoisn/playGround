#ifndef PARSE_JPEG_H_
#define PARSE_JPEG_H_

#include "imgProcCore.h"

typedef enum {
  parseJpeg_MARKER_SOI,     // 0xFF D8 - Start of Image
  parseJpeg_MARKER_APP0,    // 0xFF E0 - Application Default Header
  parseJpeg_MARKER_QUANT,   // 0xFF DB - Quantisation Table
  parseJpeg_MARKER_SOF,     // 0xFF C0 - Start of Frame
  parseJpeg_MARKER_HUFF,    // 0xFF C4 - Define Huffman Table
  parseJpeg_MARKER_SOS,     // 0xFF DA - Start of Scan
  parseJpeg_MARKER_EOI      // 0xFF D9 - End of Image
} parseJpeg_marker_te;

imgProcCore_monoMap_t parseJpeg_getMonoMap(char* imgPath);


#endif
