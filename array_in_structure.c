//
// Created by michael on 1/30/19.
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
    sb->fs_size = 4;
    sb->first_blk = 1282;
    sb->bitmap = 1;

    fwrite((void *) sb, sizeof(struct Sb), 1, file);

    char bitmap[BLOCK_SIZE];
    memset(bitmap, '0', BLOCK_SIZE);
    fseek(file, BLOCK_SIZE, SEEK_SET);
    fwrite(bitmap, sizeof(bitmap), 1, file);

    struct Root_directory *root_directory = malloc(sizeof(struct Root_directory));
    root_directory->numbers = 1;
    strncpy(root_directory->directories[0].directory_name, "new", 8);
    root_directory->directories[0].nStartBlock = 1283;
    root_directory->nNextBlock = 0;
    fseek(file, sb->first_blk * BLOCK_SIZE, SEEK_SET);
    fwrite((void *) root_directory, sizeof(struct Root_directory), 1, file);
    fclose(file);
    return 1;
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
    fseek(file, sb.first_blk * BLOCK_SIZE, SEEK_SET);

    struct Root_directory root_directory;
    fread((void *) &root_directory, sizeof(struct Root_directory), 1, file);
    for (int i = 0; i < MAX_DIRS_IN_ROOT; i++) {
        if (root_directory.directories[i].nStartBlock != 0) {
            printf("%s\n%li\n", root_directory.directories[i].directory_name,
                   root_directory.directories[i].nStartBlock);
        }
    }
    fclose(file);
    return 1;
}

int main(int argc, char *argv[]) {
    write();
    read();
}