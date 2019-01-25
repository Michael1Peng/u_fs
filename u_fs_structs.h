#ifndef u_fs_structs_H_
#define u_fs_structs_H_

#define	BLOCK_SIZE 512
#define	MAX_FILENAME 8
#define	MAX_EXTENSION 3
#define MAX_FILES_IN_DIR (BLOCK_SIZE - sizeof(int)) / ((MAX_FILENAME + 1) + (MAX_EXTENSION + 1) + sizeof(size_t) + sizeof(long))

struct sb {
    long fs_size; //size of file system, in blocks
    long first_blk; //first block of root directory
    long bitmap; //size of bitmap, in blocks
};

struct u_fs_file_directory {
    char fname[MAX_FILENAME + 1]; //filename (plus space for nul)
    char fext[MAX_EXTENSION + 1]; //extension (plus space for nul)
    size_t fsize; //file size
    long nStartBlock; //where the first block is on disk
    int flag; //indicate type of file. 0:for unused; 1:for file; 2:for directory
}

struct u_fs_disk_block {
    size_t size; // how many bytes are being used in this block
    long nNextBlock; //The next disk block, if needed. This is the next pointer in the linked allocation list
    char data[MAX_DATA_IN_BLOCK];// And all the rest of the space in the block can be used for actual data storage.
};

#endif