#ifndef UPDATE_UTIL_H
#define UPDATE_UTIL_H

#include <stdio.h>
#include <stdbool.h>

#define DIGEST_LENGTH_SHA256 32

int fileSHA256(FILE* f, unsigned char *digest, unsigned long bufsiz);
bool isHashEqual(unsigned char const *digest1, unsigned char const *digest2, unsigned long digestLength);
bool winReplaceFile(const char *master, const char *replaceme);
bool xpReplaceFile(const char *master, const char *replaceme);
void printDigest(unsigned char const *digest, unsigned long length);

#endif
