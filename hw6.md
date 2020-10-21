
# Homework 6: Threads and Context Switching

See [https://pdos.csail.mit.edu/6.828/2011/homework/xv6-sched.html](https://pdos.csail.mit.edu/6.828/2011/homework/xv6-sched.html)

*__Submit__: Where is the stack that sched() executes on? Where is the stack that scheduler() executes on? When sched() calls swtch(), does that call to swtch() ever return? If so, when?*

The stack that ```sched()``` executes on is the current processes kernel stack.

The stack that ```scheduler()``` executes on is the cpu's initial, static, booting kernel stack (not the stack of any process' kernel thread).

When ```sched()``` calls ```swtch()```, that call returns when the process that made the call is ran again by the scheduler. This creates the ilusion to the user program that it is running continously.

*__Submit__: Could swtch do less work and still be correct? Could we reduce the size of a struct context? Provide concrete examples if yes, or argue for why not.*

It would be possible for ```swtch``` to do less work and to reduce the size of ```struct context```, but I don't think it would be worth the effort.

The tradeoff is that ```swtch``` would then have to make assumptions its caller: whether the caller uses the registers that the callee shold preserve. If not, then, striclty speaking there would be need to save them. 

The trap handling code will already have saved all the interrupted program's registers, and if it is guaranteed that none of the kernel's code between that and the call to ```swtch()``` doesn't use those registers then there would be no need to save them, and ```struct context``` could be reduced to the stack pointer (which is just the address of the context).

```swtch()``` would be faster, because it would have less work to do (in duplicate, it would not only have to copy fewer registers to the context, but it would also have to copy fewer values from the context to said registers.

But the tradeoff is that ```swtch()``` would be less modular, because it would have to be modified if any of its current (and potential future callers) was modified to use any of those registers.

So, the current implementation of swtch() is trading a bit of speed for better modularity. My impression is that for most applications this is the right tradeoff.

*Surround the call to ```swtch()``` in ```scheduler()``` with calls to ```cprintf()``` like this:*
```
  cprintf("a");
  swtch(&cpu->scheduler, proc->context);
  cprintf("b");
```
*Similarly, surround the call to swtch() in sched() with calls to cprintf() like this:*
```
  cprintf("c");
  swtch(&proc->context, cpu->scheduler);
  cprintf("d");
```

*__Submit__: What is the four-character pattern? The very first characters are ac. Why does this happen?*

This is a snippet of the pattern: ```acbadcbadcbadcbadcbadcbadc```

The four character pattern is:

```b``` Back to the scheduler.
```a``` The scheduler switches to another process
```d``` Some process is run again
```c``` Some process is switched out

The very first characters:

```a```	The scheduler switches to the first process
```c``` The first process is switched out 

 The reason is that the first process is faked to run as if it had returned from a system call to fork, rather than a timer interrupt (that would have ended with a call to ```swtch()```.

