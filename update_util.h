#ifndef UPDATE_UTIL_H
#define UPDATE_UTIL_H

#include <stdio.h>
#include <stdbool.h>

#define DIGEST_LENGTH_SHA256 32

/**
 * Computes the SHA256 checksum of a file
 * f = file to checksum
 * digest = pointer to memory to store the digest in (32 bytes)
 * bufsiz = size, in bytes, of chunks in which the file should be read
 *          larger values use more memory, smaller values take more time
 * returns: 0 on success, nonzero on error
 */
int fileSHA256(FILE* f, unsigned char *digest, unsigned long bufsiz);

/**
 * Compares two hash message digests for equality.
 * digest1 = pointer to first digest
 * digest2 = pointer to second digest
 * digestLength = length of each digest
 * returns: true if equal, false otherwise
 */
bool isHashEqual(unsigned char const *digest1, unsigned char const *digest2, unsigned long digestLength);

/**
 * Overwrites one file with another using Windows API.
 * master = file to use as replacement
 * slave = file to overwrite
 * returns: true on success; false on error
 */
bool winReplaceFile(const char *master, const char *slave);

/**
 * Overwrites one file with another in a cross-platform fashion.
 * master = file to use as replacement
 * slave = file to overwrite
 * returns: true on success; false on error
 */
bool xpReplaceFile(const char *master, const char *replaceme);

/**
 * Prints a message digest as a string of hex digits.
 * digest = pointer to digest contents
 * length = length of the digest (in bytes)
 */
void printDigest(unsigned char const *digest, unsigned long length);

#endif
