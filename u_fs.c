#define FUSE_USE_VERSION 31

#include "fuse.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <assert.h>

static int u_fs_getattr(const char *path, struct stat *st,
                        struct fuse_file_info *fi) {
    st->st_uid = getuid();
    st->st_gid = getgid();
    st->st_atime = time(NULL);
    st->st_mtime = time(NULL);
    if (strcmp(path, "/") == 0) {
        st->st_mode = S_IFDIR | 0755;
        st->st_nlink = 2;
        return 0;
    } else if (strcmp(path + 1, "file54") == 0
               || strcmp(path + 1, "file349") == 0) {
        st->st_mode = S_IFREG | 0666
                ;
        st->st_nlink = 1;
        st->st_size = 1024;
        return 0;
    } else return -ENOENT;

}

static int u_fs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                        off_t offset, struct fuse_file_info *fi,
                        enum fuse_readdir_flags flags) {
    filler(buf, ".", NULL, 0, 0);
    filler(buf, "..", NULL, 0, 0);
    if (strcmp(path, "/") == 0) {
        filler(buf, "file54", NULL, 0, 0);
        filler(buf, "file349", NULL, 0, 0);
    }

    return 0;

}

static int u_fs_mkdir(const char *path, mode_t mode) {
//    printf("It's mkdir, %s\n", path);
//    return 0;
    int res;
    res = mkdir(path, mode);
    if (res == -1)
        return -errno;
    return 0;
}

static int u_fs_read(const char *path, char *buf, size_t size, off_t offset,
                     struct fuse_file_info *fi) {
    char file54Text[] = "Hello World From File54!";
    char file349Text[] = "Hello World From File349!";
    char *selectedText = NULL;
    if (strcmp(path, "/file54") == 0)
        selectedText = file54Text;
    else if (strcmp(path, "/file349") == 0)
        selectedText = file349Text;
    else
        return -1;
    memcpy(buf, selectedText + offset, size);

    return strlen(selectedText) - offset;
}

static struct fuse_operations u_fs_operations = {
        .getattr    = u_fs_getattr,
        .readdir    = u_fs_readdir,
        //.mkdir      = u_fs_mkdir,
        .read    = u_fs_read,
};

int main(int argc, char *argv[]) {
    return fuse_main(argc, argv, &u_fs_operations, NULL);
}