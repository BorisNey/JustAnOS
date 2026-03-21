#include "../include/string.h"

//TODO: think about exits/faults

size_t strlen(const char* string){
	size_t len = 0;
	while(string[len]) {
		len++;
	}
	
	return len;
}

int strcpy(const char* src, char* dest){
	int i = 0;
	do {
		dest[i] = src[i];
	} while(src[i]);
	
	return 0;
}

int memcpy(void* dest, void* src, size_t len){
	unsigned char* dest_cast = (unsigned char*) dest;
	unsigned char* src_cast = (unsigned char*) src;
	for(size_t i = 0; i < len; i++){
		dest_cast[i] = src_cast[i];
	}
	
	return 0;
}

int memset(void* dest, unsigned char value, size_t len){
	unsigned char* dest_cast = (unsigned char*) dest;
	for(size_t i = 0; i < len; i++){
		dest_cast[i] = value;
	}

	return 0;
}

