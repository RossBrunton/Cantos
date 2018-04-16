#include <stdint.h>

#include "fs/filesystem.hpp"
#include "structures/utf8.hpp"
#include "main/printk.hpp"
#include "main/errno.h"
#include "test/test.hpp"
#include "main/cpu.hpp"

namespace filesystem {
    Inode::Inode(Filesystem &fs, uint64_t number, InodeType type, uint64_t size)
        : fs(fs), inode_no(number), type(type), size(size) {
        // TODO: Not thread safe
        fs.inodes ++;
    }

    Inode::~Inode() {
        fs.inodes --;
    }


    FilePathEntry::FilePathEntry(Utf8 name, shared_ptr<FilePathEntry> parent, shared_ptr<Inode> inode)
        : parent(parent), inode(inode), name(name) {}

    error_t FilePathEntry::populate(FilePathEntry& error_loc) {
        error_t err;
        shared_ptr<Inode> dir;
        FilePathEntry &lp = logical_parent();

        if(!lp.get_inode()) {
            // Parent node does not exist, ask it to populate
            err = parent->populate(error_loc);
            if(err != 0) {
                return err;
            }
        }

        if(name == ".") {
            ref = &lp;
            return EOK;
        }

        if(name == "..") {
            ref = &lp.logical_parent();
            return EOK;
        }

        dir = lp.get_inode();
        error_loc = *this;

        if(dir->type != InodeType::DIRECTORY) {
            error_loc = *parent;
            return ENOTDIR;
        }

        for(InodeEntry &ie : dir->children) {
            if(ie.name == name) {
                auto result = dir->fs.read_inode(ie.inode);

                if(result) {
                    inode = result.val;
                    return EOK;
                }else{
                    inode = nullptr;
                    return err;
                }
            }
        }

        return ENOENT;
    }

    shared_ptr<Inode> FilePathEntry::get_inode() {
        if(ref) {
            return ref->get_inode();
        }else{
            return inode;
        }
    }

    FilePathEntry &FilePathEntry::path_parent() {
        return *parent;
    }

    FilePathEntry &FilePathEntry::logical_parent() {
        if(ref) {
            return ref->logical_parent();
        }else if(!parent) {
            return *this;
        }else{
            return *parent;
        }
    }



    Failable<page::Page *> EmptyStorage::read(addr_logical_t addr, uint32_t count) {
        page::Page *page = page::alloc(flags, count);
        return Failable<page::Page *>(EOK, page);
    }


    Filesystem::~Filesystem() {
        if(inodes) {
            panic("Filesystem was destroyed without releasing all inodes");
        }
    }

    shared_ptr<FilePathEntry> parse_path(const Utf8& path, shared_ptr<FilePathEntry>& base) {
        size_t pos = 0;
        size_t oldpos = 0;
        shared_ptr<FilePathEntry> parent = base;

        while((pos = path.find('/', oldpos)) != Utf8::npos) {
            Utf8 fname = path.substr(oldpos, pos - oldpos);

            if(fname.bytes()) {
                parent = make_shared<FilePathEntry>(fname, parent);
            }

            oldpos = pos + 1;
        }

        // And the rest of the string
        Utf8 fname = path.substr(oldpos, Utf8::npos);
        if(fname.bytes()) {
            parent = make_shared<FilePathEntry>(fname, parent);
        }

        return parent;
    }

    Filesystem *rootfs = nullptr;
}
