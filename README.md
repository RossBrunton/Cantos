## Cantos ##

This is a silly little toy x86 operating system I'm writing.

It's mostly just for fun, and to see if I can do it, I doubt this will see actual usage anywhere.

At the moment, the following features are kind-of supported:

- Booting into protected mode with a page table (using multiboot)
- Basic (and not that stable) multithreading
- Printing to the screen and serial port
- Reading from the keyboard (but not doing anything with that information)
- Handling multiple kernel threads
- Parsing its ELF headers to build a stack trace

### Compilation ###
First of all, you'll need a GCC cross compiler for `i686-elf`, instructions on how to do that can be found at http://wiki.osdev.org/GCC_Cross-Compiler .

It should then be as simple as running `make all` to get a kernel binary in `bin/cantos.bin`, or `make grub` to get an ISO with grub as `cantos.iso`. The latter requires the `grub-mkrescue` program to be installed.

### License ###
I'm not really sure what license I'll end up using for this, so for now I've released it under the GPLv3. I may make it more permissive at a later point.

### Thanks ###
Most of this was done following the tutorials and references on http://wiki.osdev.org/Main_Page .
