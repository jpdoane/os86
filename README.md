# os86
This is a pretty rudimentary hobby 32bit x86 Operating System.  My goals are pretty modest, mostly just to learn about OS architecture.

Implemented:
- multiboot
- paging and virtual memory, with high-memory kernel
- simple heap allocation with first-fit kmalloc() and kfree()
- simple interrupts and exceptions
- simple cooperative multitasking

Todo
- Ring 3, User space
- Scheduling and preemptive multitasking
- File system
- lots of other stuff
- Port to RISC-V and run on jpu?


To compile and run, you will need:
- qemu
- cross-platform 32bit x86 toolchain
