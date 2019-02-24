//
// Created by michael on 1/27/19.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define    MAX_FILENAME 8

struct Root_directory_data {
    //static int number_directories;
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
    printf("%s", data);
}

int main(int argc, char *argv[]) {
    char data[16] = "dat     00000010";
    struct Root_directory_data *root_directory_data = Root_directory_data_new(data);
    printf("%s %d\n", root_directory_data->directory_name, root_directory_data->block_position);
    store_information(root_directory_data);
}