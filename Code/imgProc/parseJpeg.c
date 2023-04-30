#include "parseJpeg.h"

#define IS_MARKER(x) ((x[0] == 0xFF) && (x[1] != 0x00) && (x[1] != 0xFF))
#define IS_MARKER_WITH_SIZE(x) ((x[1] != 0xFF01) && !((x[1] >= 0xD0) && (x[1] <= 0xD9)))
#define FLIP_BYTES(x) ((uint16_t)x[1] | ((uint16_t)x[0] << 8))

typedef struct segment_node {
  uint8_t marker_code;
  uint32_t data_size;
  uint8_t* data_ptr;
  struct segment_node* next_node;
} segment_node_t;

static char* getMarkerString(uint8_t);
static segment_node_t* getSegmentedData(char*);
static void parseComment(uint8_t*, uint32_t);
static void parseDQT(uint8_t*, uint32_t);

imgProcCore_monoMap_t parseJpeg_getMonoMap(char* imgPath) {
  imgProcCore_monoMap_t* imgMap = (imgProcCore_monoMap_t*)malloc(sizeof(imgProcCore_monoMap_t));
  imgMap->yData = NULL;
  imgMap->width = 0;
  imgMap->height = 0;
  
  segment_node_t* segments_head = getSegmentedData(imgPath);
  
  segment_node_t* segment_ptr = segments_head;
  uint32_t pos = 0;
  while(segment_ptr != NULL) {
    printf("\n\n\t[0x%02X | %s]", segment_ptr->marker_code, getMarkerString(segment_ptr->marker_code));
    printf("\n\t→ Size: %d", segment_ptr->data_size);
    for (uint32_t i = 0; i < segment_ptr->data_size; i++) {
      if (i%0x20 == 0) printf("\n\t\t");
      else printf(" ");
      printf("%02X", segment_ptr->data_ptr[i]);
    }
    switch(segment_ptr->marker_code) {
      case 0xFE:
        parseComment(segment_ptr->data_ptr, segment_ptr->data_size);
        break;
      case 0xDB:
        parseDQT(segment_ptr->data_ptr, segment_ptr->data_size);
        break;
    }
    
    segment_ptr = segment_ptr->next_node;
  }
  
  return imgMap[0];
}

static char* getMarkerString(uint8_t x) {
  char* str;
  if ((x >= 0xE1) && (x <= 0xEF)) x = 0xE1; //App specific headers
  switch(x) {
    case 0x00:
      str = "Image Data - Entropy Coded Segment";
      break;
    case 0xD8:
      str = "SOI, Start of Image";
      break;
    case 0xD9:
      str = "EOI, End of Image";
      break;
    case 0xE0:
      str = "APP0, Application Default Header";
      break;
    case 0xE1:
      str = "APPn, Application Specific Header";
      break;
    case 0xDB:
      str = "DQT, Quantisation Table";
      break;
    case 0xC0:
      str = "SOF0, Start of Frame - Baseline DCT";
      break;
    case 0xC1:
      str = "SOF1, Start of Frame - Extended Sequencial DCT";
      break;
    case 0xC2:
      str = "SOF2, Start of Frame - Progressive DCT";
      break;
    case 0xC3:
      str = "SOF3, Start of Frame - Lossless sequencial";
      break;
    case 0xC4:
      str = "Define Huffman Table";
      break;
    case 0xDA:
      str = "SOS, Start of Scan";
      break;
    case 0xFE:
      str = "Comments";
      break;
    default:
      str = "Unknown";
  }
  return str;
}

static segment_node_t* getSegmentedData(char* imgPath) {
  segment_node_t* dummy_node = (segment_node_t*)malloc(sizeof(segment_node_t));
  dummy_node->next_node = NULL;
  segment_node_t* node_ptr = dummy_node;
  
  FILE* imgFile = fopen(imgPath, "rb");
  if (imgFile != NULL) {
    uint32_t count = 0;
    fpos_t start_pos = 0, end_pos = 0;
    while (1) {
      fgetpos(imgFile, &start_pos);
      uint8_t dat[2] = {0};
      if (fread(dat, 1, 2, imgFile) != 2) { break; }
      
      if IS_MARKER(dat) {
        uint16_t marker = FLIP_BYTES(dat);
        char* marker_name = getMarkerString(dat[1]);
        uint8_t* marker_data = NULL;
        uint16_t marker_data_size = 0;
        
        if (start_pos != end_pos) {
          segment_node_t* imgData_node = (segment_node_t*)malloc(sizeof(segment_node_t));
          imgData_node->next_node = NULL;
          imgData_node->marker_code = 0x00;
          imgData_node->data_size = start_pos - end_pos;
          uint8_t* dat_ptr = (uint8_t*)malloc(imgData_node->data_size);
          fseek(imgFile, end_pos, SEEK_SET);
          fread(dat_ptr, 1, imgData_node->data_size, imgFile);
          
          uint32_t j = 0;
          for (uint32_t i = 0; i < imgData_node->data_size; i++) {
            dat_ptr[j] = dat_ptr[i];
            if (dat_ptr[i] == 0xFF) i++;
            j++;
          }
          imgData_node->data_size = j;
          imgData_node->data_ptr = (uint8_t*)malloc(imgData_node->data_size);
          memcpy(imgData_node->data_ptr, dat_ptr, imgData_node->data_size);
          
          node_ptr->next_node = imgData_node;
          node_ptr = node_ptr->next_node;
          fseek(imgFile, 2, SEEK_CUR);
        }
        
        segment_node_t* marker_node = (segment_node_t*)malloc(sizeof(segment_node_t));
        marker_node->next_node = NULL;
        marker_node->marker_code = (uint8_t)(marker & 0x00FF);
        if IS_MARKER_WITH_SIZE(dat) {
          fread(dat, 1, 2, imgFile);
          marker_data_size = FLIP_BYTES(dat) - 2;
          marker_data = (uint8_t*)malloc(marker_data_size);
          fread(marker_data, 1, marker_data_size, imgFile);
        }
        marker_node->data_size = marker_data_size;
        marker_node->data_ptr = marker_data;
        node_ptr->next_node = marker_node;
        node_ptr = node_ptr->next_node;
        
        fgetpos(imgFile, &end_pos);
      }
      else {
        fseek(imgFile, -1, SEEK_CUR);
      }
    }
  }
  
  return dummy_node->next_node;
}

static void parseComment(uint8_t* data_ptr, uint32_t count) {
  uint8_t* comment_string = (uint8_t*)malloc(count+1);
  comment_string[count] = 0x00;
  memcpy(comment_string, data_ptr, count);
  printf("\n\t\t┌");
  for (uint32_t i = 0; i < count; i++) printf("─");
  printf("┐\n\t\t│%s│\n\t\t└",comment_string);
  for (uint32_t i = 0; i < count; i++) printf("─");
  printf("┘");
}

static void parseDQT(uint8_t* data_ptr, uint32_t count) {
  uint8_t Pq = (data_ptr[0] & 0xF0) >> 4;
  uint8_t Tq = data_ptr[0] & 0x0F;
  printf("\n\t\t• Quantization table element precision, Pq = %d", Pq);
  printf("\n\t\t• Quantization table destination identifier, Tq = %d", Tq);
  
  uint8_t zigzagMatrix[64] =
    {  0,  1,  5,  6, 14, 15, 27, 28,
       2,  4,  7, 13, 16, 26, 29, 42,
       3,  8, 12, 17, 25, 30, 41, 43,
       9, 11, 18, 24, 31, 40, 44, 53,
      10, 19, 23, 32, 39, 45, 52, 54,
      20, 22, 33, 38, 46, 51, 55, 60,
      21, 34, 37, 47, 50, 56, 59, 61,
      35, 36, 48, 49, 57, 58, 62, 63  };
  uint8_t mat[64] = {0};
  data_ptr++;
  for (int j = 0; j < 64; j++) { 
    uint8_t x = zigzagMatrix[j];
    mat[j] = data_ptr[x];
  }
  printf("\n\t\t• Quantization table element, Qk\n\t\t\t┌                         ┐");
  for (uint32_t i = 0; i < 64; i++) {
    if (i % 8 == 0) printf("\n\t\t\t│ ");
    printf("%02X ", mat[i]);
    if (i % 8 == 7) printf("│");
  }
  printf("\n\t\t\t└                         ┘");
  
}

