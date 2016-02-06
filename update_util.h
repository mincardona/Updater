#ifndef UPDATE_UTIL_H
#define UPDATE_UTIL_H

#include <stdio.h>
#include <stdbool.h>

int fileSHA256(FILE* f, unsigned char *digest, unsigned long bufsiz);
bool isHashEqual(unsigned char *digest1, unsigned char *digest2, unsigned long digestLength);
bool replaceFile(const char *master, const char *replaceme);
bool cpReplaceFile(const char *master, const char *replaceme);

#endif
