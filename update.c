#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include "update_util.h"
#include "../cutil/cutil.h"

char *line;
char *srcworkspace;
char *destworkspace;

char *dest;
char *src;
char *flistpath;

int update(int argc, char** argv);

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, "%d args provided.\nUsage: update dest_dir source_dir file_listing\n", argc);
        return 1;
    }
    
    line = malloc(PATH_MAX);
    srcworkspace = malloc(PATH_MAX * 2);
    destworkspace = malloc(PATH_MAX * 2);
    dest = malloc(PATH_MAX);
    src = malloc(PATH_MAX);
    flistpath = malloc(PATH_MAX);
    
    int ret = update(argc, argv);
    
    free(line);
    free(src);
    free(dest);
    free(srcworkspace);
    free(destworkspace);
    free(flistpath);
    
    return ret;
}

int update(int argc, char** argv) {
    unsigned char digestsrc[DIGEST_LENGTH_SHA256];
    unsigned char digestdest[DIGEST_LENGTH_SHA256];
    
    strncpy(dest, argv[1], PATH_MAX);
    strncpy(src, argv[2], PATH_MAX);
    strncpy(flistpath, argv[3], PATH_MAX);
    
    int srclen = strlen(src);
    int destlen = strlen(dest);
    
    strncpy(srcworkspace, src, srclen);
    strncpy(destworkspace, dest, destlen);
    
    FILE* flist = fopen(flistpath, "r");
    if (!flist) {
        fprintf(stderr, "File listing not found\n");
        return 1;
    }
    
    char *fpath = srcworkspace + srclen;
    
    while (getln_s(flist, fpath, PATH_MAX) != NULL) {
        strncpy(destworkspace + destlen, fpath, PATH_MAX);
        
        FILE* srcfile = fopen(srcworkspace, "rb");
        if (!srcfile) {
            fprintf(stderr, "File not found: %s\n", fpath);
            continue;
        }
        
        FILE* destfile = fopen(destworkspace, "rb");
        if (!destfile) {
            fprintf(stderr, "Error opening destination file: %s\n", fpath);
            fclose(srcfile);
            continue;
        }
        
        int check = fileSHA256(srcfile, digestsrc, DIGEST_LENGTH_SHA256);
        if (check != 0) {
            fprintf(stderr, "File source checksum failed (%d): %s\n", check, fpath);
            fclose(srcfile);
            fclose(destfile);
            continue;
        }
        
        check = fileSHA256(destfile, digestdest, DIGEST_LENGTH_SHA256);
        if (check != 0) {
            fprintf(stderr, "File destination checksum failed (%d): %s\n", check, fpath);
            fclose(srcfile);
            fclose(destfile);
            continue;
        }
        
        fclose(destfile);
        
        if (!isHashEqual(digestsrc, digestdest, DIGEST_LENGTH_SHA256)) {
            destfile = fopen(destworkspace, "wb");
            if (!destfile) {
                fprintf(stderr, "Error opening destination file: %s\n", fpath);
                fclose(srcfile);
                continue;
            }
            xpReplaceFile(srcworkspace, destworkspace);
        }
        
        fclose(srcfile);
        fclose(destfile);
    }
    
    fclose(flist);
    return 0;
}
