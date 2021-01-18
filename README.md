# os86
baremetal sandbox for rudimentary x86 os development

Implemented:
- simple paging and virtual memory
- simple linear heap allocation (but not yet free)
- simple interrupts and exceptions
- Simple task swicthing

Todo
- User space
- Scheduling
- Proper memory management
- File system
- lots of other stuff

To compile and run, you will need to set up qemu and a cross-platform 32bit x86 toolchain, per instructions on osdev.org
