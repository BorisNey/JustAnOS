#pragma once

#include <stddef.h>
#include <stdint.h>

size_t strlen(const char* data);
int strcpy(const char* src, char* dest);
int memcpy(void* dest, void* src, size_t len);
int memset(void* dest, unsigned char value, size_t len);
int int_abs(int number);
void itoa(int number, char* string);
void htoa(uint32_t value, char* string);
