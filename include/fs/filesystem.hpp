#ifndef _HPP_FS_FILESYSTEM_
#define _HPP_FS_FILESYSTEM_

#include "main/common.h"
#include "structures/utf8.hpp"
#include "mem/object.hpp"
#include "main/errno.h"
#include "structures/shared_ptr.hpp"
#include "structures/list.hpp"

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
    };

    class Inode {
    public:
        Inode(Filesystem *fs, uint64_t number, InodeType type, uint64_t size);

        Filesystem *fs;
        uint64_t inode_no;
        void *fs_data = nullptr;
        InodeType type;
        object::Object *contents = nullptr;
        uint32_t start = 0; // Start location in the first page
        uint64_t size = 0;
        // Probably other stuff
        Inode *next = nullptr;
        list<InodeEntry> children; // for directories
    };

    // Used to identify files uniquely
    class InodeId {
    public:
        uint32_t filesystem; // id of filesystem
        uint64_t inode;
    };

    class UnderlyingInfo {// Details for the underlying filesystem
    public:
        UnderlyingType type; // UNDER_FS, GENERATED, etc.

        union {
            struct {
                addr_logical_t base;
                bool physical;
            } mem;
        };
    };

    // Base class, individual filesystems (ext4, FAT, etc) inherit this
    class FilePathEntry;
    class Filesystem {
    public:
        uint32_t id; // random id
        // Again, maybe other stuff
        // And details about the underlying filesystem

        virtual error_t read_inode(FilePathEntry *path, uint64_t inode_no, shared_ptr<Inode>& inode) = 0;
        virtual error_t root_inode(shared_ptr<Inode> &inode) = 0;

        UnderlyingInfo info;
    };


    /** Represents the base of a filepath and maybe link to its associated inode
     *
     * When you take a path like `"some/path"`, it is broken down into two FilePathEntry instances (one for "some" and
     *  one for "path"). This path can then be followed from another path in order to navigate the file tree.
     *
     * A FilePathEntry can also be "populated", which means that it (and all its parents) get an Inode associated with
     *  them, which can be used to read and manipulate the file's contents.
     *
     * A FilePathEntry is the leaf node to a tree that represents all of its parents and siblings. In the `"some/path"`
     *  example, the FilePathEntry we manipulate is `"path"`, which has the `"some"` FilePathEntry as its parent.
     *
     * Note that editing directories or mount points will not update the inodes associated with FilePathEntry instances,
     *  they still point to the old mount points.
     *
     * FilePathEntry instances don't load their inode on construction (Instead, you have to call
     *  FilePathEntry::populate). This means it is possible that they are invalid.
     */
    class FilePathEntry {
    public:
        /** The name of this element of the file path */
        Utf8 name;
        /** The parent of this FilePathEntry
         *
         * This may be empty, indicating the lack of any parent.
         */
        shared_ptr<FilePathEntry> parent;
        /** The inode associated with this FilePathEntry
         *
         * May be empty, indicating that no inode has been calculated yet.
         */
        shared_ptr<Inode> inode;

        /** Create a new FilePathEntry */
        FilePathEntry(Utf8 name = Utf8(""), shared_ptr<FilePathEntry> parent = nullptr,
            shared_ptr<Inode> inode = nullptr);

        /** Retreive the Inode from the filesystem for this FilePathEntry and its parents
         *
         * This reads the inode from its parent (populating that, if necessary) to get an inode for this child, and then
         *  sets the FilePathEntry::inode property as appropriate. It may fail in a variety of ways, indicated by the
         *  return value. In which case, the inode will be cleared, and error_loc is set to the FilePathEntry that
         *  caused the error.
         *
         * As well as error conditions produced by reading the inode from the filesystem, the following errors can
         *  occur:
         *
         * * ENOPATHBASE: We climbed to the beginning of the path, and couldn't find a pre-populated FilePathEntry.
         * * ENOTDIR: `error_loc` appears in the middle of the path but is not actually a directory.
         * * ENOENT: `error_loc` does not appear in its parent inode's file list.
         * * EOK: No error
         *
         * @param error_loc A reference set to a FilePathEntry that caused an error
         * @return An error code representing failure of some fashion
         */
        error_t populate(FilePathEntry& error_loc);
    };

    /** Given a file path, convert it into a FilePathEntry path
     *
     * For example, this converts `"some/path"` to a series of FilePathEntry instances. These will be relative to
     *  `base`, which may be null.
     *
     * @param path The path to create
     * @param base The root directory to start creating the path from
     * @return The FilePathEntry of the last entry in the path
     */
    shared_ptr<FilePathEntry> parse_path(const Utf8& path, shared_ptr<FilePathEntry>& base);

    /** The root filesystem */
    extern Filesystem *rootfs;
}

#endif
