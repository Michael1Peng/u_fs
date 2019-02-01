//
// Created by michael on 1/31/19.
//

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <assert.h>
#include <stdlib.h>
#include "u_fs_structs.h"
#include "disk_operation.h"

int write() {
    FILE *file = fopen(".disk", "rb+");
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
    fwrite(bitmap, BLOCK_SIZE, 1, file);

    struct Root_directory *root_directory = malloc(sizeof(struct Root_directory));
    root_directory->numbers = 1;
    strncpy(root_directory->directories[0].directory_name, "new", 8);
    root_directory->directories[0].nStartBlock = 1283;
    root_directory->nNextBlock = 0;
    printf("%li\n", sb->first_blk);
    fseek(file, sb->first_blk * BLOCK_SIZE, SEEK_SET);
    fwrite((void *) root_directory, sizeof(struct Root_directory), 1, file);

    struct u_fs_File_directory *u_fs_file_directory = malloc(sizeof(struct u_fs_File_directory));
    strncpy(u_fs_file_directory->fname, "new", 8);
    u_fs_file_directory->nStartBlock = 1284;
    u_fs_file_directory->flag = 2;
    fseek(file, root_directory->directories[0].nStartBlock * BLOCK_SIZE, SEEK_SET);
    fwrite((void *) u_fs_file_directory, sizeof(struct u_fs_File_directory), 1, file);

    struct Directory_entry *directory_entry = malloc(sizeof(struct Directory_entry));
    directory_entry->numbers = 1;
    strncpy(directory_entry->u_fs_file_directory_list[0].fname, "new", 8);
    directory_entry->u_fs_file_directory_list[0].nStartBlock = 1285;
    directory_entry->u_fs_file_directory_list[0].flag = 1;
    fseek(file, u_fs_file_directory->nStartBlock * BLOCK_SIZE, SEEK_SET);
    fwrite((void *) directory_entry, sizeof(struct Directory_entry), 1, file);

    u_fs_file_directory = malloc(sizeof(struct u_fs_File_directory));
    strncpy(u_fs_file_directory->fname, "new", 8);
    u_fs_file_directory->nStartBlock = 1286;
    u_fs_file_directory->flag = 1;
    fseek(file, directory_entry->u_fs_file_directory_list[0].nStartBlock * BLOCK_SIZE, SEEK_SET);
    fwrite((void *) u_fs_file_directory, sizeof(struct u_fs_File_directory), 1, file);

    struct u_fs_Disk_block *u_fs_disk_block = malloc(sizeof(struct u_fs_Disk_block));
    fseek(file, u_fs_file_directory->nStartBlock * BLOCK_SIZE, SEEK_SET);
    fwrite((void *) u_fs_disk_block, sizeof(struct u_fs_Disk_block), 1, file);

    fclose(file);
    return 1;
}

int read() {
    long location;
    location = u_fs_find_directory("new");
    printf("%li\n", location);
    location = u_fs_find_file(location, "new");
    printf("%li", location);
    return 1;
}

int main(int argc, char *argv[]) {
    write();
    read();
}

