// implement fork from user space

#include <inc/string.h>
#include <inc/lib.h>

// PTE_COW marks copy-on-write page table entries.
// It is one of the bits explicitly allocated to user processes (PTE_AVAIL).
#define PTE_COW		0x800

extern void _pgfault_upcall(void);

//
// Custom page fault handler - if faulting page is copy-on-write,
// map in our own private writable copy.
//
static void
pgfault(struct UTrapframe *utf)
{
	void *addr = (void *) utf->utf_fault_va;
	int r;

	// Check that the faulting access was (1) a write, and (2) to a
	// copy-on-write page.  If not, panic.
	// Hint:
	//   Use the read-only page table mappings at vpt
	//   (see <inc/memlayout.h>).

	// LAB 4: Your code here.

	pte_t pte = vpt[PGNUM(utf->utf_fault_va)];
	unsigned perm = pte & 0xFFF;

	// must not use thisenv, because initially it will still be the parent's
	envid_t envid = sys_getenvid();

	//cprintf("pgfault  envid %x, err 0x%03x, va %08p, pte 0x%08x, perm 0x%03x, eip %08p\n",
	//	envid, utf->utf_err, utf->utf_fault_va, pte, (pte&0xFFF), utf->utf_eip);

	// If a write, the second error bit is set.
	if (!((utf->utf_err & 2) && (perm | PTE_COW))) {
		panic("pgfault perm");
	}

	// Allocate a new page, map it at a temporary location (PFTEMP),
	// copy the data from the old page to the new page, then move the new
	// page to the old page's address.
	// Hint:
	//   You should make three system calls.
	//   No need to explicitly delete the old page's mapping.

	// LAB 4: Your code here.

	//panic("pgfault not implemented");

	int newperm = ((perm & ~PTE_COW) | PTE_W);

	if ((r = sys_page_alloc(envid, (void*) PFTEMP, newperm)) < 0) {
		panic("pgfault alloc  err %d, envid %x, perm %03x, newperm %03x",
		       r, envid, perm, newperm);
	}

	memmove((void*) PFTEMP, (void*) (utf->utf_fault_va & ~0xFFF), PGSIZE);

	if ((r = sys_page_map(envid, (void*) PFTEMP, 
			      envid, (void*) utf->utf_fault_va, newperm)) < 0) {
		panic("pgfault map");
	}
}

//
// Map our virtual page pn (address pn*PGSIZE) into the target envid
// at the same virtual address.  If the page is writable or copy-on-write,
// the new mapping must be created copy-on-write, and then our mapping must be
// marked copy-on-write as well.  (Exercise: Why do we need to mark ours
// copy-on-write again if it was already copy-on-write at the beginning of
// this function?)
//
// Returns: 0 on success, < 0 on error.
// It is also OK to panic on error.
//
static int
duppage(envid_t envid, uint32_t pn)
{
	//cprintf("duppage  envid %08p, pn %d\n", envid, pn);
	int r;

	// LAB 4: Your code here.
	//panic("duppage not implemented");

	pte_t pte = vpt[pn];

	if ((pte & PTE_W) || (pte & PTE_COW)) {
		void* va = (void*) (pn << 12);
		unsigned perm = (((pte & 0xFFF) | PTE_COW) & ~PTE_W);
		int err;
		//cprintf("duppage  thisenv %08x, va %08p, envid %08x, perm %03x\n",
			//thisenv->env_id, va, envid, perm);
		if ((err = sys_page_map(thisenv->env_id, va, envid,   va, perm)) < 0) {
			panic("duppage child  %d", err);
		}
		if ((err = sys_page_map(thisenv->env_id, va, thisenv->env_id, va, perm)) < 0) {
			panic("duppage parent  %d", err);
		}
	}

	return 0;
}

//
// User-level fork with copy-on-write.
// Set up our page fault handler appropriately.
// Create a child.
// Copy our address space and page fault handler setup to the child.
// Then mark the child as runnable and return.
//
// Returns: child's envid to the parent, 0 to the child, < 0 on error.
// It is also OK to panic on error.
//
// Hint:
//   Use vpd, vpt, and duppage.
//   Remember to fix "thisenv" in the child process.
//   Neither user exception stack should ever be marked copy-on-write,
//   so you must allocate a new page for the child's user exception stack.
//
envid_t
fork(void)
{
	// LAB 4: Your code here.
	//panic("fork not implemented");

	int r;

	set_pgfault_handler(pgfault);

	envid_t envid;
 	if ((envid = sys_exofork()) < 0) {
		panic("fork");
	} else if (envid == 0) {
		// in the child

		thisenv = &envs[ENVX(sys_getenvid())];
		
		return 0;
	}

	// in the parent

	// Copy address space to the child.
	// - In the outer loop over the page directory PTEs to check if a page table
	//   is writable or COW.
	// - If it is, then loop over it to look for writable or COW PTEs.

	// page fault handler setup to the child.
	if ((r = sys_env_set_pgfault_upcall(envid, _pgfault_upcall)) < 0) {
		panic("fork pgfault upcall  envid %x, err %i", envid, r); 
	}
	if ((r = sys_page_alloc(envid, (void*) (UXSTACKTOP - PGSIZE), PTE_U|PTE_W)) < 0) {
		panic("fork uxstack alloc  envid %x, err %i", envid, r);
	}

	//cprintf("uvpt %p\n", uvpt[PGNUM(0xef7bb000)]);
	//cprintf("uvpt %p\n", *(uint32_t*)(UVPT + PDX(0x800000)*PGSIZE + PTX(0x800000)*4));
	//cprintf("uvpt %p\n", ((pte_t*)UVPT)[PGNUM(0x800000)]);

	// For each writable or copy-on-write page in its address space below UTOP, 
	// the parent calls duppage

	uint32_t pdx, ptx;

	for (pdx = 0; pdx < 1024; pdx++) {
		pte_t pde = vpd[pdx];
		if ((pde & (PTE_P|PTE_U)) == (PTE_P|PTE_U)) {   // user pde?
			//cprintf("PDE[%03x] %08p ", pdx, pde);
			//if (pde & PTE_U) cprintf("U");
			//if (pde & PTE_W) cprintf("W");
			//cprintf("\n");
			for (ptx = 0; ptx < 1024; ptx++) {
				uint32_t pn = pdx*1024 + ptx;
				pte_t pte = vpt[pn];
				//if ((pte & PTE_P) && ((pte & PTE_W) || (pte & PTE_COW))) {
					//cprintf("\tPTE[%03x] %08p ", ptx, pte);
					//if (pte & PTE_U) cprintf("U");
					//if (pte & PTE_W) cprintf("W");
					//cprintf("\n");
				//}
				//cprintf("fork  pdx %d, perm 0x%03x, ptx %d, perm 0x%03x, pn %d\n", 
					//pdx, (pde&0xFFF), ptx, (pte&0xFFF), pn);
				if ((pte & PTE_P) != 0) {   // page mapped?
					// don't duplicate any pages above the user
					// stack bottom
					if (pn < PGNUM(USTACKTOP-PGSIZE)) {
						duppage(envid, pn);
					} 

				}
			}
		}
	}

        // Also copy the stack we are currently running on.
        duppage(envid, PGNUM(ROUNDDOWN(USTACKTOP-PGSIZE, PGSIZE)));


	if ((r = sys_env_set_status(envid, ENV_RUNNABLE)) < 0) {
		panic("fork set status  envid %x, err %d", envid, r);
	}

	return envid;
}

// Challenge!
int
sfork(void)
{
	panic("sfork not implemented");
	return -E_INVAL;
}
