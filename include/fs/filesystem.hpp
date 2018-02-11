#ifndef _HPP_FS_FILESYSTEM_
#define _HPP_FS_FILESYSTEM_

#include "main/common.hpp"
#include "structures/utf8.hpp"
#include "mem/object.hpp"
#include "main/errno.h"
#include "structures/shared_ptr.hpp"
#include "structures/list.hpp"

namespace filesystem {
    class Filesystem;

    static uint32_t filesystem_counter;

    enum InodeType {
        TYPE_DIRECTORY,
        TYPE_FILE
    };

    struct InodeEntry {
        uint64_t inode;
        Utf8 name;
    };

    class Inode {
    public:
        Inode(Filesystem &fs, uint64_t number, InodeType type, uint64_t size);
        ~Inode();

        Filesystem& fs;
        uint64_t inode_no;
        void *fs_data = nullptr;
        InodeType type;
        shared_ptr<object::Object> contents;
        uint64_t size = 0;
        // Probably other stuff

        vector<InodeEntry> children; // for directories
    };

    /** A filesystem id and inode pair
     *
     * This can be used to uniquely identify inodes across filesystems.
     */
    struct InodeId {
        uint32_t filesystem; /**< Filesystem id */
        uint64_t inode; /**< Inode id */
    };

    class Storage {// Details for the underlying filesystem
    public:
        virtual ~Storage() {};

        virtual Failable<page::Page *> read(addr_logical_t addr, uint32_t count) = 0;
    };

    class EmptyStorage : public Storage {
    private:
        uint8_t flags;

    public:
        EmptyStorage(uint8_t flags) : flags(flags) {};
        Failable<page::Page *> read(addr_logical_t addr, uint32_t count) override;
    };

    // Base class, individual filesystems (ext4, FAT, etc) inherit this
    class FilePathEntry;

    /** Represents a single filesystem
     *
     * Different filesystem types (ext4, FAT, etc.) should create subclasses of this class to represent themselves,
     *  and provide the various operations required here.
     *
     * Instances of those created classes represent instances of those filesystems mapped over a given
     *  UnderlyingStorage (which manages reading of the raw binary data).
     */
    class Filesystem {
    public:
        /** A unique ID for the filesystem */
        uint32_t id;

        /** Constructs a new Filesystem over the given underlying storage
         *
         * This constructor should be called by any subclasses, and generates the filesystem id.
         *
         * @param us The UnderlyingStorage that this filesystem rests upon
         */
        Filesystem(shared_ptr<Storage> us) : id(filesystem_counter++), storage(us) {};
        virtual ~Filesystem();

        /** Given an inode number, load it or return an error
         *
         * If an error occurs, inode may be left in an undefined state.
         *
         * Implementations may reuse the same inode instance to represent the same inode.
         *
         * @param path The file path to read, this may be null
         * @param inode_no The inode number to read
         * @param inode A shared_ptr to the inode, this should be updated to point to an inode instance
         * @return An error code
         */
        virtual Failable<shared_ptr<Inode>> read_inode(uint64_t inode_no) = 0;
        /** Returns the root inode of this filesystem, or returns an error
         *
         * As read_inode but returns the root inode of the filesystem, rather than a specified one.
         *
         * Root inodes must be of type TYPE_DIRECTORY.
         *
         * @param inode A shared_ptr to the inode, this should be updated to point to an inode instance
         * @return An error code
         */
        virtual Failable<shared_ptr<Inode>> root_inode() = 0;

        virtual shared_ptr<Storage> get_storage() {
            return storage;
        }

    private:
        friend Inode;
        int32_t inodes = 0;
        shared_ptr<Storage> storage;
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
    private:
        shared_ptr<FilePathEntry> parent;
        FilePathEntry *ref = nullptr;
        shared_ptr<Inode> inode;

    public:
        /** The name of this element of the file path */
        Utf8 name;

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
        error_t populate(FilePathEntry &error_loc);

        FilePathEntry &logical_parent();
        FilePathEntry &path_parent();
        shared_ptr<Inode> get_inode();
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
