#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include "update_util.h"
#include "../cutil/cutil.h"

char *line;
char *srcworkspace;     // path to each source file
char *destworkspace;    // path to each destination file
char *dest; // destination directory
char *src;  // source directory
char *flistpath;    // path to file listing

typedef struct {
    int updated;    // files successfully updated
    int failed;     // failed file updates
    int ok;         // files already up-to-date
} UpdateResult;

UpdateResult update(int argc, char** argv);

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, "%dUsage: update dest_dir source_dir file_listing\n", argc);
        return 1;
    }
    
    // allocate memory
    line = malloc(PATH_MAX);
    srcworkspace = malloc(PATH_MAX * 2);
    destworkspace = malloc(PATH_MAX * 2);
    dest = malloc(PATH_MAX);
    src = malloc(PATH_MAX);
    flistpath = malloc(PATH_MAX);
    
    // perform the update and get the return code
    UpdateResult ret = update(argc, argv);
    
    // free memory
    free(line);
    free(src);
    free(dest);
    free(srcworkspace);
    free(destworkspace);
    free(flistpath);
    
    printf("Update finished with:\n"
           "%d incomplete transactions\n"
           "%d files updated\n"
           "%d files up-to-date\n", 
           ret.failed, ret.updated, ret.ok);
    
    return ret.failed;
}

UpdateResult update(int argc, char** argv) {
    UpdateResult result = {0, 0, 0};
    
    // source and destination digests
    unsigned char digestsrc[DIGEST_LENGTH_SHA256];
    unsigned char digestdest[DIGEST_LENGTH_SHA256];
    
    // copy source, destination, and file listing paths from args to buffers
    strncpy(dest, argv[1], PATH_MAX);
    strncpy(src, argv[2], PATH_MAX);
    strncpy(flistpath, argv[3], PATH_MAX);
    
    // lengths of source and destination paths
    int srclen = strlen(src);
    int destlen = strlen(dest);
    
    // copy the source and destination paths to the workspace buffers
    strncpy(srcworkspace, src, srclen);
    strncpy(destworkspace, dest, destlen);
    
    // try to open the file listing
    FILE* flist = fopen(flistpath, "r");
    if (!flist) {
        fprintf(stderr, "File listing not found\n");
        result.failed = 1;
        return result;
    }
    
    // points to the sub-path of the current file, pulled from the listing, 
    // as part of the source workspace
    char *fpath = srcworkspace + srclen;
    
    // get lines from the listing and append them to the source workspace
    while (getln_s(flist, fpath, PATH_MAX) != NULL) {
        // append the listing path to the destination path in the workspace
        strncpy(destworkspace + destlen, fpath, PATH_MAX);
        
        // try to open the source file
        // on failure, move to next file
        FILE* srcfile = fopen(srcworkspace, "rb");
        if (!srcfile) {
            fprintf(stderr, "File not found: %s (%s)\n", fpath, srcworkspace);
            result.failed++;
            continue;
        }
        
        // try to open the destination file, to verify that it exists
        // and to compute its checksum
        FILE* destfile = fopen(destworkspace, "rb");
        if (!destfile) {
            fprintf(stderr, "Error opening destination file: %s\n", fpath);
            fclose(srcfile);
            result.failed++;
            continue;
        }
        
        // try to checksum the source file
        // on failure, move to next file
        int check = fileSHA256(srcfile, digestsrc, DIGEST_LENGTH_SHA256);
        if (check != 0) {
            fprintf(stderr, "File source checksum failed (%d): %s\n", check, fpath);
            fclose(srcfile);
            fclose(destfile);
            result.failed++;
            continue;
        }
        
        // try to checksum the destination file
        // on failure, move to next file
        check = fileSHA256(destfile, digestdest, DIGEST_LENGTH_SHA256);
        if (check != 0) {
            fprintf(stderr, "File destination checksum failed (%d): %s\n", check, fpath);
            fclose(srcfile);
            fclose(destfile);
            result.failed++;
            continue;
        }
        
        // close the destination file because it was opened for reading only
        fclose(destfile);
        
        // if the hashes do not match, then replace the destination file with the source file
        if (!isHashEqual(digestsrc, digestdest, DIGEST_LENGTH_SHA256)) {
            // open destination file for writing
            // on failure, move to the next file
            destfile = fopen(destworkspace, "wb");
            if (!destfile) {
                fprintf(stderr, "Error opening destination file: %s\n", fpath);
                fclose(srcfile);
                result.failed++;
                continue;
            }
            // replace the file
            if (xpReplaceFile(srcworkspace, destworkspace)) {
                result.updated++;
            }
        } else {
            result.ok++;
        }
        
        // close each file
        fclose(srcfile);
        fclose(destfile);
    }
    
    // close the file listing
    fclose(flist);
    
    return result;
}
