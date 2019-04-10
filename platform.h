#ifndef PLATFORM_H
#define PLATFORM_H

#include <dirent.h>
#include <fcntl.h>

/*!
 * Platform definitions
 */

const char PATH_SEPARATOR = '/';

struct linux_dirent64
{
    ino64_t        d_ino;     /* 64-bit inode number */
    off64_t        d_off;     /* 64-bit offset to next structure */
    unsigned short d_reclen;  /* Size of this dirent */
    unsigned char  d_type;    /* File type */
    char           d_name[1]; /* Filename (flexible, null-terminated) */
};

#endif // PLATFORM_H
