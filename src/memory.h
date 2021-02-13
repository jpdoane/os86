#ifndef __MEMORY_H__
#define __MEMORY_H__

/**** 1st MB layout (identity mapped) ******
0x00000000 	0x000003FF 	Real Mode IVT
0x00000400 	0x000004FF 	BIOS data area (BDA)
0x00000500 	0x00000fff 	AVAILABLE
0x00001000 	0x00001fff 	kernel page directory
0x00002000 	0x00002fff 	Identity page table for 1st MB
0x00003000 	0x00003fff 	kerenel's 1st page table
0x00004000 	0x00007BFF 	AVAILABLE
0x00007C00 	0x00007DFF 	BootSector (512B)
0x00007E00 	0x0007FFFF 	AVAILABLE (480kB)
0x00080000 	0x0009FFFF 	Extended BIOS Data Area
0x000A0000 	0x000BFFFF 	Video display memory
0x000C0000 	0x000C7FFF 	Video BIOS
0x000C8000 	0x000EFFFF 	BIOS Expansions
0x000F0000 	0x000FFFFF 	Motherboard BIOS 
********************************************/

// example paging table layout after boot
// cr3: 0x0010a000                                      Page directory (_boot_page_directory)
// 0x00000000-0x003fffff -> 0x00000000-0x003fffff       identity map of 1st 4MiB (uses pd[0])
// 0x00400000-0x00400fff -> 0x0010d000-0x0010dfff       first page allocated on heap: (uses pd[1], pt[0])
// 0xc0000000-0xc000bfff -> 0x00101000-0x0010cfff       kernel pages (uses pd[0x300] = pd+0xc00)
// 0xffc00000-0xffc00fff -> 0x00000000-0x00000fff       pd[0] remaps lowest page (not actual page table but intepreted as one)
// 0xffc01000-0xffc01fff -> 0x0010e000-0x0010efff       pd[1] page table, maps heap pages 0x00400000-0x007ff000
// 0xfff00000-0xfff00fff -> 0x0010b000-0x0010bfff       pd[0x300] (boot_pt_kernel), maps kernel pages 0xc0000000-0xc03ff000
// 0xfffff000-0xffffffff -> 0x0010a000-0x0010afff       Self-map of last pde -> pd

// extern char _kernel_start_phys;
// extern char _kernel_start;
// #define KERNEL_BASE_PHYS    &_kernel_start_phys //0x00101000
// #define KERNEL_BASE         &_kernel_start      //0xc0000000

// virtual address of the start of the paging structure.  This address is fixed by self-mapping the
// last page directory entry to the page directory itself.
#define PAGING_ADDR         0xffc00000 
#define VGA_BUFFER          0xff000000
#define KERNEL_TOP          0xf8000000
#define KERNEL_BASE         0xc0000000

#define USER_STACK_BASE     0xbffffff0
#define USER_BASE           0x01000000

#define VGA_BUFFER_PHYS     0x000b8000
#define VGA_BUFFER_SIZE     0x00007000

#define KERNEL_BASE_PHYS    0x00101000
#define KERNEL_VIRT_TO_PHYS(p) p-KERNEL_BASE+KERNEL_BASE_PHYS
#define KERNEL_PHYS_TO_VIRT(p) p+KERNEL_BASE-KERNEL_BASE_PHYS


#endif