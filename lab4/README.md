
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

