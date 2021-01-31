# os86
baremetal sandbox for rudimentary x86 os development

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

To compile and run, you will need:
- qemu
- cross-platform 32bit x86 toolchain
