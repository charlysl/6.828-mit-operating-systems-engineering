
# Lab 4: Preemptive Multitasking

See (https://pdos.csail.mit.edu/6.828/2011/labs/lab4/#Exercise-1)[https://pdos.csail.mit.edu/6.828/2011/labs/lab4/#Exercise-1]

## Question 1

*Compare ```kern/mpentry.S``` side by side with ```boot/boot.S```. Bearing in mind that ```kern/mpentry.S``` is compiled and linked to run above ```KERNBASE``` just like everything else in the kernel, what is the purpose of macro ```MPBOOTPHYS```?*

The macro translates a link address in ```kern/mpentry.S``` to the physical address to which the data in that link address has been copied to by ```kern/boot_aps```. 

It does this by subtracting the source base link address (```mpentry_start```) and adding the destination base physical address (```MPENTRY_PADDR```).

*Why is it necessary in kern/mpentry.S but not in boot/boot.S? In other words, what could go wrong if it were omitted in kern/mpentry.S?
Hint: recall the differences between the link address and the load address that we have discussed in Lab 1.*

It is necessary in ```kern/mpentry.S``` because ```boot_aps``` copies the code starting at link address ```mpentry_start``` (in ```kern/mpentry.S```) to physical address ```MPENTRY_PADDR```, from where it is run by each PA in turn, to boot itself.

But symbolic addresses in the ```kern/mpentry.S``` code (like ```gtdesc```) are still link addresses, so they have to be translated to the physical address where they will be for execution, which is what the macro does.

On the other hand, each link address in ```boot/boot.S``` is identical to its physical address, so there is no need to translate it.

## Question 2

*It seems that using the big kernel lock guarantees that only one CPU can run the kernel code at a time. Why do we still need separate kernel stacks for each CPU?* 

The problem is that even when one of the CPUs might already be holding the lock, this wouldn't prevent an interrupt in other CPUs from cloberring the kernel stack.

*Describe a scenario in which using a shared kernel stack will go wrong, even with the protection of the big kernel lock.*

1. CPU 0 is interrupted, trap pushes registers to kernel stack, acquires kernel lock, and starts handling interrupt

2. CPU 1 is interrupted, trap pushes registers to kernel stack (totally confusing CPU 0 interrupt handling code), spins on kernel lock

3. CPU 0 panics, or behaves strangely, because the kernel stack was corrupted by CPU 1

## Question 3

*In your implementation of ```env_run()``` you should have called ```lcr3()```. Before and after the call to ```lcr3()```, your code makes references (at least it should) to the variable ```e```, the argument to ```env_run```. Upon loading the ```%cr3``` register, the addressing context used by the MMU is instantly changed. But a virtual address (namely ```e```) has meaning relative to a given address context--the address context specifies the physical address to which the virtual address maps. Why can the pointer ```e``` be dereferenced both before and after the addressing switch?*

```e``` points to a virtual address that is above ```KERNBASE```. All virtual addresses above ```KERNBASE``` are mapped for all environment's page directories to map to the same, kernel, physical addresses, readable only when executing in the kernel. Hence, although the call to ```lcr3()``` has changed the page directory, ```e``` is still mapped to exactly the same physical address, and because we are still executing inside ```env_run()```, we are still inside the kernel, so the pointer to ```e``` can still be dereferenced both before and after the addressing switch.


