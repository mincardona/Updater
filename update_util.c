#include "update_util.h"

#include "platform.h"

#include <openssl/sha.h>
#include <openssl/rand.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <Windows.h>

// NOTE: SHA*_*() functions return 1 on success, 0 otherwise

// https://stackoverflow.com/questions/918676/generate-sha-hash-in-c-using-openssl-library
// https://www.openssl.org/docs/manmaster/crypto/sha.html

static unsigned long readBytes(FILE* f, unsigned long maxbytes, unsigned char *buf);
static void clearBuf(void *buf, unsigned long size);

/**
 * Computes the SHA256 checksum of a file
 * f = file to checksum
 * digest = pointer to memory to store the digest in (32 bytes)
 * bufsiz = size, in bytes, of chunks in which the file should be read
 *          larger values use more memory, smaller values take more time
 * returns: 0 on success, nonzero on error
 */
int fileSHA256(FILE* f, unsigned char *digest, unsigned long bufsiz) {
    int code = 0;   // return code
    unsigned long bufbytes = sizeof(unsigned char) * bufsiz;
    unsigned char *buf = malloc(bufbytes);
    
    SHA256_CTX context;
    
    bool wasDataRead = false;
    bool updateSuccess = true;
    bool initSuccess = true;
    
    if (!SHA256_Init(&context)) {
        initSuccess = false;    // failed to init SHA256 system
    } else {
        for(;;) {
            unsigned long bytesRead = readBytes(f, bufsiz, buf);
            if (bytesRead > 0) {
                wasDataRead = true;
            } else if (bytesRead == 0) {
                break;
            }
            if (!SHA256_Update(&context, buf, bytesRead)) {
                updateSuccess = false;      // failed to update sum
                break;
            }
        }
    }
    
    if (!initSuccess) {
        code = 1;
    } else if (!wasDataRead) {
        // fill in hash with arbitrary data
        RAND_bytes(digest, DIGEST_LENGTH_SHA256);
    } else if (!updateSuccess) {
        code = 2;
    } else if (!SHA256_Final(digest, &context)) {
        code = 4;
    }
    
    clearBuf(buf, bufbytes);
    free(buf);
    return code;
}

/**
 * Compares two hash message digests for equality.
 * digest1 = pointer to first digest
 * digest2 = pointer to second digest
 * digestLength = length of each digest
 * returns: true if equal, false otherwise
 */
bool isHashEqual(unsigned char const *digest1, unsigned char const *digest2, unsigned long digestLength) {
    for (unsigned long ofs = 0; ofs < digestLength; ofs++) {
        if (digest1[ofs] != digest2[ofs])
            return false;
    }
    return true;
}

/**
 * Reads bytes from a file into a buffer.
 * f = file to read from
 * maxbytes = maximum number of bytes to read
 * buf = pointer to buffer of at least maxbytes size in bytes
 * returns: the number of bytes read
 */
static unsigned long readBytes(FILE* f, unsigned long maxbytes, unsigned char *buf) {
    int c;
    unsigned long ofs;
    // short-circuit order matters here! Don't want to pull an extra char
    for (ofs = 0; ofs < maxbytes && (c = getc(f)) != EOF; ofs++) {
        buf[ofs] = (unsigned char)c;
    }
    return ofs;
}

/**
 * Fills a buffer with '\0'.
 * buf = pointer to memory to fill
 * size = number of bytes to overwrite
 */
static void clearBuf(void *buf, unsigned long size) {
    for (unsigned long i = 0; i < size; i++) {
        ((unsigned char *)(buf))[i] = '\0';
    }
}

bool winReplaceFile(const char *master, const char *replaceme) {
    return CopyFile(master, replaceme, FALSE);  // windows api
}

bool xpReplaceFile(const char *master, const char *replaceme) {
    FILE* fmaster = fopen(master, "rb");
    if (!fmaster) {
        return false;
    }
    FILE* freplaceme = fopen(replaceme, "wb");
    if (!freplaceme) {
        fclose(fmaster);
        return false;
    }
    char c;
    while ((c = getc(fmaster)) != EOF) {
        putc(c, freplaceme);
    }
    fclose(fmaster);
    fclose(freplaceme);
    return true;
}

void printDigest(unsigned char const *digest, unsigned long length) {
    for (int i = 0; i < length; i++)
        printf("%hhx", digest[i]);
}
