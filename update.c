#include <stdio.h>
#include "update_util.h"

int main() {
    unsigned char digest[32]; 
    FILE* sample = fopen("howto.txt", "rb");
    if (!sample) {
        fprintf(stderr, "File not found!");
        getchar();
        return 1;
    }
    
    int result = fileSHA256(sample, digest, 5000);
    
    if (result != 0) {
        fprintf(stderr, "Error computing checksum (code %d)", result);
        getchar();
        return 2;
    } else {
        for (int i = 0; i < 32; i++)
            printf("%hhx", digest[i]);
    }
    
    getchar();
    return 0;
}
