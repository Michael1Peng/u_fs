//
// Created by michael on 1/29/19.
//

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <assert.h>
#include <stdlib.h>
#include "u_fs_structs.h"

int write() {
    FILE *file = fopen("file", "rb+");
    if (file == NULL) {
        printf("fail to read the file.\n");
        return -1;
    }

    struct Sb *sb = malloc(sizeof(struct Sb));
    sb->fs_size = 3;
    sb->first_blk = 1282;
    sb->bitmap = 1;

    fwrite((void *) sb, sizeof(struct Sb), 1, file);

    char bitmap[BLOCK_SIZE];
    memset(bitmap, '0', BLOCK_SIZE);
    fseek(file, sizeof(struct Sb), SEEK_SET);
    fwrite(bitmap, sizeof(bitmap), 1, file);

    fclose(file);
}

int read() {
    FILE *file = fopen("file", "rb+");
    if (file == NULL) {
        printf("fail to read the file.\n");
        return -1;
    }

    struct Sb sb;
    fread((void *) &sb, sizeof(struct Sb), 1, file);
    printf("%d %d %d\n", (int) sb.fs_size, (int) sb.first_blk, (int) sb.bitmap);

    char bitmap[BLOCK_SIZE];
    fseek(file, sizeof(struct Sb), SEEK_SET);
    fread(bitmap, BLOCK_SIZE, 1, file);
    printf("%s", bitmap);
    fclose(file);
}

int main(int argc, char *argv[]) {
    write();
    read();
}