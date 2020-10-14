# Homework 3

## Memory layout of kern_pgdir
```
4 Gig -------->  +------------------------------+
                 |        Invalid memory        |
                 -------------------------------- 0xffc00000
                 :                              :
		 :   Remapped physical memory   :
                 :                              :
KERNBASE ----->  +------------------------------+ 0xf0000000
                 |        Invalid memory        |
KSTACKTOP ---->  +------------------------------+ 0xefc00000
		 |         Kernel stack         | 
		 | - - - - - - - - - - - - - -  | 0xefbf8000 KSTKSIZE (8*PGSIZE)
                 |        Invalid memory        |
		 +------------------------------+ 0xef800000
                 |        Invalid memory        |
		 +------------------------------+ 0xef400000
                 |      User image of pages     |
UPAGES --------> +------------------------------+ 0xef000000
		 :                              :
		 :        Invalid memory        :
		 :                              :
0 -------------> +------------------------------+ 0x00000000
```
Instances where multiple regions of virtual memory point to the same physical meory:
- the "User image of pages" and [pages, pages + 4MB]

The kernel want to make some of its structures visible to the user.

## Memory layout of entry_pgdir
``` 
4 Gig ---------> +-----------------------------+
                 :                             :
                 :       Invalid memory        :
                 :                             :
                 +-----------------------------+ 0xf0400000
                 |          Kernel             |
                 |~~~~~~~~~~~~~~~~~~~~~~~~~~~~~| 0xf0100000
		 | Map to 1st MB of phys. mem. |
KERNBASE ------> +-----------------------------+ 0xf0000000
                 :                             :
                 :       Invalid memory        :
                 :                             :
4 MB ----------> +-----------------------------+ 0x00400000
		 | Map to 1st 4MB of phys. mem.|
0 -------------> +-----------------------------+ 0x00000000
``` 

In the entry_pgdir layout the kernel is mapped to a smaller, overlapping region, so it's safe to switch to kern_pgdir layout, where the kernel is mapped to a larger region.

		
