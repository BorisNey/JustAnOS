#include "../include/stdio.h"

void memcpy(uint16_t* dest, uint16_t* src, size_t len){
  for(size_t i = 0; i < len; i++){
    dest[i] = src[i];
  }
}
