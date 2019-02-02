//
// Created by Michael on 2019-02-02.
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

    struct Sb *sb = malloc(sizeof(struct Sb));
    sb->fs_size = 4;
    sb->first_blk = 1282;
    sb->bitmap = 1;

    write_sb(0, sb);

    char bitmap[BLOCK_SIZE];
    memset(bitmap, '0', BLOCK_SIZE);
    write_bitmap(1, bitmap);

    struct Root_directory *root_directory = malloc(sizeof(struct Root_directory));
    root_directory->numbers = 1;
    strncpy(root_directory->directories[0].directory_name, "new", 8);
    root_directory->directories[0].nStartBlock = 1283;
    root_directory->nNextBlock = 0;
    write_root_directory(sb->first_blk, root_directory);

    struct u_fs_File_directory *u_fs_file_directory = malloc(sizeof(struct u_fs_File_directory));
    strncpy(u_fs_file_directory->fname, "new", 8);
    u_fs_file_directory->nStartBlock = 1284;
    u_fs_file_directory->flag = 2;
    write_u_fs_file_directory(root_directory->directories[0].nStartBlock, u_fs_file_directory);

    struct Directory_entry *directory_entry = malloc(sizeof(struct Directory_entry));
    directory_entry->numbers = 1;
    strncpy(directory_entry->u_fs_file_directory_list[0].fname, "new", 8);
    directory_entry->u_fs_file_directory_list[0].nStartBlock = 1285;
    directory_entry->u_fs_file_directory_list[0].flag = 1;
    write_directory_entry(u_fs_file_directory->nStartBlock, directory_entry);

    u_fs_file_directory = malloc(sizeof(struct u_fs_File_directory));
    strncpy(u_fs_file_directory->fname, "new", 8);
    u_fs_file_directory->nStartBlock = 1286;
    u_fs_file_directory->flag = 1;
    write_u_fs_file_directory(directory_entry->u_fs_file_directory_list[0].nStartBlock, u_fs_file_directory);

    struct u_fs_Disk_block *u_fs_disk_block = malloc(sizeof(struct u_fs_Disk_block));
    write_u_fs_disk_block(u_fs_file_directory->nStartBlock, u_fs_disk_block);

    return 1;
}

void read() {
    struct Sb *sb = malloc(sizeof(struct Sb));
    if (get_sb(0, sb)) {
        printf("%li\n", sb->first_blk);
    };

    struct Root_directory *root_directory = malloc(sizeof(struct Root_directory));
    if (get_root_directory(sb->first_blk, root_directory)) {
        printf("%li\n", root_directory->directories[0].nStartBlock);
    }

    struct u_fs_File_directory *u_fs_file_directory = malloc(sizeof(struct u_fs_File_directory));
    if (get_u_fs_file_directory(root_directory->directories[0].nStartBlock, u_fs_file_directory)) {
        printf("%li\n", u_fs_file_directory->nStartBlock);
    }

    struct Directory_entry *directory_entry = malloc(sizeof(struct Directory_entry));
    if (get_directory_entry(u_fs_file_directory->nStartBlock, directory_entry)) {
        printf("%li\n", directory_entry->u_fs_file_directory_list[0].nStartBlock);
    }

    u_fs_file_directory = malloc(sizeof(struct u_fs_File_directory));
    if (get_u_fs_file_directory(directory_entry->u_fs_file_directory_list[0].nStartBlock, u_fs_file_directory)) {
        printf("%li\n", u_fs_file_directory->nStartBlock);
    }

    struct u_fs_Disk_block *u_fs_disk_block = malloc(sizeof(struct u_fs_Disk_block));
    if (get_u_fs_disk_block(u_fs_file_directory->nStartBlock, u_fs_disk_block)) {
        printf("%s\n", u_fs_disk_block->data);
    }
}

int main(int argc, char *argv[]) {
    write();
    read();
}
