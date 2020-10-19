# Lab 3 - User Environments

See [https://pdos.csail.mit.edu/6.828/2011/labs/lab3/#Part-A--User-Environments-and-Exception-Handling](https://pdos.csail.mit.edu/6.828/2011/labs/lab3/#Part-A--User-Environments-and-Exception-Handling)

## Question 1

The purpose of having an individual handler function for each exception/interrupt is that, otherwise, it is not possible for the kernel to know the vector number, wich is easily solved by having a different gate function for each it is easy to work out.

## Question 2

The only interruption that a user program is allowed to explicitly invoke is the one that corresponds to a system call. 

Any other explicit invocation of an interruption is programmed by the kernel to be illegal, resulting in a general protection fault. 

So, for instance, if a user program attempts to explicitly invoke a page fault exception, the processor will raise a general protection fault.

The reason that the user shouldn't be allowed to explicitly invoke a page fault exception is that this would be terribly confusing to any kernel page fault handling code, which is often used to implement features that shold be transparent to user programs, such as copy-on-write or distributed memory.

## Question 3

Initially the ```SETGATE``` for ```T_BRKPT``` was configured with ```DPL 0```, which is kernel privileges, and this would cause a general protection fault when ```int3``` was invoked by a user program, given that it runs at ```DPL 3```, user priviliges.

To get it to work, ```T_BRKPT``` must be configured in the IDT with ```DPL 3```.


## Question 4

I think that the point of these IDT protection mechanisms is that it would be a bug for a user program to explicitly invoke interrupts other than system calls and breakpoints. 

This is so because it would defeat the interrupt's mechanism purpose, which is to catch bugs, by allowing the kernel to discriminate unintentional user interrupts (like divide by zero or overflow, which are bugs, or page faults, which might be bugs or not, this is up to the kernal to decide) from intentional user interrupts (system calls and breakpoints).

Protection also allows the kernel to discriminate interrupts caused by user programs, whether intentional or not, bugs or not, from those caused by devices, which would always be a bug if invoked by a user program (unless you want to do something strange, like maybe simulate a device from a user program, but there might be better ways of accomplishing this, I am just guessing).

Proper protection configuration in the ```IDT``` makes user programs more robust and bugs easier to catch, and one can argue that the lack of such protection is one of the reasons that makes kernel development harder.


