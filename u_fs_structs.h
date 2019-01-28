#ifndef u_fs_structs_H_
#define u_fs_structs_H_

#include "fuse.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <assert.h>
#include "disk_operation.h"

#define    BLOCK_SIZE 512
#define    MAX_FILENAME 8
#define    MAX_EXTENSION 3
#define MAX_DATA_IN_BLOCK BLOCK_SIZE - sizeof(size_t)-sizeof(long)
#define Bitmap_START 1
#define Bitmap_END 1281

struct Sb {
    long fs_size; //size of file system, in blocks
    long first_blk; //first block of root directory
    long bitmap; //size of bitmap, in blocks
};

struct u_fs_File_directory {
    char fname[MAX_FILENAME + 1]; //filename (plus space for nul)
    char fext[MAX_EXTENSION + 1]; //extension (plus space for nul)
    size_t fsize; //file size
    long nStartBlock; //where the first block is on disk
    int flag; //indicate type of file. 0:for unused; 1:for file; 2:for directory
};

struct Root_directory_data {
    static int number_directories;
    char directory_name[MAX_FILENAME];
    int block_position;
};
struct Root_directory_data *Root_directory_data_new(char data[16]) {
    struct Root_directory_data *root_directory_data = malloc(sizeof(struct Root_directory_data));
    strncpy(root_directory_data->directory_name, data, 8);
    root_directory_data->block_position = atoi(data + 8);
    return root_directory_data;
}

void store_information(struct Root_directory_data *root_directory_data) {
    char data[16]="";
    int length = (int) strlen(root_directory_data->directory_name);
    memset(data + length, ' ', 8);
    strncpy(data, root_directory_data->directory_name, 8);

    char temp_block_position[8];
    sprintf(temp_block_position, "%d", root_directory_data->block_position);
    length = (int) strlen(temp_block_position);
    char string_block_position[8];
    memset(string_block_position, '0', 8);
    strncpy(string_block_position+8-length,temp_block_position,length);
    strncpy(data+8, string_block_position, 8);
}

struct u_fs_Disk_block {
    size_t size; // how many bytes are being used in this block
    long nNextBlock; //The next disk block, if needed. This is the next pointer in the linked allocation list
    char data[MAX_DATA_IN_BLOCK];// And all the rest of the space in the block can be used for actual data storage.
};

#endif