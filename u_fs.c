/** @file
 * Compile with:
 *
 *     gcc -Wall u_fs.c `pkg-config fuse3 --cflags --libs` -o u_fs
 */

#define FUSE_USE_VERSION 31

#include "fuse.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <assert.h>
#include <stdlib.h>
#include "u_fs_structs.h"
#include "disk_operation.h"

static int u_fs_getattr(const char *path, struct stat *stbuf,
                        struct fuse_file_info *fi) {
    (void) fi;

    memset(stbuf, 0, sizeof(struct stat));
    char directoryname[MAX_FILENAME + 1];
    char filename[MAX_FILENAME + 1];
    char extension[MAX_EXTENSION + 1];
    memset(directoryname, 0, MAX_FILENAME + 1);
    memset(filename, 0, MAX_FILENAME + 1);
    memset(extension, 0, MAX_EXTENSION + 1);

    if (strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
        return 0;
    } else {
        sscanf(path, "/%[^/]/%[^.].%s", directoryname, filename, extension);
        long location_directory = u_fs_find_directory(directoryname);
        if (location_directory == 0) {
            return -ENOENT;
        }
        if (filename[0] == '\0') { //no file name, and dir exists
            stbuf->st_mode = S_IFDIR | 755;
            stbuf->st_nlink = 2;
            return 0;
        } else {
            size_t fsize = 0;
            long location_file = u_fs_find_file(location_directory, directoryname);
            if (location_file == 0) {
                return -ENOENT;
            }
            stbuf->st_mode = S_IFREG | 0666;
            stbuf->st_nlink = 1;
            stbuf->st_size = fsize;
            return 0;
        }
    }
}

static int u_fs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                        off_t offset, struct fuse_file_info *fi,
                        enum fuse_readdir_flags flags) {
    (void) offset;
    (void) fi;
    (void) flags;

    char directoryname[MAX_FILENAME + 1];
    char filename[MAX_FILENAME + 1];
    char extension[MAX_EXTENSION + 1];
    memset(directoryname, 0, MAX_FILENAME + 1);
    memset(filename, 0, MAX_FILENAME + 1);
    memset(extension, 0, MAX_EXTENSION + 1);

    sscanf(path, "/%[^/]/%[^.].%s", directoryname, filename, extension);

    if (strcmp(path, "/") == 0) {

        filler(buf, ".", NULL, 0, 0);
        filler(buf, "..", NULL, 0, 0);

        struct Sb sb;

        get_sb(0, &sb);
        struct Root_directory root_directory;
        if (!get_root_directory(sb.first_blk, &root_directory)){
            return -ENOENT;
        }
        for (int i = 0; i < MAX_DIRS_IN_ROOT; i++) {
            if (strcmp(root_directory.directories[i].directory_name, "") != 0)
                filler(buf, root_directory.directories[i].directory_name, NULL, 0, 0);
        }
        while (root_directory.nNextBlock != 0) {
            get_root_directory(root_directory.nNextBlock, &root_directory);
            for (int i = 0; i < MAX_DIRS_IN_ROOT; i++) {
                if (strcmp(root_directory.directories[i].directory_name, "") != 0)
                    filler(buf, root_directory.directories[i].directory_name, NULL, 0, 0);
            }
        }
        return 0;
    }

    long location_directory = u_fs_find_directory(directoryname);
    if (location_directory == 0) {
        return -ENOENT;
    }

    struct u_fs_File_directory u_fs_file_directory;
    get_u_fs_file_directory(location_directory, &u_fs_file_directory);
    struct Directory_entry directory_entry;
    get_directory_entry(u_fs_file_directory.nStartBlock, &directory_entry);

    for (int i = 0; i < MAX_FILES_IN_DIRECTORY; ++i) {
        if (strcmp(directory_entry.u_fs_file_directory_list[i].fname, "") == 0) {
            filler(buf, directory_entry.u_fs_file_directory_list[i].fname, NULL, 0, 0);
        }
    }

    while (directory_entry.nNextBlock != 0) {
        get_directory_entry(directory_entry.nNextBlock, &directory_entry);
        for (int i = 0; i < MAX_FILES_IN_DIRECTORY; ++i) {
            if (strcmp(directory_entry.u_fs_file_directory_list[i].fname, "") == 0) {
                filler(buf, directory_entry.u_fs_file_directory_list[i].fname, NULL, 0, 0);
            }
        }
    }
    return 0;

}

static int u_fs_read(const char *path, char *buf, size_t size, off_t offset,
                     struct fuse_file_info *fi) {
    (void) fi;

    char directoryname[MAX_FILENAME + 1];
    char filename[MAX_FILENAME + 1];
    char extension[MAX_EXTENSION + 1];
    memset(directoryname, 0, MAX_FILENAME + 1);
    memset(filename, 0, MAX_FILENAME + 1);
    memset(extension, 0, MAX_EXTENSION + 1);

    sscanf(path, "/%[^/]/%[^.].%s", directoryname, filename, extension);

    long location_directory = u_fs_find_directory(directoryname);
    if (location_directory == 0) {
        return -ENOENT;
    }

    long location_file = u_fs_find_file(location_directory, directoryname);
    if (location_file == 0) {
        return -ENOENT;
    }

    struct u_fs_File_directory u_fs_file_directory;
    get_u_fs_file_directory(location_file, &u_fs_file_directory);

    if (offset > u_fs_file_directory.fsize) {
        return -EFBIG;
    }

    struct u_fs_Disk_block u_fs_disk_block;
    get_u_fs_disk_block(u_fs_file_directory.nStartBlock, &u_fs_disk_block);

    int offset_remainder = (int) offset;
    while (offset_remainder >= MAX_DATA_IN_BLOCK) {
        offset_remainder -= MAX_DATA_IN_BLOCK;
        if (get_u_fs_disk_block(u_fs_disk_block.nNextBlock, &u_fs_disk_block)) {
            continue;
        } else {
            printf("The offset is out of range.\n");
            return -1;
        }
    }

    int i;
    for (i = 0; i < size; i++) {
        if ((i + offset_remainder) % MAX_DATA_IN_BLOCK == 0 && i != 0) {
            get_u_fs_disk_block(u_fs_disk_block.nNextBlock, &u_fs_disk_block);
        }
        if (i + offset > u_fs_file_directory.fsize) {
            break;
        }
        buf[i] = u_fs_disk_block.data[(i + offset_remainder) % MAX_DATA_IN_BLOCK];
    }
    return i;
}

static struct fuse_operations u_fs_operations = {
        .getattr=u_fs_getattr,
        .readdir=u_fs_readdir,
        .read=u_fs_read,
};

int main(int argc, char *argv[]) {
    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
}