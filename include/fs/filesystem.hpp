#ifndef _HPP_FS_FILESYSTEM_
#define _HPP_FS_FILESYSTEM_

#include "main/common.h"
#include "structures/utf8.hpp"
#include "mem/object.hpp"
#include "main/errno.h"

namespace filesystem {
    class Filesystem;

    enum UnderlyingType {
        UT_UNDER_FS,
        UT_GENERATED,
        UT_STORAGE,
        UT_FILE,
        UT_MEM
    };

    enum InodeType {
        TYPE_DIRECTORY,
        TYPE_FILE
    };

    class InodeEntry {
    public:
        InodeEntry(uint64_t inode, Utf8 name);

        uint64_t inode;
        Utf8 name;
        // Probably other stuff
        InodeEntry *next;
    };

    class Inode {
    public:
        Inode(Filesystem *fs, uint64_t number, InodeType type, uint64_t size);

        void adjust_ref(uint32_t delta);

        Filesystem *fs;
        uint64_t inode_no;
        void *fs_data;
        InodeType type;
        object::Object *contents;
        uint64_t size;
        // Probably other stuff
        Inode *next;
        InodeEntry *children; // for directories

    private:
        uint32_t ref = 0;
    };

    // Used to identify files uniquely
    class InodeId {
    public:
        uint32_t filesystem; // id of filesystem
        uint64_t inode;
    };

    // Base class, individual filesystems (ext4, FAT, etc) inherit this
    class FilePathEntry;
    class Filesystem {
    public:
        uint32_t id; // random id
        UnderlyingType underlying_type; // UNDER_FS, GENERATED, etc.
        // Again, maybe other stuff
        // And details about the underlying filesystem

        virtual error_t read_inode(FilePathEntry *path, uint32_t inode_no, Inode **inode);
        virtual error_t root_inode(Inode **inode);
    };


    class FilePathEntry {
    public:
        Utf8 name;
        FilePathEntry *parent;
        Inode *inode;

        FilePathEntry(Utf8 name, FilePathEntry *parent);
        FilePathEntry(Utf8 name, FilePathEntry *parent, Inode *inode);
        ~FilePathEntry();
        error_t populate(FilePathEntry **tip);
        FilePathEntry *copy();
        void adjust_ref(uint32_t delta);

    private:
        uint32_t ref;
    };

    FilePathEntry *parse_path(Utf8 path, FilePathEntry *parent);
    void free_path(FilePathEntry *path);

    extern Filesystem *rootfs;
}

#endif
