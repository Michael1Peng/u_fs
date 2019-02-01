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

static struct fuse_operations u_fs_operations = {
        .getattr=u_fs_getattr,
};

int main(int argc, char *argv[]) {
    return fuse_main(argc, argv, &u_fs_operations, NULL);
}