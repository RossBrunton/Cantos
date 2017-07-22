#include <stdint.h>

#include "fs/filesystem.hpp"
#include "structures/utf8.hpp"
#include "main/printk.hpp"
#include "main/errno.h"

namespace filesystem {
    InodeEntry::InodeEntry(uint64_t inode, Utf8 name) : inode(inode), name(name) {}

    Inode::Inode(Filesystem *fs, uint64_t number, InodeType type, uint64_t size) {
        this->fs = fs;
        this->inode_no = number;
        this->type = type;
        this->size = size;
    }

    void Inode::adjust_ref(uint32_t delta) {
        this->ref += delta;

        if(!this->ref) {
            delete this;
        }
    }


    FilePathEntry::FilePathEntry(Utf8 name, FilePathEntry *parent) : name(name), parent(parent) {
        this->parent->adjust_ref(1);
    }
    FilePathEntry::FilePathEntry(Utf8 name, FilePathEntry *parent, Inode* inode)
        : name(name), parent(parent), inode(inode) {
        this->parent->adjust_ref(1);
    }

    FilePathEntry::~FilePathEntry() {
        this->parent->adjust_ref(-1);

        if(this->inode) {
            this->inode->adjust_ref(-1);
        }
    }

    error_t FilePathEntry::populate(FilePathEntry **tip) {
        error_t err;
        if(!this->parent) {
            // Root of the fs tree
            *tip = this;
            err = rootfs->root_inode(&this->inode);
            if(err != 0) {
                this->inode = nullptr;
                return err;
            }
            return EOK;
        }else{
            if(!this->parent->inode) {
                // Parent node does not exist, ask it to populate
                err = this->parent->populate(tip);
                if(err != 0) {
                    return err;
                }
            }

            *tip = this;

            if(this->parent->inode->type != TYPE_DIRECTORY) {
                return ENOTDIR;
            }

            InodeEntry *ie = this->parent->inode->children;
            while(ie) {
                if(ie->name == this->name) {
                    err = this->parent->inode->fs->read_inode(this, ie->inode, &this->inode);

                    if(err) {
                        this->inode = nullptr;
                        return err;
                    }else{
                        return EOK;
                    }
                }
                ie = ie->next;
            }

            return ENOENT;
        }
    }

    void FilePathEntry::adjust_ref(uint32_t delta) {
        this->ref += delta;

        if(!this->ref) {
            delete this;
        }
    }

    FilePathEntry *FilePathEntry::copy() {
        FilePathEntry *cpy = new FilePathEntry(this->name, this->parent, this->inode);

        cpy->adjust_ref(1);

        return cpy;
    }

    FilePathEntry *parse_path(Utf8 path, FilePathEntry *parent) {
        size_t pos = 0;
        size_t oldpos = 0;

        while((pos = path.find('/', oldpos)) != Utf8::npos) {
            Utf8 fname = path.substr(oldpos, pos - oldpos);

            if(fname.bytes()) {
                parent = new FilePathEntry(fname, parent);
            }

            oldpos = pos + 1;
        }

        // And the rest of the string
        Utf8 fname = path.substr(oldpos, Utf8::npos);
        if(fname.bytes()) {
            parent = new FilePathEntry(fname, parent);
        }

        parent->adjust_ref(1);
        return parent;
    }


    class TestFilesystem : public Filesystem {
        error_t read_inode(FilePathEntry *path, uint32_t inode_no, Inode **inode) {
            if(inode_no == 1) {
                Inode *i = new Inode(this, 1, TYPE_DIRECTORY, 0);
                i->adjust_ref(1);
                InodeEntry *x = i->children = new InodeEntry(1, Utf8("."));
                x = x->next = new InodeEntry(1, Utf8(".."));
                x->next = new InodeEntry(1, Utf8("a"));

                *inode = i;
                return EOK;
            }else{
                return ENOENT;
            }
        }

        error_t root_inode(Inode **inode) {
            return this->read_inode(nullptr, 1, inode);
        }
    };

    Filesystem *rootfs = new TestFilesystem();
}
