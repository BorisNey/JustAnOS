#include "../include/string.h"

size_t strlen(const char* string){
	size_t len = 0;
	while(string[len])
		len++;
	return len;
}

int strcpy(const char* src, char* dest, size_t lenght){
  for (size_t i = 0; i < lenght; i++){
    dest[i] = src[i];
  }
  return 0;
}
