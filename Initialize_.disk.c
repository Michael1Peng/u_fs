//
// Created by Michael on 2019-02-06.
//

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "u_fs_structs.h"

void Init() {

//    初始化Super block 和root directory
    FILE *file = fopen("/data/.disk", "w");
    if (file == NULL) {
        printf("fail to read the file.\n");
        return;
    }

    struct Sb *sb = malloc(sizeof(struct Sb));
    sb->first_blk = 1;
    fwrite((void *) sb, sizeof(struct Sb), 1, file);

    struct Root_directory *root_directory = malloc(sizeof(struct Root_directory));
    fseek(file, sb->first_blk * BLOCK_SIZE, SEEK_SET);
    fwrite((void *) root_directory, sizeof(struct Root_directory), 1, file);
    fclose(file);

//    初始化bitmap
    file = fopen("/data/diskimg", "w");
    if (file == NULL) {
        printf("fail to read the file.\n");
        return;
    }
    int bitmap_length = 5000000;
    fwrite(&bitmap_length, sizeof(int), 1, file);
    fseek(file, sizeof(int), SEEK_SET);
    char bitmap[bitmap_length];
    memset(bitmap, '0', bitmap_length);
    bitmap[0] = '1';
    bitmap[1] = '1';
    fwrite(bitmap, (size_t) bitmap_length, 1, file);
    fclose(file);

    file = fopen("/data/diskimg", "rb+");
    if (file == NULL) {
        printf("fail to read the file.\n");
        return;
    }
}

int main(int argc, char *argv[]) {
    Init();
}