#include "../include/string.h"

/*
IMPROVEMENTS:
	- think about exits/faults
*/
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

int int_abs(int number){
	#define INT_MAX 2147483647
	int result = number;

	if (number < 0){
		if (number < -INT_MAX){	// Overflow handling when: number = INT_MIN = -2147483648
			result = INT_MAX;
		}
		else{
			result = number * (-1);
		}
	}

	return result;
}

void itoa(int number, char* string){
	int i = 0;

	// check if negative
	if (number < 0){
		string[i] = '-';
		i++;
		number = int_abs(number);
	}

	char temp_string[12];
	int last_non_zero = 0;
	int k = 0;
	for (int dec = 1; dec <= 1000000000; dec *= 10){
		temp_string[k] = (char)((number / dec) % 10 + '0');
		if (temp_string[k] != '0'){
			last_non_zero = k;
		}
		k++;
	}

	for (k = last_non_zero; k >= 0; k--){
		string[i] = temp_string[k];
		i++;
	}

	string[i] = '\0';
	return;
}
