# Lab 3 - User Environments

See [https://pdos.csail.mit.edu/6.828/2011/labs/lab3/#Part-A--User-Environments-and-Exception-Handling](https://pdos.csail.mit.edu/6.828/2011/labs/lab3/#Part-A--User-Environments-and-Exception-Handling)

## Question 1

The purpose of having an individual handler function for each exception/interrupt is that, otherwise, it is not possible for the kernel to know the vector number, wich is easily solved by having a different gate function for each it is easy to work out.

## Question 2

The only interruption that a user program is allowed to explicitly invoke is the one that corresponds to a system call. 

Any other explicit invocation of an interruption is programmed by the kernel to be illegal, resulting in a general protection fault. 

So, for instance, if a user program attempts to explicitly invoke a page fault exception, the processor will raise a general protection fault.

The reason that the user shouldn't be allowed to explicitly invoke a page fault exception is that this would be terribly confusing to any kernel page fault handling code, which is often used to implement features that shold be transparent to user programs, such as copy-on-write or distributed memory.


