# Lab 2

## Exercise 3

### Question 1
```
	mystery_t x;
	char* value = return_a_pointer();
	*value = 10;
	x = (mystery_t) value;
```

mystery_t should have type uintptr_t, given that it points to a virtual address.

## Exercise 5

### Question 2
```
Entry 	Base Virtual Address 	Points to (logically):

1023	
1022	0xFF800000		0x0F800000 (max of mappable physical memory)
...				
960     0xF0000000		0x00000000 (start of physical memory)
...
958	0xEFC00000		(kernel stack, 8 4K pages starting at page top)
...				
955	0xEF000000		(pages)
...
0
```

### Question 3

User's won't be able to read or write kernel memory, in spite of being in the same address space, becuase of how the protection bits have been set in the relevant table entries.

### Question 4

This system can support at most 256M of physical memory, because all physical memory is mapped starting at virtual address 0xF0000000, leaving only said ammount of memory.

### Question 5

The overhead for managing memory, if the maximum physical memory was available (256MB), just counting the Pages structure (which keeps a linked list of free physical memory 4MB pages), would be around 512K (256MB total physical memory/4K per page = 64K instances of struct Page, each of which is at most 8 bytes (6 bytes aligned)). On top of this one would have to add the directory tables, one per process, and the page tables, at least a few per process, but these add to just a few tens of KB, I suspect, for most processes. So, all things considered, I reckon that memory management adds up to the order of less than 1% of available physical memory.

### Question 6

We transion to a high EIP in kernel entry by setting a few control registers and then jumping. It is possible to execute at EIPs and then at high EIPs because both have been mapped; the transition to high EIPs is necessary because the kernel was linked to high addresses, but we have to start executing at low EIPs, becuase the kernel is physically loaded at a low address.
